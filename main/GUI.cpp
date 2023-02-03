#include <GUI.hpp>
#include "assert.h"
#include <ResourceAccessor.hpp>
#include <GUIConfig.hpp>
#include <GuiDefines.hpp>
#include <string>
#include <Utils/ThreadSafeDbg.hpp>
#include <DisplayManager.hpp>
#include <Settings.hpp>
#include <SmartCaseDarkTheme.hpp>
#include <BLETest.hpp>
#include <BLE.hpp>
#include <Volume.h>
#include <MorphDevice.hpp>
#include <PairedDevice.hpp>
#include <LVGL.hpp>
#include <SmartCase.hpp>
#include <AsyncFunctor.hpp>
#include <HttpWebserver.hpp>
#include <deque>
#include <AsyncFunctor.hpp>
#include <FactoryReset/FactoryResetEvents.hpp>
#include <FactoryReset/FactoryResetFSM.hpp>
#include <LanguagePackCollection.hpp>
#include <FontTypes.hpp>
#include <ScreenlockImageType.hpp>

using namespace Languages;

#define IMG_1       &scull_img_90_90
#define IMG_2       &cassete_img_139_90
#define IMG_3       &cat_img_135_90
#define IMG_DEFAULT &morph_img_black

static const char* TAG {"GUI"};
#define DEFAULT_DEVICE_ICON             LV_SYMBOL_BLUETOOTH
#define ADD_SPINNER_HIDE_TIME_S         5
#define DEVICE_ITEM_LIST_DRAG_POS_LEFT  -15
#define DEVICE_ITEM_LIST_DRAG_POS_RIGHT 370
#define DEVICE_ITEM_LIST_EDGE_PADDING   10
#define TRASH_BTN_WIDTH                 45
#define DEVICE_ITEM_LIST_VER_SPACE      9
#define DEVICE_ITEM_LIST_HEIGHT         37
#define DEVICE_ITEM_LIST_WIDTH          170

static uint64_t macToDeleteFromPaired{};
GUI::GUI()
    :  ITask("GUI", configMINIMAL_STACK_SIZE*5, 5),
       _isLocked(false) {
}

void GUI::run(void* args) {
    
    static message_t msg {};
    while(1) {
        consumeMessage(msg);
        // DBG_PRINT_TAG(TAG, "mesasge received: %u", msg.messageCode);
        handleMessage(msg);
    }
}

void GUI::handleMessage(const message_t& msg) {
    switch (msg.messageCode) {
        case MESSAGE_BLE_CONNECTION_STATE_CHANGED:
            setConnectionState(BLE_DEVICE.isConnected());
            if (BLE_DEVICE.isConnected()) {
                // addToast(LANGUAGE_PACK[eTitleType::TITLE_EARBUDS_CONNECTED]);
                // to prevent going to sleep if we were waiting for a connection
                lv_disp_trig_activity(nullptr);
            }
        break;

        case MESSAGE_EARBUDS_CHARGE_LEVEL_CHANGED:
            setEarbudsChargeLevel(MORPH_DEVICE.get_chargeLeft(), MORPH_DEVICE.get_chargeRight());
        break;

        case MESSAGE_ANC_STATE_CHANGED:
        case MESSAGE_ANC_MODE_CHANGED: {
            updateAncMode(MORPH_DEVICE.get_anc(), MORPH_DEVICE.get_ancMode());
            break;
        }

        case MESSAGE_MIC_STATE_CHANGED:
            setMicState(!MORPH_DEVICE.get_micMuteStatus());
        break;

        case MESSAGE_VOLUME_CHANGED:
            setVolume(MORPH_DEVICE.get_volume());
        break;

        case MESSAGE_HANDSET_INFO_CHANGED:
        case MESSAGE_HANDSET_LIST_CHANGED:
            updateDevicesList();
            setConnectedDevice(MORPH_DEVICE.get_handsetMac(), MORPH_DEVICE.get_handsetConnected());
        break;

        case MESSAGE_HANDSET_STATE_CHANGED:
            if (false == MORPH_DEVICE.get_handsetConnected()) {
                setConnectedDevice(MORPH_DEVICE.get_handsetMac(), false);
            }
        break;

        case MESSAGE_HANDSET_CONNECTION_STARTED:
            showLoadingPage(LANGUAGE_PACK[eTitleType::TITLE_PLEASE_WAIT]);
        break;

        case MESSAGE_HANDSET_CONNECTION_FINISHED:
            hideLoadingPage();
        break;

        case MESSAGE_HIDE_LOADING_PAGE:
            hideLoadingPage();
        break;

        case MESSAGE_WEBSERVER_STATE_CHANGED:
            setWebserverState(WEBSERVER.isActive());
            if (WEBSERVER.isActive()) {
                addToast(LANGUAGE_PACK[eTitleType::TITLE_WIFI_LAUNCHED]);
                addToast(LANGUAGE_PACK[eTitleType::TITLE_WEBSERVER_LAUNCHED]);
            }
            else {
                GRAPHICS.addToast(LANGUAGE_PACK[eTitleType::TITLE_WEBSERVER_STOPPED]);
            }
        break;

        case MESSAGE_PRIMARY_DEVICE_CHANGED:
            if (ePrimaryEarbud::EARBUD_UNKNOWN != MORPH_DEVICE.get_primaryEarbud()) {
                ePrimaryEarbud::EARBUD_LEFT == MORPH_DEVICE.get_primaryEarbud() ?
                setPrimaryEarbudLeft() : setPrimaryEarbudRight();
            }
        break;

        case MESSAGE_OTA_STARTED:
            showLoadingPage(LANGUAGE_PACK[eTitleType::TITLE_OTA_WAIT]);
        break;

        case MESSAGE_OTA_FAILED:
            hideLoadingPage();
            addToast(LANGUAGE_PACK[eTitleType::TITLE_OTA_FAILED]);
        break;

        case MESSAGE_TOAST_ADDED: {
            const char* const pMsg {msg.data.pStringData};
            addToast(pMsg);
        }
        break;    

        default:
            assert(false);
        break;
    }
}

void GUI::updateAncMode(const bool ancState, const eAncMode ancMode) {
    const bool isTranspOn {ancState and eAncMode::ANC_TRANSP == ancMode};
    setAncDisabled(isTranspOn);
    setTranpState(isTranspOn);
    if (!isTranspOn) {
        setAncState(ancState and eAncMode::ANC_ADAPTIVE == ancMode);
    }
}

void GUI::init(xSemaphoreHandle mutex) {

    _toastSmphr = xSemaphoreCreateBinary();
    assert(nullptr != _toastSmphr);

    _lvglMutex = mutex;
    ResourceAccessor accessor(_lvglMutex);

    static SmartCaseDarkTheme theme;
    theme.init();
    LVGLBase::setTheme(&theme);

    createTabView();
    createHomePage();
    createDevicesPage();
    createEarbudsPage();
    createSettingsPage();
    updateSettings(); 

    updateDevicesList();
    setConnectionState(false);

    createToaster();
    createLockPage();

    createFactoryResetScreen();
    createWaitPage();

    std::function<void()> toasterTaskHandler = [this]() {
        while (1) {
        
            if (_toastList.size()) {
                std::string msg {_toastList.back()};
                _toastList.pop_back();
                showToast(msg);
                xSemaphoreTake(_toastSmphr, portMAX_DELAY);
            }
            delay(500);
        }
    };

    static AsyncFunctor toasterTask(toasterTaskHandler, "toasterTask", configMINIMAL_STACK_SIZE*3, 1);
    toasterTask.start();

    _factoryResetEventGroup = xEventGroupCreate();
    assert(nullptr != _factoryResetEventGroup);

    updateLanguage();
}

void GUI::addToast(const std::string& msg) {
    _toastList.push_front(msg);
    if (_toastList.size() > MAX_TOASTS_IN_LIST) {
        DBG_PRINT_TAG(TAG, "toasts list overflow!");
        _toastList.pop_back();
    }
    // DBG_PRINT_TAG(TAG, "toast added, total toasts in list: %u", _toastList.size());
}


void GUI::setVolume(const uint8_t value) {
    ResourceAccessor accessor(_lvglMutex);
    _volumeSlider->setValue(value);
}

void GUI::setAncState(const bool value) {
    ResourceAccessor accessor(_lvglMutex);
    _ancSwitch->setValue(value);
}

void GUI::setAncDisabled(const bool disable) {
    ResourceAccessor accessor(_lvglMutex);
    _ancSwitch->setState(disable ? LV_STATE_DISABLED : LV_STATE_DEFAULT);
}

void GUI::setTranpState(const bool value) {
    ResourceAccessor accessor(_lvglMutex);
    _transpSwitch->setValue(value);
}

void GUI::setMicState(const bool value) {
    ResourceAccessor accessor(_lvglMutex);
    _micSwitch->setValue(value);
}

void GUI::updateSettings() {
    _powerSavingNumberBox->setValue(SETTINGS.displayOffPeriod());
    _screenLockSwitch->setValue(SETTINGS.isSwipeToUnlockActive());
}

void GUI::createTabView() {   
    ResourceAccessor accessor(_lvglMutex);
    _tv = new LVGLTabView();
    _tv->setSize(LV_HOR_RES, /*LV_VER_RES*/240);
    _tv->setNavigationBarsPosition(LV_TABVIEW_TAB_POS_LEFT);
    _tv->setAnimationTime(0);
    _tv->setIndicatorWidth(0);

    auto onTabChangedCb = [this](const uint16_t index) {
        setDeviceListItemsDefaultPosition();

        switch (index) {
            case eTabScreens::SCREEN_MAIN:
                setCaseChargeLevel(SMARTCASE.get_batteryPercent());
                if (BLE_DEVICE.isConnected()) {     
                    std::function<void()> bleAsynReq = []() {
                        BLE_DEVICE.requestUpdateChargeLevel();
                        BLE_DEVICE.requestPrimaryDevice();
                    };

                    static AsyncFunctor bleTask(bleAsynReq, "async_requestUpdateHomePage", configMINIMAL_STACK_SIZE*3);
                    if (!bleTask.isRunning()) {
                        bleTask.start();
                    }
                }
            break;
        
            case eTabScreens::SCREEN_DEVICES:
                setConnectedDevice(MORPH_DEVICE.get_handsetMac(), MORPH_DEVICE.get_handsetConnected());
                if (BLE_DEVICE.isConnected()) {     
                        std::function<void()> bleAsynReq = []() {
                        BLE_DEVICE.requestPairedDevicesNumber();
                        // BLE_DEVICE.requestAllPairedDevices();
                        BLE_DEVICE.requestUpdateHandsetState();
                    };

                    static AsyncFunctor bleTask(bleAsynReq, "async_requestUpdateHandsetState", configMINIMAL_STACK_SIZE*3);
                    if (!bleTask.isRunning()) {
                        bleTask.start();
                    }
                }                    
            break;

            case eTabScreens::SCREEN_AUDIO:
            break;

            default:
            break;
        }
    };
    _tv->onTabChanged(onTabChangedCb);
}

void GUI::createWaitPage() {
    ResourceAccessor accessor(_lvglMutex);
    _waitPage = new LVGLPage();
    _waitPage->hide();
    _waitPage->setSize(LV_HOR_RES, 240);
    _waitPage->align(_waitPage->parent(), LV_ALIGN_IN_TOP_MID, 0, 0);
    _waitPage->setStyleBgColor(LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    _waitPage->setStyleBgOpacity(LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_70);
    LVGLColumn* column {new LVGLColumn(_waitPage)};
    column->setStyleBgOpacity(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    _waitLabel = new LVGLLabel("", column);
    _waitLabel->setStyleBgOpacity(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    _waitLabel->setAlignMode(LV_LABEL_ALIGN_CENTER);
    _waitLabel->setLabelLongMode(LV_LABEL_LONG_BREAK);
    _waitLabel->setSize(250, 50);
    
    _waitSpinner = new LVGLSpinner(500, column);
    _waitSpinner->setSize(70, 70);

    column->align(_waitPage, LV_ALIGN_CENTER, 0, 0);
}


void GUI::createHomePage() {
    ResourceAccessor accessor(_lvglMutex);
    LVGLPage* homePage = _tv->addTab(LV_SYMBOL_HOME);
    homePage->setScrollPropogation(false);
    homePage->setPaddings(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 10, 0, 0, 0);
    homePage->setInnerPadding(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    // homePage->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    LVGLColumn* columnHome = new LVGLColumn(homePage);
    columnHome->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);
    columnHome->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

    LVGLRow* rowChargeEarbuds = new LVGLRow(columnHome);
    rowChargeEarbuds->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);

    LVGLColumn* leftEarbudColumn = new LVGLColumn(rowChargeEarbuds);
    leftEarbudColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

    LVGLColumn* rightEarbudColumn = new LVGLColumn(rowChargeEarbuds);
    rightEarbudColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

    _leftCharge = new LVGLArc(false, 270, 630, "%", leftEarbudColumn);
    _leftCharge->setSize(90, 90);
    _leftErbudLabel = new LVGLLabel("", leftEarbudColumn);
    _leftErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _leftErbudLabel->setStyleBgOpacity(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    _leftErbudLabel->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10, 5);
    _leftErbudLabel->setStyleRadius(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    _rightCharge = new LVGLArc(false, 270, 630, "%", rightEarbudColumn);
    _rightCharge->setSize(90, 90);
    _rightErbudLabel = new LVGLLabel("", rightEarbudColumn);
    _rightErbudLabel->setStyleBgOpacity(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    _rightErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _rightErbudLabel->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10, 5);
    _rightErbudLabel->setStyleRadius(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    LVGLColumn* batteryColumn = new LVGLColumn(columnHome);
    batteryColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);
    batteryColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);

    _caseBattery = new LVGLBatteryIndicator(eBatteryIndicatorSize::BATTERY_SIZE_HUGE, LV_COLOR_WHITE, batteryColumn);

    LVGLButton* _secretBtn = new LVGLButton("", homePage);
    _secretBtn->resetStyle(LV_BTN_PART_MAIN);
    _secretBtn->setSize(100, 100);
    _secretBtn->setStyleBgOpacity(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    _secretBtn->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    _secretBtn->align(homePage, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

    static size_t secretBtnPressCntr{0};
    auto secretCb = [this, &secretBtnPressCntr](LVGLButton* btn) {
        static const size_t triggerCntr {(SECRET_BTN_TRIGGER_PERIOD_MS - LV_INDEV_DEF_LONG_PRESS_TIME) / LV_INDEV_DEF_LONG_PRESS_REP_TIME};
        if (secretBtnPressCntr++ >= triggerCntr) {
            DBG_PRINT_TAG(TAG, "secret button triggered!");
            BLE_DEVICE.stop();
            WEBSERVER.stop();
            FACTORY_RESET_FSM.start();
            secretBtnPressCntr = 0;
        }
    };
    _secretBtn->onLongPressedRepeat(secretCb);

    auto onReleasedCb = [&secretBtnPressCntr](LVGLButton* btn) {
        DBG_PRINT_TAG(TAG, "secret button activity reseted");
        secretBtnPressCntr = 0;
    };
    _secretBtn->onReleased(onReleasedCb);

    _batteryLabel = new LVGLLabel("", batteryColumn);

    auto onChangedArc = [](AbstractValueChangable<int>* obj) {

        LVGLArc* base = static_cast<LVGLArc*>(obj);
        if (obj->value() <= BATTERY_RED_LEVEL) {
            base->setStyleLineColor(LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
        }
        else if (obj->value() <= BATTERY_ORANGE_LEVEL) {
            base->setStyleLineColor(LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
        }
        else if (obj->value() <= BATTERY_YELLOW_LEVEL) {
            base->setStyleLineColor(LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
        }
        else {
            base->setStyleLineColor(LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_LIME);
        }
    };
    _leftCharge->onChanged(onChangedArc);
    _leftCharge->setValue(0);

    _rightCharge->onChanged(onChangedArc);
    _rightCharge->setValue(0);
    columnHome->align(homePage, LV_ALIGN_CENTER, 0, 0);
}

void GUI::setDeviceListItemsDefaultPosition() {
    size_t btnIndex{};
    for (std::map<PairedDevice, LVGLButton*>::iterator it = _devicesBtn.begin(); it != _devicesBtn.end(); ++it) {
        LVGLButton* btn {it->second};
        const lv_coord_t verticalOffset = 10 + (DEVICE_ITEM_LIST_HEIGHT + DEVICE_ITEM_LIST_VER_SPACE) * btnIndex++;
        btn->alignHorizontally(_devicesColumn, LV_ALIGN_IN_TOP_MID, 0);
        btn->alignVertically(lv_page_get_scrollable(_devicesColumn->innerData()), LV_ALIGN_IN_TOP_MID, verticalOffset);
        it->second->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
        it->second->show();
    }
    _deleteBtn->hide();
}

void GUI::createDevicesPage() {
    ResourceAccessor accessor(_lvglMutex);
    _devicesPage = _tv->addTab(LV_SYMBOL_BLUETOOTH);
    _devicesPage->setScrollPropogation(false);
    _devicesPage->setPaddings(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0, 0, 0, 0);
    _devicesPage->setInnerPadding(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    // _devicesPage->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    _devicesColumn = new LVGLPage(_devicesPage);
    _devicesColumn->setPaddings(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0, 0, 0, 0);
    _devicesColumn->setInnerPadding(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    _devicesColumn->setSize(_devicesPage->width(), 200);
    _devicesColumn->setScrollableFit(LV_FIT_NONE, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_TIGHT);
    // _devicesColumn->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);


    auto onFocused = [this](LVGLBase* src) {
        setDeviceListItemsDefaultPosition();        
    };
    _devicesPage->onFocused(onFocused);
    _devicesColumn->onFocused(onFocused);

    _devicesColumn->setScrollBarMode(LV_SCROLLBAR_MODE_AUTO);

    _addBtn = new LVGLButton("+", _devicesPage);
    _addBtn->setSize(80, 30);
    _addBtn->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,  &lv_font_montserrat_24);
    _addBtn->setFont(LV_LABEL_PART_MAIN, LV_STATE_DISABLED,  &lv_font_montserrat_24);


    _deleteBtn = new LVGLButton(LV_SYMBOL_TRASH, _devicesColumn);
    _deleteBtn->hide();
    _deleteBtn->setSize(TRASH_BTN_WIDTH, DEVICE_ITEM_LIST_HEIGHT);
    _deleteBtn->align(_devicesColumn, LV_ALIGN_IN_RIGHT_MID, -15, 0);

    auto onAddCb = [this](LVGLButton* btn) {
        setDeviceListItemsDefaultPosition();

        if (true == BLE_DEVICE.isConnected()){

            if (MORPH_DEVICE.pairedDeviceList().size() >= MAX_PAIRED_DEVICES) {
                addToast(LANGUAGE_PACK[eTitleType::TITLE_MAX_DEVICES]);
                return;
            }

            std::function<void()> bleAsynReq = [this]() {
                addToast(BLE_DEVICE.requestPairingMode() ? LANGUAGE_PACK[eTitleType::TITLE_PAIRING_MODE_ACTIVATED] :
                    LANGUAGE_PACK[eTitleType::TITLE_REQUEST_FAILED]);
            };

            static AsyncFunctor bleTask(bleAsynReq, "async_requestPairingMode", configMINIMAL_STACK_SIZE*3);
            if (!bleTask.isRunning()) {
                bleTask.start();
            }
        }
    };
    _addBtn->onClicked(onAddCb);

    auto onDeleteDevice = [this, onAddCb](LVGLBase* btnActor) {   
        setDeviceListItemsDefaultPosition();
        DBG_PRINT_TAG(TAG, "Deleting paired device...");
        if (macToDeleteFromPaired == MORPH_DEVICE.get_handsetMac() and true == MORPH_DEVICE.get_handsetConnected()) {
            addToast(LANGUAGE_PACK[eTitleType::TITLE_DISCONNECT_FIRST]);
            return;
        }

        if (BLE_DEVICE.isConnected()) {     
            std::function<void()> bleAsynReq = [this]() {
                addToast(BLE_DEVICE.requestDeletePairedHandset(macToDeleteFromPaired) ? LANGUAGE_PACK[eTitleType::TITLE_COMMAND_SENT] :
                    LANGUAGE_PACK[eTitleType::TITLE_REQUEST_FAILED]);
            };

            static AsyncFunctor bleTask(bleAsynReq, "async_requestDeletePairedHandsetl", configMINIMAL_STACK_SIZE*3);
            if (!bleTask.isRunning()) {
                bleTask.start();
            }
        }
    };

    _deleteBtn->onClicked(onDeleteDevice);

    _devicesColumn->align(_devicesPage, LV_ALIGN_IN_TOP_MID, 0, 0);
    _addBtn->align(_devicesPage, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
}

void GUI::updateDevicesList() {
    ResourceAccessor accessor(_lvglMutex);
    std::set<PairedDevice> actualDeviceSet {MORPH_DEVICE.pairedDeviceList()};

    deleteDevicesOutOfSet(actualDeviceSet);
    addDevicesFromSet(actualDeviceSet);
    setDeviceListItemsDefaultPosition();
}

void GUI::deleteDevicesOutOfSet(const std::set<PairedDevice>& srcSet) {
    for (auto &device : _devicesBtn) {
        if (srcSet.end() == srcSet.find(device.first)) {
            delete device.second;
            _devicesBtn.erase(device.first);
        }
    }
}

// void GUI::realignExistingDevices() {
//     size_t btnIndex{};
//     for (std::map<PairedDevice, LVGLButton*>::iterator deviceItem = _devicesBtn.begin(); deviceItem != _devicesBtn.end(); ++deviceItem) {
//         const lv_coord_t verticalOffset = 10 + (DEVICE_ITEM_LIST_HEIGHT + DEVICE_ITEM_LIST_VER_SPACE) * btnIndex++;
//         LVGLButton* btn {deviceItem->second};
//         btn->alignVertically(lv_page_get_scrollable(_devicesColumn->innerData()), LV_ALIGN_IN_TOP_MID, verticalOffset);
//         btn->alignHorizontally(_devicesColumn, LV_ALIGN_IN_TOP_MID, 0);
//     }
// }

void GUI::addDevicesFromSet(const std::set<PairedDevice>& srcSet) {
    for (std::set<PairedDevice>::iterator it = srcSet.begin(); it != srcSet.end(); ++it) {
        addDevice(*it);
    }
}

// void GUI::deleteAllDevices() {
//     ResourceAccessor accessor(_lvglMutex);
//     DBG_PRINT_TAG(TAG, "deleting all devices..\r\n");
//     for (std::map<PairedDevice, LVGLButton*>::iterator deviceItem = _devicesBtn.begin(); deviceItem != _devicesBtn.end(); ++deviceItem) {
//         delete deviceItem->second;
//     }
//     _devicesBtn.clear();
// }

void GUI::updateLanguage() {
    updateLockPageLanguage();
    updateWaitPageLanguage();
    updateResetPageLanguage();
    updateToastsLanguage();
    
    updateHomePageLanguage();
    updateEarbudsPageLanguage();
    updateSettingsPageLanguage();
}

void GUI::updateLockPageLanguage() {
    _unLockLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_18]);
    _unLockLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_SWIPE_TO_UNLOCK]);
}

void GUI::updateWaitPageLanguage() {
    _waitLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_24]);
}

void GUI::updateResetPageLanguage() {
    _resetPageCancelButton->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _resetPageCancelButton->setLabelText(LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_CANCEL]);

    _resetPageNextButton->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _resetPageNextButton->setLabelText(LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_NEXT]);

    _resetPageOKButton->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _resetPageOKButton->setLabelText(LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_OK]);

    _resetPageDescription->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
}

void GUI::updateToastsLanguage() {
    _toaster->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
}


void GUI::updateHomePageLanguage() {
    
    _leftErbudLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _leftErbudLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT]);

    _rightErbudLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _rightErbudLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT]);

    _batteryLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _batteryLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_CASE_BATTERY]);
}

void GUI::updateEarbudsPageLanguage() {

    _volumeLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _volumeLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_VOLUME]);

    _transparencyLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _transparencyLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_TRANSPARENCY]);

    _micLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _micLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_MIC]);

    _ancLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _ancLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_ANC]);
}

void GUI::updateSettingsPageLanguage() {

    _powerSavingLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _powerSavingLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_POWER_SAVING_AFTER]);

    _powerSavingNumberBox->setLabelFont(LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _powerSavingNumberBox->setLabelText(LANGUAGE_PACK[Languages::eTitleType::TITLE_SEC]);

    _swipeSwitchLabel->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_PACK[eFontType::FONT_NORMAL_14]);
    _swipeSwitchLabel->setText(LANGUAGE_PACK[Languages::eTitleType::TITLE_UNLOCK]);
}

void GUI::addDevice(const PairedDevice& device) {
    ResourceAccessor accessor(_lvglMutex);
    DBG_PRINT_TAG(TAG, "adding new device %s\r\n", device.name);
    const std::string label {stringUtils::format("%s %s",DEFAULT_DEVICE_ICON, device.name)};
    auto it = _devicesBtn.find(device);

    if (it != _devicesBtn.end()) {
        it->second->setLabelText(label);
        return;
    }

    LVGLButton* newBtn { new LVGLButton(label, _devicesColumn)};
    newBtn->hide();
    
    _devicesBtn[device] = newBtn;
    newBtn->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    newBtn->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_BTN_STATE_CHECKED_RELEASED, 0);
    newBtn->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DISABLED, 0);

    newBtn->setSize(DEVICE_ITEM_LIST_WIDTH, DEVICE_ITEM_LIST_HEIGHT);
    newBtn->setDragEnabled(true);
    newBtn->setDragDirection(LV_DRAG_DIR_HOR);
    
    // const lv_coord_t verticalOffset = (10 + (DEVICE_ITEM_LIST_HEIGHT + DEVICE_ITEM_LIST_VER_SPACE) * (_devicesBtn.size() - 1));
    
    // DBG_PRINT_TAG(TAG, "new btn vertical offset: %d", verticalOffset);
    // DBG_PRINT_TAG(TAG, "devices: %u", _devicesBtn.size());
    // newBtn->align(lv_page_get_scrollable(_devicesColumn->innerData()), LV_ALIGN_IN_TOP_MID, 0, verticalOffset);

    auto onDragStopped = [this](LVGLBase* btnActor) {   
        DBG_PRINT_TAG(TAG, "onDragger triggered");

        const lv_area_t area{btnActor->coordinates()};
        setDeviceListItemsDefaultPosition();

        bool edgeTriggered{false};
        if (area.x1 < DEVICE_ITEM_LIST_DRAG_POS_LEFT) {
            btnActor->alignHorizontally(_devicesColumn, LV_ALIGN_IN_LEFT_MID, DEVICE_ITEM_LIST_EDGE_PADDING);
            btnActor->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
            _deleteBtn->setY(btnActor->y());
            _deleteBtn->alignHorizontally(_devicesColumn, LV_ALIGN_IN_RIGHT_MID, -(DEVICE_ITEM_LIST_EDGE_PADDING + 5));
            _deleteBtn->show();
            edgeTriggered = true;
        }
        else if (area.x2 > DEVICE_ITEM_LIST_DRAG_POS_RIGHT) {
            btnActor->alignHorizontally(_devicesColumn, LV_ALIGN_IN_RIGHT_MID, -(DEVICE_ITEM_LIST_EDGE_PADDING + 5));
            btnActor->setStyleBorderWidth(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
            _deleteBtn->setY(btnActor->y());
            _deleteBtn->alignHorizontally(_devicesColumn, LV_ALIGN_IN_LEFT_MID, DEVICE_ITEM_LIST_EDGE_PADDING);
            _deleteBtn->show();
            edgeTriggered = true;
        }

        if (true == edgeTriggered) {
            for (std::map<PairedDevice, LVGLButton*>::iterator it = _devicesBtn.begin(); it != _devicesBtn.end(); ++it) {
                if (it->second == btnActor) {
                    macToDeleteFromPaired = it->first.macAddress;
                }
                else {
                    it->second->hide();
                }            
            }
        }
    };
    auto onClicked = [this](LVGLButton* btnActor)
    {   
        for (std::map<PairedDevice, LVGLButton*>::iterator it = _devicesBtn.begin(); it != _devicesBtn.end(); ++it) {
            if (it->second == btnActor) {
                if (MORPH_DEVICE.get_handsetMac() != it->first.macAddress or 
                    false == MORPH_DEVICE.get_handsetConnected()) {
                    BLE_DEVICE.startHandsetConnectionTask(it->first);
                }
                else {
                    addToast(true == BLE_DEVICE.requestDisconnectHandset() ? LANGUAGE_PACK[eTitleType::TITLE_DISCONNECTING] :
                        LANGUAGE_PACK[eTitleType::TITLE_REQUEST_FAILED]);
                }
                return;
            }
        }
    };

    newBtn->onClicked(onClicked);
    newBtn->onDragStopped(onDragStopped);
}

uint16_t GUI::getPairedDeviceIndexInList(const PairedDevice& device)const {
    ResourceAccessor accessor(_lvglMutex);
    const auto distance = std::distance(_devicesBtn.begin(), _devicesBtn.find(device));
    return distance == _devicesBtn.size() ? DEVICE_NOT_IN_LIST : distance;
}

void GUI::createToaster() {
    _toaster = new LVGLToaster(_toastSmphr);
    _toaster->align(_toaster->parent(), LV_ALIGN_CENTER, 0, 0);
    _toaster->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _toaster->setStyleBgColor(LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_AQUA);
    _toaster->setStyleBgOpacity(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    _toaster->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10, 10);
}

void GUI::showToast(const std::string& msg) {
    _toaster->setToast(msg);
}

void GUI::setConnectedDevice(const uint64_t mac, const bool isConnected) {
    ResourceAccessor accessor(_lvglMutex);
    lv_coord_t connectedDeviceY{};
    for (std::map<PairedDevice, LVGLButton*>::iterator itGui = _devicesBtn.begin(); itGui != _devicesBtn.end(); ++itGui) {
        const bool isDeviceConnected {itGui->first.macAddress == mac and isConnected};
        if (isDeviceConnected) {
            connectedDeviceY = itGui->second->y();
        }
        itGui->second->setStyleBgColor(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, isDeviceConnected ? GUI_HIGHLIGTH_COLOR : GUI_BG_COLOR);
        itGui->second->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, isDeviceConnected ? GUI_BG_COLOR : GUI_MAIN_COLOR);
    }

    lv_obj_t* scrollable = lv_page_get_scrl(_devicesColumn->innerData());
    const lv_coord_t scrollableY = -lv_obj_get_y(scrollable);
    DBG_PRINT_TAG(TAG, "scrollable y: %d", scrollableY);

    DBG_PRINT_TAG(TAG, "connected Device y: %d", connectedDeviceY);

    const int triggerForScrollUpCoordinate {scrollableY + _devicesColumn->height() - DEVICE_ITEM_LIST_HEIGHT - DEVICE_ITEM_LIST_VER_SPACE};
    const int triggerForScrollDownCoordinate {scrollableY + DEVICE_ITEM_LIST_VER_SPACE};

    if (connectedDeviceY and connectedDeviceY > triggerForScrollUpCoordinate) {
        const int scrollValue {connectedDeviceY - triggerForScrollUpCoordinate};
        DBG_PRINT_TAG(TAG, "need to scroll up by %d", scrollValue);
        _devicesColumn->scrollVertically(-scrollValue);
    }
    else if (connectedDeviceY and connectedDeviceY < triggerForScrollDownCoordinate) {
        const int scrollValue {connectedDeviceY - triggerForScrollDownCoordinate};
        DBG_PRINT_TAG(TAG, "need to scroll down by %d", scrollValue);
        _devicesColumn->scrollVertically(-scrollValue);
    }
}

void GUI::showLoadingPage(const std::string& txt, const bool spinner) {
    ResourceAccessor accessor(_lvglMutex);
    _waitLabel->setText(txt);
    _waitPage->show();
    spinner ? _waitSpinner->show() : _waitSpinner->hide();
}

void GUI::hideLoadingPage() {
    ResourceAccessor accessor(_lvglMutex);
    _waitPage->hide();
}

void GUI::createFactoryResetScreen() {
    ResourceAccessor accessor(_lvglMutex);

    _resetPage = new LVGLPage;
    _resetPage->hide();
    _resetPage->setSize(LV_HOR_RES, LV_VER_RES);
    _resetPage->setScrollableFit(LV_FIT_NONE);
    _resetPage->setStyleBgColor(LV_PAGE_PART_BG, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _resetPage->setStyleBgOpacity(LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

    _resetPageDescription = new LVGLLabel("", _resetPage);
    _resetPageDescription->setAutoRealign(true);
    _resetPageDescription->setAlignMode(LV_LABEL_ALIGN_CENTER);
    _resetPageDescription->setLabelLongMode(LV_LABEL_LONG_BREAK);
    _resetPageDescription->setSize(280, 50);
    _resetPageDescription->setRecolor(true);

    _resetPageStepLabel = new LVGLLabel("", _resetPage);
    _resetPageStepLabel->setAlignMode(LV_LABEL_ALIGN_CENTER);
    _resetPageStepLabel->setAutoRealign(true);


    LVGLRow* btnsRow {new LVGLRow(_resetPage)};
    btnsRow->setAutoRealign(true);
    _resetPageCancelButton = new LVGLButton("Cancel", btnsRow);
    _resetPageOKButton = new LVGLButton("OK", btnsRow);
    _resetPageNextButton = new LVGLButton("Next", btnsRow);

    auto cancelCb = [this](LVGLButton* btn) {
        DBG_PRINT_TAG(TAG, "cancel button clicked!");
        FACTORY_RESET_FSM.stop();
        delay(100);
        hideResetScreen();
        BLE_DEVICE.start();
    };
    _resetPageCancelButton->onClicked(cancelCb);

    auto nextCb = [this](LVGLButton* btn) {
        DBG_PRINT_TAG(TAG, "NEXT button clicked!");
        xEventGroupSetBits(_factoryResetEventGroup, NEXT_BIT);
    };
    _resetPageNextButton->onClicked(nextCb);

    _resetPageOKButton->onClicked(cancelCb);

    _resetPageStepLabel->align(_resetPage, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
    _resetPageDescription->align(_resetPage, LV_ALIGN_CENTER, 0, -15);
    btnsRow->align(_resetPage, LV_ALIGN_IN_BOTTOM_MID, 0, -15);
}

void GUI::setResetScreen(const std::string& step, const std::string& description, const bool cancelButton,
            const bool okButton, const bool nextButton) {

    _resetPageStepLabel->setText(step);
    _resetPageDescription->setText(description);
    cancelButton ? _resetPageCancelButton->show() : _resetPageCancelButton->hide();
    okButton ? _resetPageOKButton->show() : _resetPageOKButton->hide();
    nextButton ? _resetPageNextButton->show() : _resetPageNextButton->hide();

    _resetPage->show();
}

void GUI::createEarbudsPage() {
    ResourceAccessor accessor(_lvglMutex);
    // earbuds page
    LVGLPage* earbudsPage = _tv->addTab(LV_SYMBOL_AUDIO);
    earbudsPage->setScrollPropogation(false);
    earbudsPage->setPaddings(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 10, 0, 0, 0);
    earbudsPage->setInnerPadding(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    // earbudsPage->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    LVGLColumn* earbudsColumn = new LVGLColumn(earbudsPage);
    earbudsColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    earbudsColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0, 0, 0);
    // earbudsColumn->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

    static const size_t innerPadding{6};
    LVGLColumn *volumeColumn {new LVGLColumn(earbudsColumn)};
    volumeColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, innerPadding + 4);
    volumeColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5, 15, 20, 20);
    // volumeColumn->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

    _volumeLabel = new LVGLLabel("", volumeColumn);
    _volumeSlider = new LVGLSlider(MIN_VOLUME_VALUE, MAX_VOLUME_VALUE, VOLUME_CHANGE_STEP, volumeColumn);
    _volumeSlider->setSize(210, 25);

    auto onVolumeChanged = [](const int value) {
        const bool requestRest {BLE_DEVICE.requestVolumeChange(value)};
        DBG_PRINT_TAG(TAG, requestRest ?
                "volume change request sent" : "failed to send volume change request");
           
    };
    _volumeSlider->onMoved(onVolumeChanged);

    LVGLRow* row{ new LVGLRow(earbudsColumn)};
    row->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);

    LVGLColumn *transpColumn {new LVGLColumn(row)};
    transpColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, innerPadding);
    transpColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);
    // transpColumn->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);


    LVGLColumn *micColumn {new LVGLColumn(row)};
    micColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, innerPadding);
    micColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0);
    // micColumn->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);


    _transparencyLabel = new LVGLLabel("", transpColumn);
    _transpSwitch = new LVGLSwitch(100, transpColumn);
    auto onTranspTaped = [this](bool value) {

        if(value) {
            const bool requestRes {BLE_DEVICE.requestAncModeChange(eAncMode::ANC_TRANSP)};
            DBG_PRINT_TAG(TAG, requestRest ?
                "anc mode transp request send" : "failed to send anc mode transp request");
            if (!MORPH_DEVICE.get_anc()) {
                const bool requestRes {BLE_DEVICE.requestAncStateChange(true)};
                DBG_PRINT_TAG(TAG, requestRes ?
                    "anc on request send" : "failed to send anc on request");
            }
        }

        else {
            if (!this->_ancSwitch->value()) {
                const bool requestRes { BLE_DEVICE.requestAncStateChange(false)};
                DBG_PRINT_TAG(TAG, requestRes ?
                    "anc off request send" : "failed to send anc off request");
            }
            const bool requestRes { BLE_DEVICE.requestAncModeChange(eAncMode::ANC_ADAPTIVE)};
            DBG_PRINT_TAG(TAG, requestRes ?
                "anc mode adaptive request sent" : "failed to send anc mode adaptive request");
        }
    };
    _transpSwitch->onToggled(onTranspTaped);
    _transpSwitch->setSize(90, 40);

    _micLabel = new LVGLLabel("", micColumn);
    _micSwitch = new LVGLSwitch(100, micColumn);
    _micSwitch->setSize(90, 40);
    
    
    auto onMicTaped = [](bool value) {
        const bool requestRes { BLE_DEVICE.requestMicToggle() };        
        DBG_PRINT_TAG(TAG, requestRes ?
                "mic toggle request send" : "failed to send mic toggle request");
    };
    _micSwitch->onToggled(onMicTaped);

    LVGLColumn *ancColumn {new LVGLColumn(earbudsColumn)};
    ancColumn->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, innerPadding);
    ancColumn->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3, 3);
    
    _ancLabel = new LVGLLabel("", ancColumn);
    _ancSwitch = new LVGLSwitch(100, ancColumn);
    _ancSwitch->setSize(90, 40);
    auto onAncTaped = [](bool value) {
        const bool requestRes { BLE_DEVICE.requestAncStateChange(value) };
        DBG_PRINT_TAG(TAG, requestRes ?
                "anc on/off request send" : "failed to send anc on/off request");
    };
    _ancSwitch->onToggled(onAncTaped);

    earbudsColumn->align(earbudsPage, LV_ALIGN_CENTER, 0, 0);
}

void GUI::createSettingsPage() {
    ResourceAccessor accessor(_lvglMutex);
     // settings page
    LVGLPage* settingsPage = _tv->addTab(LV_SYMBOL_SETTINGS);
    settingsPage->setScrollPropogation(false);
    settingsPage->setPaddings(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 10, 0, 0, 0);
    settingsPage->setInnerPadding(LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    // settingsPage->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    LVGLColumn* columnSettings = new LVGLColumn(settingsPage);
    columnSettings->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    columnSettings->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0, 0, 0, 0);
    // columnSettings->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

    _powerSavingLabel = new LVGLLabel("", columnSettings);
    _powerSavingNumberBox = new LVGLNumberBox(15, 60, 5, "", columnSettings);
    _powerSavingNumberBox->setSize(200, 50);
    _powerSavingNumberBox->setButtonFont(LV_STATE_DEFAULT, &lv_font_montserrat_26);
    _powerSavingNumberBox->setButtonFont(LV_STATE_DISABLED, &lv_font_montserrat_26);

    LVGLRow* bottomRow = new LVGLRow(columnSettings);
    // bottomRow->setStyleBgColor(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

    LVGLColumn* swipeColumn = new LVGLColumn(bottomRow);
    
    _swipeSwitchLabel = new LVGLLabel("", swipeColumn);
    _screenLockSwitch = new LVGLSwitch(250, swipeColumn);
    _screenLockSwitch->setSize(90, 40);

    _wifiBtn = new LVGLButton(LV_SYMBOL_WIFI, bottomRow);
    _wifiBtn->setSize(65,65);
    _wifiBtn->setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_26);
    _wifiBtn->setFont(LV_LABEL_PART_MAIN, LV_STATE_DISABLED, &lv_font_montserrat_26);
    _wifiBtn->setStyleRadius(LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 5);
    _wifiBtn->setStyleRadius(LV_BTN_PART_MAIN, LV_STATE_DISABLED, 5);
    _wifiBtn->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_SECONDARY_COLOR);

    
    auto onChangedCbBx = [](AbstractValueChangable<int>* obj) {
        DBG_PRINT_TAG(TAG,"nmbbx changed:%d", obj->value());
        SETTINGS.setDisplayOffPeriod(obj->value());
        SETTINGS.save();
    };

    auto onTapWifiCb = [this](LVGLButton* btn) {
        if (WEBSERVER.isActive()) {
            WEBSERVER.stop();
            // BLE_DEVICE.start();
        }
        else {
            std::function<void()> asyncWebserverStart = [btn]() {
                btn->disable();
                BLE_DEVICE.stop();
                WEBSERVER.start();
                btn->enable();
            };
            
            static AsyncFunctor webserverStartTask(asyncWebserverStart, "asyncWebserverStart", configMINIMAL_STACK_SIZE*3);
            if (!webserverStartTask.isRunning()) {
                webserverStartTask.start();
            }
        }
    };
    _wifiBtn->onClicked(onTapWifiCb);
    
    auto onChangedCbSw = [](bool value) {
        DBG_PRINT_TAG(TAG, "switch changed: %d", value);
        SETTINGS.setSwipeToUnlock(value);
        SETTINGS.save();
    };    
    _powerSavingNumberBox->onChanged(onChangedCbBx);
    _screenLockSwitch->onToggled(onChangedCbSw);
    columnSettings->align(settingsPage, LV_ALIGN_CENTER, 0, 0);
}

void GUI::createLockPage() {
    ResourceAccessor accessor(_lvglMutex);
    _lockPage = new LVGLPage();
    _lockPage->hide();
    _lockPage->setSize(LV_HOR_RES, LV_VER_RES);
    _lockPage->setClickable(false);

    _lockscreenImage = new LVGLImage(nullptr, _lockPage);
    updateLockscreenImage();

    LVGLColumn* column_lockPage = new LVGLColumn(_lockPage);
    column_lockPage->setPaddings(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 25, 10);
    column_lockPage->setInnerPadding(LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 15);

    _unLockLabel = new LVGLLabel("", column_lockPage);
    LVGLUnlocker* ul = new LVGLUnlocker(column_lockPage);
    ul->setStyleBgColor(LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, GUI_BG_COLOR);

    lv_obj_set_style_local_bg_grad_color(ul->innerData(), LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, GUI_HIGHLIGTH_COLOR);
    lv_obj_set_style_local_bg_grad_dir(ul->innerData(), LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_GRAD_DIR_HOR);
    lv_obj_set_style_local_bg_main_stop(ul->innerData(), LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, ul->width() * 0.1);
    lv_obj_set_style_local_bg_grad_stop(ul->innerData(), LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, ul->width() * 0.95);
    
    ul->setSize(LV_HOR_RES * 0.75, 25);
        auto onUnlockCb = [this](LVGLUnlocker* obj) {
        showMainSreen();
    };

    ul->onUnlock(onUnlockCb);

    column_lockPage->align(_lockPage, LV_ALIGN_IN_BOTTOM_MID, 0, -30);
}

void GUI::updateLockscreenImage() {
    const uint8_t preset{SETTINGS.lockScreenImage()};

    switch (preset){
        case eScreenlockImage::IMAGE_CUSTOM_1:
            _lockscreenImage->updateImage(IMG_1);
        break;

        case eScreenlockImage::IMAGE_CUSTOM_2:
            _lockscreenImage->updateImage(IMG_2);
        break;

        case eScreenlockImage::IMAGE_CUSTOM_3:
            _lockscreenImage->updateImage(IMG_3);
        break;
    
        default:
            _lockscreenImage->updateImage(IMG_DEFAULT);
        break;
    }
    
    _lockscreenImage->align(_lockPage, LV_ALIGN_IN_TOP_MID, 0, 30);
}

void GUI::showLockPage() {
    ResourceAccessor accessor(_lvglMutex);
    if (!_isLocked) {
        _tv->hide();
        _lockPage->show();
    }
    _isLocked = true;
}

void GUI::showMainSreen() {
    ResourceAccessor accessor(_lvglMutex);
    if (_isLocked) {
        _lockPage->hide();
        _tv->show();
    }
    _isLocked = false;
}

void GUI::setEarbudsChargeLevel(const int left, const int right) {
    ResourceAccessor accessor(_lvglMutex);
    _leftCharge->setValue(left);
    _rightCharge->setValue(right);
}

void GUI::setPrimaryEarbudLeft() {
    _leftErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);
    _leftErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);

    _rightErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _rightErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);
}

void GUI::setPrimaryEarbudRight() {
    _rightErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);
    _rightErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);

    _leftErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
    _leftErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);
}

void GUI::setCaseChargeLevel(const uint8_t value) {
    ResourceAccessor accessor(_lvglMutex);
    _caseBattery->setValue(value);
}

void GUI::setWebserverState(const bool value) {
    _wifiBtn->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, value ? GUI_HIGHLIGTH_COLOR : GUI_SECONDARY_COLOR);
}

void GUI::setConnectionState(const bool isConnected) {
    ResourceAccessor accessor(_lvglMutex);
    const uint8_t state {isConnected ? LV_STATE_DEFAULT : LV_STATE_DISABLED};

    // if QCC was connected to any handset - now it's invalid information
    if (!isConnected) {
        setConnectedDevice(MORPH_DEVICE.get_handsetMac(), false);
    }

    _leftCharge->setState(state);
    _rightCharge->setState(state);
    _ancSwitch->setState(state);
    _transpSwitch->setState(state);
    _micSwitch->setState(state);
    _volumeSlider->setState(state);

    setDeviceListItemsDefaultPosition();
    for (auto it : _devicesBtn) {
        isConnected ? it.second->enable() : it.second->disable();
    }

    _leftCharge->setState(state);
    _rightCharge->setState(state);

    isConnected ? _addBtn->enable() : _addBtn->disable();

    if (!isConnected) {
        _rightErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
        _rightErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);

        _leftErbudLabel->setStyleBgColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_BG_COLOR);
        _leftErbudLabel->setStyleTextColor(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, GUI_MAIN_COLOR);
    }
}
