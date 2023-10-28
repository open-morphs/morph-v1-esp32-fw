#pragma once

#include "singletonTemplate.hpp"
#include "ITask.hpp"
#include "GAIA.hpp"
#include "anc_modes.hpp"
#include "freertos/event_groups.h"
#include "PairedDevice.hpp"
#include "macUtils.hpp"
#include "BLE/BLEAdvertisedDevice.h"
#include "BLE/BLEClient.h"
#include "MessageConsumer.hpp"
#include "MessageProducer.hpp"
#include "MorphDevice.hpp"

enum eVendorType {
  VENDOR_V3QC,
  VENDOR_MORPH
};

class AddvertiseCbHandler : public ESP32Kolban::BLEAdvertisedDeviceCallbacks {
    void onResult(ESP32Kolban::BLEAdvertisedDevice advertisedDevice) override;
};

class SecurityCbHandler : public ESP32Kolban::BLESecurityCallbacks {
  uint32_t onPassKeyRequest();
  void onPassKeyNotify(uint32_t pass_key);
  bool onConfirmPIN(uint32_t pass_key);
  bool onSecurityRequest();
  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl);
};

class ClientCbHandler : public ESP32Kolban::BLEClientCallbacks {
  void onConnect(ESP32Kolban::BLEClient* pclient);
  void onDisconnect(ESP32Kolban::BLEClient* pclient);
};

class BLE : public Singleton<BLE>, public ITask, public MessageProducer, public MessageConsumer {
  friend class Singleton;
  friend class SecurityCbHandler;
  friend class AddvertiseCbHandler;
  friend class ClientCbHandler;
  friend void connectToHandsetTask(void* args);
  public:
    void setServerDevice(const ESP32Kolban::BLEAdvertisedDevice& device);
    void setServerMacType(const esp_ble_addr_type_t type);
    bool deleteBondingInformation();

    void startHandsetConnectionTask(const PairedDevice& device);
    bool sendCommand(const eVendorType vendor, const uint8_t cmd, const uint8_t feature, const uint8_t* data = nullptr, const size_t dataLen = 0);
    bool sendRawData(uint8_t* data, const size_t dataLen);
    bool isConnected() const;
    bool isMorphMacUnknown();
    bool isMorphDeviceFound(); 
    // requests
    bool updateAll();
    bool requestUpdateVolume();
    bool requestUpdateAncState();
    bool requestUpdateMicState();
    bool requestUpdateHandsetState();
    bool requestUpdateHandsetInfo();
    bool requestUpdateAncMode();
    bool requestUpdateChargeLevel();
    bool requestVolumeChange(const uint8_t value);
    bool requestAncStateChange(const bool value);
    bool requestAncModeChange(const eAncMode value);
    bool requestDisconnectHandset();
    bool requestPairingMode();
    bool requestConnectToHandset(const uint64_t mac, const uint32_t mask);
    bool requestMicToggle();
    bool requestNotificationsRegister(const uint8_t feature);
    bool requestDeletePairedHandset(const uint64_t mac);
    // bool requestAllPairedDevices();
    bool requestPrimaryDevice();
    bool requestPairedDevicesNumber();
    bool requestPairedDevicesList(const size_t indexFrom, const size_t amount);
    bool requestBeep(const uint8_t numberOfBeeps);

    // void requestAllPairedHandsets();

    void changeCurrentUsedMac();
    void setMacSwapEnabled(const bool value);

  private:
    BLE();
    void run(void* args) override;
    void init();
    void handleMessage(const message_t& msg);
    void startScan(const size_t period);
    void stopScan();
    bool connectToServer();
    void disconnectFromServer();
    void deinit();
    bool sendPacket(ESP32Kolban::BLERemoteCharacteristic* characteristic, const PduPacket& packet, const uint8_t* data = nullptr, const size_t dataLen = 0);
    void onStop() override;
    static void responseNotifyCb(ESP32Kolban::BLERemoteCharacteristic* pBLERemoteCharacteristic,
      uint8_t* pData, size_t length, bool isNotify);
    static void handleMorphVendorPacket(const uint8_t featureID, const ePduTypes pduType, const uint8_t specID,
      const uint8_t* data, const size_t length);
    static void handleMorphResponse(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length);
    static void handleMorphNotification(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length);
    static void handleQCCV3Packet(const uint8_t featureID, const ePduTypes pduType, const uint8_t specID,
      const uint8_t* data, const size_t length);
    static void handleQCCV3Response(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length);
    static void handleQCCV3Notification(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length);
    
    void notifyToast(const char* msg);
    void notifyAncState(const bool state);
    void notifyAncMode(const eAncMode mode);
    void notifyConnectionState(const bool state);
    void notifyPrimaryEarbud(const ePrimaryEarbud value);
    void notifyVolume(const uint8_t value);
    void notifyEarbudsCharge();
    void notifyMicState(const bool state);
    void notifyHandsetInfo();
    void notifyHandsetListUpdate();
    void notifyHandsetState(const bool state);
    void notifyHandsetConnectionStarted();
    void notifyHandsetConnectionFinished();
    void notifyHandsetNumberReceived(const uint8_t number);
  
  private:
    AddvertiseCbHandler* _addCbHandler;
    SecurityCbHandler* _securityCbHandler;
    ClientCbHandler* _clientCbHandler;
    ESP32Kolban::BLESecurity* _security;
    ESP32Kolban::BLEClient* _client;
    ESP32Kolban::BLEAdvertisedDevice _morphDevice;
    ESP32Kolban::BLERemoteCharacteristic* _commandChar;
    ESP32Kolban::BLERemoteCharacteristic* _responseChar;
    ESP32Kolban::BLEAddress _morphMac;
    esp_ble_addr_type_t _macType;
    uint8_t _defaultMacAddress[MAC_LENGTH]{};
    uint8_t _optionalMacAddress[MAC_LENGTH]{};
    uint8_t* _currentUsedMac;
    size_t _failedConnections;
    bool _isProperlyConnected;
    bool _macSwapEnabled = true;
};

#define BLE_DEVICE BLE::instance()
