#pragma once

#include "singletonTemplate.hpp"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "lvgl_classes.h"
#include "freertos/semphr.h"
#include "anc_modes.hpp"
#include "PairedDevice.hpp"
#include <map>
#include <deque>
#include <set>
#include "freertos/event_groups.h"
#include "ITask.hpp"
#include "MessageConsumer.hpp"

#define MAX_TOASTS_IN_LIST  5
#define DEVICE_NOT_IN_LIST  65535UL

LV_IMG_DECLARE(morph_img_black);
LV_IMG_DECLARE(scull_img_90_90);
LV_IMG_DECLARE(cassete_img_139_90);
LV_IMG_DECLARE(cat_img_135_90);

enum eTabScreens {
    SCREEN_MAIN,
    SCREEN_DEVICES,
    SCREEN_AUDIO,
    SCREEN_SETTINGS,
    SCREEN_MAX
};

enum eBatteryChargeLevel : uint8_t {
    BATTERY_EMPTY,
    BATTERY_LOW,
    BATTERRY_HALF,
    BATTERY_HIGH,
    BATTERY_FULL,
};

class GUI : public Singleton<GUI>, public ITask, public MessageConsumer{
    friend class Singleton;

    public:
        void init(xSemaphoreHandle mutex);
        void showLockPage();
        void showMainSreen();
        void setEarbudsChargeLevel(const int left, const int right);
        void setCaseChargeLevel(const uint8_t value);
        void setVolume(const uint8_t value);
        void setAncState(const bool value);
        void setAncDisabled(const bool disable);
        void setTranpState(const bool value);
        void setMicState(const bool value);
        void setConnectionState(const bool isConnected);
        void setConnectedDevice(const uint64_t mac, const bool isConnected);
        void setWebserverState(const bool value);
        void setPrimaryEarbudLeft();
        void setPrimaryEarbudRight();
        void createFactoryResetScreen();
        void setResetScreen(const std::string& step, const std::string& description, const bool cancelButton,
            const bool okButton, const bool nextButton);
        void hideResetScreen() {
            _resetPage->hide();
        }

        void updateSettings();

        void updateDevicesList();

        void updateLanguage();

        void updateLockscreenImage();

        void showLoadingPage(const std::string& txt, const bool spinner = true);
        void hideLoadingPage();
        void addToast(const std::string& msg);
        bool isLocked() const {
            return _isLocked;
        }

        EventGroupHandle_t eventGroup() const {
            return _factoryResetEventGroup;
        }

        uint16_t getPairedDeviceIndexInList(const PairedDevice& device)const;

    private:

        void run(void* args) override; 
        void addDevice(const PairedDevice& device);
        GUI();
        void setDeviceListItemsDefaultPosition();
        void showToast(const std::string& msg);
        void createLockPage();
        void createTabView();
        void createHomePage();
        void createDevicesPage();
        void createEarbudsPage();
        void createSettingsPage();
        void createWaitPage();
        void createToaster();
        void updateHomePageLanguage();
        void updateEarbudsPageLanguage();
        void updateSettingsPageLanguage();
        void updateLockPageLanguage();
        void updateToastsLanguage();
        void updateWaitPageLanguage();
        void updateResetPageLanguage();
        void deleteDevicesOutOfSet(const std::set<PairedDevice>& srcSet);
        // void realignExistingDevices();
        void addDevicesFromSet(const std::set<PairedDevice>& srcSet);
        void updateAncMode(const bool ancState, const eAncMode ancMode);
        void handleMessage(const message_t& msg);

    private:
        SemaphoreHandle_t _lvglMutex = nullptr;
        SemaphoreHandle_t _toastSmphr = nullptr;

        LVGLTabView* _tv = nullptr;
        LVGLPage* _devicesPage = nullptr;
        LVGLPage* _devicesColumn = nullptr;
        
        LVGLPage* _lockPage = nullptr;
        LVGLLabel* _unLockLabel = nullptr;

        LVGLPage* _waitPage = nullptr;
        LVGLLabel* _waitLabel = nullptr;
        LVGLSpinner* _waitSpinner = nullptr;

        LVGLArc* _leftCharge = nullptr;
        LVGLArc* _rightCharge = nullptr;
        LVGLLabel* _leftErbudLabel = nullptr;
        LVGLLabel* _rightErbudLabel = nullptr;
        LVGLLabel* _batteryLabel = nullptr;
        LVGLBatteryIndicator* _caseBattery = nullptr;

        LVGLLabel* _volumeLabel = nullptr;
        LVGLSlider* _volumeSlider = nullptr;
        LVGLLabel* _transparencyLabel = nullptr;
        LVGLSwitch* _transpSwitch = nullptr;
        LVGLLabel* _micLabel = nullptr;
        LVGLSwitch* _micSwitch = nullptr;
        LVGLLabel* _ancLabel = nullptr;
        LVGLSwitch* _ancSwitch = nullptr;
        LVGLButton* _wifiBtn = nullptr;
        
        LVGLButton* _addBtn = nullptr;
        LVGLButton* _deleteBtn = nullptr;
        LVGLToaster* _toaster = nullptr;

        LVGLLabel* _powerSavingLabel = nullptr;
        LVGLNumberBox* _powerSavingNumberBox = nullptr;
        LVGLLabel* _swipeSwitchLabel = nullptr;
        LVGLSwitch* _screenLockSwitch = nullptr;


        LVGLPage* _resetPage = nullptr;
        LVGLLabel* _resetPageStepLabel = nullptr;
        LVGLLabel* _resetPageDescription = nullptr;
        LVGLButton* _resetPageCancelButton = nullptr;
        LVGLButton* _resetPageOKButton = nullptr;
        LVGLButton* _resetPageNextButton = nullptr;
        LVGLImage* _lockscreenImage = nullptr;

        std::map<PairedDevice, LVGLButton*> _devicesBtn;
        std::deque<std::string> _toastList;
        bool _isLocked;
        EventGroupHandle_t _factoryResetEventGroup = nullptr;
};
#define GRAPHICS    GUI::instance()