#include "Arduino.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "driver/gpio.h"
#include <LVGL.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include <Dimmer.hpp>
#include <Settings.hpp>
#include <GUI.hpp>
#include <DisplayManager.hpp>
#include <LVGLButton.hpp>
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_gap_ble_api.h"
#include "driver/uart.h"
#include <NVS.hpp>
#include <BLETest.hpp>
#include <BLE.hpp>
#include <GAIA.hpp>
#include <Uart.hpp>
#include <MorphDevice.hpp>
#include <Utils/StringUtils.hpp>
#include "arpa/inet.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include <Volume.h>
#include <MorphDevice.hpp>
#include <Events.hpp>
#include <ADC.hpp>
#include <SmartCase.hpp>
#include <AsyncFunctor.hpp>
#include <HttpWebserver.hpp>
#include <GpioButton.hpp>
#include <ESP32.hpp>
#include "driver/dac.h"
#include <FactoryReset/FactoryResetFSM.hpp>
#include <FactoryReset/WaitForUserPressNextState.hpp>
#include <FactoryReset/EarbudsFactoryResetState.hpp>
#include <FactoryReset/WaitForConnectionState.hpp>
#include <FactoryReset/PutEarbudsInCaseState.hpp>
#include <FactoryReset/TakeEarbudsFromCaseState.hpp>
#include <FactoryReset/FinalState.hpp>
#include <LanguagePack.hpp>
#include <Languages.hpp>
#include <LanguagePackCollection.hpp>
#include <MessageDispatcher.hpp>
#include <DebugMenu.hpp>
#include <Fonts.hpp>

using namespace Languages;

void batteryControlTask(void* args);
void initializeMessagesDispatching();
static MessageConsumer webserverStopTracker;

extern "C" void app_main() {
    // that's what we do first - selfpowering as fast as possible
    SMARTCASE.setSelfPower(true);
    SMARTCASE.init();

    esp_err_t ret {nvs_flash_init()};
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Loading settings and paired devices list from FLASH(NVS)
    SETTINGS.init();
    MORPH_DEVICE.loadPairedDeviceListFromFlash();
    LANGUAGES.setLanguage(static_cast<eLanguage>(SETTINGS.language()));

#ifndef RELEASE_BUILD 
    #warning "RELEASE_BUILD is not set"
    // ONLY FOR DEBUG PURPOSE
    assert(pdTRUE == xTaskCreate(consoleTask, "menuTask", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL));
#endif

#ifdef USE_DBG
    #warning "USE_DBG is set!"
    // Thread safe dbg initialization
    DBG.setAllowPrint(true);
    DBG.start();
    vTaskDelay(pdMS_TO_TICKS(10));
#endif

    // LVGL initialization and launch
    LVGL lvgl(LVGL_TASK_PERIOD);
    assert(lvgl.init());
    lvgl.start();

    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_t* bootUpImage {lv_img_create(lv_disp_get_scr_act(NULL), NULL)};
    lv_img_set_src(bootUpImage, &morph_img_black);
    lv_obj_align(bootUpImage, NULL, LV_ALIGN_CENTER, 0, 0);

    // ADC module launching, it's needed to measure battery voltage as fast as possible
    ADC_MODULE.init();
    ADC_MODULE.start();
    vTaskDelay(pdMS_TO_TICKS(200));
    SMARTCASE.setBackLight(100);

    /*
        If voltage is in dangerous zone, e.g. below MIN_BATTERY_PERCENT_LAUNCH level,
        then user will see only warning message and then device is switched off
    */
    if (false == BATTERY_CONTROL_ENABLED or SMARTCASE.get_batteryPercent() > MIN_BATTERY_PERCENT_LAUNCH) {
        GRAPHICS.init(lvgl.acceccMutex());
        GRAPHICS.setCaseChargeLevel(SMARTCASE.get_batteryPercent());
    }
    else {
        LVGLLabel* criticalBatteryMessage = new LVGLLabel(LANGUAGE_PACK[eTitleType::TITLE_BATTERY_EMPTY]);
        criticalBatteryMessage->setFont(LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_24]);
        criticalBatteryMessage->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        criticalBatteryMessage->align(criticalBatteryMessage->parent(), LV_ALIGN_CENTER, 0, 0);
        
        SMARTCASE.setBackLight(100);
        delay(5000);
        SMARTCASE.setSelfPower(false);

        while (1) {
            // we are here only if user continue to hold the physical button
            delay(10000);
        }
    }

    /*
        Physical button initialization
        longpress for SWITCH_OFF_PRESS_TIME_MS will trigger selfpower cut-off
    */
    GpioButton caseBtn(CASE_BTN_GPIO, eGpioButtonPressedLogicLevel::BUTTON_PRESSED_LEVEL_LOW, false, true, 200);
    caseBtn.init();

    initializeMessagesDispatching();
    DISPATCHER.start();

    std::function<void (void*)> btnLongPressHandler = [](void* arg) -> void {
        SMARTCASE.disableDisplayManager();
        SMARTCASE.safeSwitchOff(LANGUAGE_PACK[eTitleType::TITLE_SWITCHING_OFF]);
    };
    caseBtn.onHeldForMs(btnLongPressHandler, SWITCH_OFF_PRESS_TIME_MS);

    // GPIO interruption settings for wake-up enabling
    // ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());
    // ESP_ERROR_CHECK(gpio_wakeup_enable(CASE_BTN_GPIO, GPIO_INTR_LOW_LEVEL));

    // ONLY FOR DEBUG PURPOSE
    esp_log_level_set("XPT2046", esp_log_level_t::ESP_LOG_ERROR);
    DBG_PRINTLN("\r\n\r\nStarted!");
    
    // BLE task starting and enabling the DisplayManager
    BLE_DEVICE.setMacSwapEnabled(true);
    BLE_DEVICE.start();
    SMARTCASE.enableDisplayManager();

    assert(pdTRUE == xTaskCreate(batteryControlTask, "batteryTask", configMINIMAL_STACK_SIZE*3, NULL, 1, NULL));

    // launching physical button handler task
    caseBtn.start();

    static WaitForUserPressNextState INIT_STATE(GRAPHICS.eventGroup());
    static EarbudsFactoryResetState EARBUDS_RESET_STATE(GRAPHICS.eventGroup());
    static WaitForConnectionState WAIT_FOR_CONNECTOION_STATE(GRAPHICS.eventGroup());
    static PutEarbudsInCaseState PUT_EARBUDS_IN_CASE_STATE(GRAPHICS.eventGroup());
    static TakeEarbudsFromCaseState TAKE_EARBUDS_FROM_CASE_STATE(GRAPHICS.eventGroup());
    static FinalState FINAL_STATE(GRAPHICS.eventGroup());

    static FSMTransition_t TRANSITIONS_TABLE[] {

        {static_cast<AbstractState*>(&INIT_STATE), FSM_SIGNAL_USER_CONFIRMATION, static_cast<AbstractState*>(&EARBUDS_RESET_STATE)},
        
        {static_cast<AbstractState*>(&EARBUDS_RESET_STATE), FSM_SIGNAL_EARBUDS_RESET_DONE, static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE)},
        
        {static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE), FSM_SIGNAL_FIRST_PAIRING_DONE, static_cast<AbstractState*>(&PUT_EARBUDS_IN_CASE_STATE)},
        {static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE), FSM_SIGNAL_SECOND_PAIRING_DONE, static_cast<AbstractState*>(&PUT_EARBUDS_IN_CASE_STATE)},
        {static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE), FSM_SIGNAL_PAIRING_TIMEOUT, static_cast<AbstractState*>(&FINAL_STATE)},
        {static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE), FSM_SIGNAL_PRIMARY_DEVICE_REQ_FAILED, static_cast<AbstractState*>(&FINAL_STATE)},
        {static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE), FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE, static_cast<AbstractState*>(&FINAL_STATE)},
        
        {static_cast<AbstractState*>(&PUT_EARBUDS_IN_CASE_STATE), FSM_SIGNAL_EARBUDS_ARE_IN_CASE_FIRST, static_cast<AbstractState*>(&TAKE_EARBUDS_FROM_CASE_STATE)},
        {static_cast<AbstractState*>(&PUT_EARBUDS_IN_CASE_STATE), FSM_SIGNAL_EARBUDS_ARE_IN_CASE_SECOND, static_cast<AbstractState*>(&FINAL_STATE)},
        {static_cast<AbstractState*>(&PUT_EARBUDS_IN_CASE_STATE), FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE, static_cast<AbstractState*>(&FINAL_STATE)},
        
        {static_cast<AbstractState*>(&TAKE_EARBUDS_FROM_CASE_STATE), FSM_SIGNAL_EARBUDS_ARE_OUT_OF_CASE, static_cast<AbstractState*>(&WAIT_FOR_CONNECTOION_STATE)},
        {static_cast<AbstractState*>(&TAKE_EARBUDS_FROM_CASE_STATE), FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE, static_cast<AbstractState*>(&FINAL_STATE)},
    };

    static std::vector<FSMTransition_t> transitions(TRANSITIONS_TABLE, TRANSITIONS_TABLE + sizeof(TRANSITIONS_TABLE)/sizeof(TRANSITIONS_TABLE[0]));

    FACTORY_RESET_FSM.setTransitionTable(transitions);
    FACTORY_RESET_FSM.setInitialState(&INIT_STATE);

    GRAPHICS.start();

    auto heapPrintFucntion = []() {

        while(1) {
            printf("free heap: %u\nmax free block size: %u\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());
            delay(1000);
        }
    };
    static AsyncFunctor heapPrinter(heapPrintFucntion, "heapPrintFucntion", configMINIMAL_STACK_SIZE*3);
    heapPrinter.start();

    while (1) {
        
        message_t message {};
        webserverStopTracker.consumeMessage(message);
        assert (eMessageCode::MESSAGE_WEBSERVER_STATE_CHANGED == message.messageCode);
        DBG_PRINTLN("Webserver state changed");
        delay(1000);
         
        if (false == WEBSERVER.isActive() and false == FACTORY_RESET_FSM.isRunning()) {
            DBG_PRINTLN("Launching BLE task..");
            BLE_DEVICE.start();
        }
    }
    vTaskDelete(NULL);
}

void initializeMessagesDispatching() {

    BLE_DEVICE.setOutcomingQueue(DISPATCHER.getRxQueue());
    WEBSERVER.setOutcomingQueue(DISPATCHER.getRxQueue());


    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_BLE_CONNECTION_STATE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_EARBUDS_CHARGE_LEVEL_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_ANC_STATE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_ANC_MODE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_MIC_STATE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_VOLUME_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_STATE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_INFO_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_LIST_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_CONNECTION_STARTED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_CONNECTION_FINISHED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HIDE_LOADING_PAGE, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_WEBSERVER_STATE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_PRIMARY_DEVICE_CHANGED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_OTA_STARTED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_OTA_FAILED, &GRAPHICS);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_TOAST_ADDED, &GRAPHICS);

    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_STATE_CHANGED, &BLE_DEVICE);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_INFO_CHANGED, &BLE_DEVICE);
    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_HANDSET_NUMBER_RECEIVED, &BLE_DEVICE);

    DISPATCHER.addSubscriber(eMessageCode::MESSAGE_WEBSERVER_STATE_CHANGED, &webserverStopTracker);
}


void batteryControlTask(void* args) {

    while(1) {
        if (true == BATTERY_CONTROL_ENABLED and SMARTCASE.get_batteryPercent() < MIN_BATTERY_PERCENT_LAUNCH) {
            SMARTCASE.disableDisplayManager();
            SMARTCASE.safeSwitchOff(LANGUAGE_PACK[eTitleType::TITLE_SWITCHING_OFF]);
        }
        delay(BATTERY_CHECK_TASK_PERIOD);
    }
    vTaskDelete(NULL);
}
