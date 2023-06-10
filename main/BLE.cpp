#include <BLE.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include <GAIA.hpp>
#include <MorphDevice.hpp>
#include <NVS.hpp>
#include <Settings.hpp>
#include <Events.hpp>
#include <MacUtils.hpp>
#include "BLE/BLEDescriptor.h"
#include "BLE/BLEDevice.h"
#include "BLE/BLEUtils.h"
#include "BLE/BLEServer.h"
#include "BLE/BLE2902.h"
#include "BLE/BLECharacteristic.h"
#include "BLE/BLEUUID.h"
#include <LanguagePackCollection.hpp>
#include <AsyncFunctor.hpp>
#include <GUI.hpp>

static const char* const TAG {"BLE"};
static const char* const MORPH_DEVICE_NAME{"LE-Morph InfiniConnect"};
// The GAIA service UUID and characteristic UUIDs
static ESP32Kolban::BLEUUID gaiaServiceUUID("00001100-D102-11E1-9B23-00025B00A5A5");
static ESP32Kolban::BLEUUID commandCharUUID("00001101-D102-11E1-9B23-00025B00A5A5");
static ESP32Kolban::BLEUUID responseCharUUID("00001102-D102-11E1-9B23-00025B00A5A5");
// Vendor IDs 
static const GaiaVendor_t v3QCVendorID {0x00, 0x1D};
static const GaiaVendor_t morphVendorID {0x0A, 0x66};
using namespace Languages;


#define BLE_SCAN_INTERVAL_MS        40UL
#define BLE_WINDOW_INTERVAL_MS      30UL
#define BLE_SCAN_PERIOD             15UL
#define BLE_TASK_PERIOD_MS          500UL
#define BLE_PAIRING_TIMEOUT_MS      15000UL

static SemaphoreHandle_t pairingSmphr{};

EventGroupHandle_t _eventGroup {nullptr};

BLE::BLE()
  :   _addCbHandler{nullptr},
      _securityCbHandler{nullptr},
      _clientCbHandler{nullptr},
      _client{nullptr},
      _commandChar{nullptr},
      _responseChar{nullptr},
      _morphMac{""},
      _macType{esp_ble_addr_type_t::BLE_ADDR_TYPE_PUBLIC},
      _failedConnections{},
      _isProperlyConnected{false} {

  pairingSmphr = xSemaphoreCreateBinary();
  assert(nullptr != pairingSmphr);
  setName(TAG);
  setStackSize(configMINIMAL_STACK_SIZE*4);
  setPriority(1);
  setPeriod(BLE_TASK_PERIOD_MS);

  _securityCbHandler = new SecurityCbHandler;
  _addCbHandler = new AddvertiseCbHandler;

  esp_efuse_mac_get_default(_defaultMacAddress);
  memcpy(_optionalMacAddress, _defaultMacAddress, MAC_LENGTH);
  _optionalMacAddress[MAC_LENGTH - 1] ++;

  _currentUsedMac = SETTINGS.isBleMacDefault() ? _defaultMacAddress : _optionalMacAddress;

  _eventGroup = xEventGroupCreate();
  assert(nullptr != _eventGroup);
}

void BLE::init() {
  DBG_PRINT_TAG(TAG, "initializing..");

  _morphDevice = {};
  _morphMac = ESP32Kolban::BLEAddress{SETTINGS.bondedMac()};
  _macType = static_cast<esp_ble_addr_type_t>(SETTINGS.bondedMacType());

  ESP_ERROR_CHECK(esp_base_mac_addr_set(_currentUsedMac));

  DBG_PRINT_TAG(TAG, "used mac: %02X:%02X:%02X:%02X:%02X:%02X", _currentUsedMac[0], _currentUsedMac[1],
    _currentUsedMac[2], _currentUsedMac[3], _currentUsedMac[4], _currentUsedMac[5]);

  assert (true == ESP32Kolban::BLEDevice::init(SETTINGS.isBleMacDefault() ? "MorphCase-D" : "MorphCase-O"));
  ESP32Kolban::BLEDevice::getScan()->setAdvertisedDeviceCallbacks(static_cast<ESP32Kolban::BLEAdvertisedDeviceCallbacks*>(_addCbHandler));
  ESP32Kolban::BLEDevice::getScan()->setActiveScan(true);
  ESP32Kolban::BLEDevice::getScan()->setInterval(BLE_SCAN_INTERVAL_MS);
  ESP32Kolban::BLEDevice::getScan()->setWindow(BLE_WINDOW_INTERVAL_MS);

  _clientCbHandler = new ClientCbHandler;
  _client = ESP32Kolban::BLEDevice::createClient();
  _client->setClientCallbacks(_clientCbHandler, true);

  _security = new ESP32Kolban::BLESecurity;
  _security->setAuthenticationMode(ESP_LE_AUTH_REQ_BOND_MITM);
  _security->setCapability(ESP_IO_CAP_NONE);
  _security->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  ESP32Kolban::BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  ESP32Kolban::BLEDevice::setSecurityCallbacks(_securityCbHandler);
}


bool BLE::deleteBondingInformation() {
  DBG_PRINT_TAG(TAG, "Deleting all bonding data...");

  SETTINGS.setBondedMac("00:00:00:00:00:00");
  SETTINGS.setBondedMacType(esp_ble_addr_type_t::BLE_ADDR_TYPE_PUBLIC);
  SETTINGS.save();

  const int bondDevicesNum {esp_ble_get_bond_device_num()};

  if (ESP_FAIL == bondDevicesNum) {
    DBG_PRINT_TAG(TAG, "failed to delete bonding info, ble is not initialized!");
    return false;
  }
  
  DBG_PRINT_TAG(TAG, "%d bonded devices found", bondDevicesNum);

  if (bondDevicesNum > 0) {
      esp_ble_bond_dev_t* bondedDevices {new esp_ble_bond_dev_t[bondDevicesNum]};
      int devicesNum {bondDevicesNum};
      esp_ble_get_bond_device_list(&devicesNum, bondedDevices);
      DBG_PRINT_TAG(TAG, "bonded devices list length: %d ", devicesNum);
      for (size_t i = 0; i < devicesNum; i++) {
          DBG_PRINTF("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", bondedDevices[i].bd_addr[0],
              bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2], bondedDevices[i].bd_addr[3],
              bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
          DBG_PRINTLN();
          ESP_ERROR_CHECK(esp_ble_remove_bond_device((bondedDevices + i)->bd_addr));
      }
      delete[] bondedDevices;
  }

  return true;
}

void BLE::setMacSwapEnabled(const bool value) {
  DBG_PRINT_TAG("BLE", "%s mac swap", value ? "ENABLING" : "DISABLING");
  _macSwapEnabled = value;
}

void BLE::changeCurrentUsedMac() {
  DBG_PRINT_TAG(TAG, "changing mac..");
  _currentUsedMac = (_currentUsedMac == _defaultMacAddress) ? _optionalMacAddress : _defaultMacAddress;
}

void BLE::deinit() {
  DBG_PRINT_TAG(TAG, "deinitializing..");
  _isProperlyConnected = false;
  disconnectFromServer();
  stopScan();

  if (_security)  
    delete _security;  
  if (_client) 
    delete _client;

  _client = nullptr;
  _security = nullptr;  
  _commandChar = nullptr;
  _responseChar = nullptr;

  ESP32Kolban::BLEDevice::deinit(false);
  delay(200 * 2);
}

void BLE::run(void* args) {

  init();

  while(1) {

    if (true == isMorphMacUnknown()) {

      while (!isMorphDeviceFound()) {
        notifyToast(LANGUAGE_PACK[eTitleType::TITLE_EARBUDS_SEARCHING]);  
        startScan(BLE_SCAN_PERIOD);
      }

      SETTINGS.setBondedMac(_morphDevice.getAddress().toString());
      SETTINGS.setBondedMacType(_morphDevice.getAddressType());
      SETTINGS.save();
      _morphMac = ESP32Kolban::BLEAddress{_morphDevice.getAddress().toString()};
    }

    if (false == isConnected()) {

      if(false == connectToServer()) {
        deinit();
        if (_macSwapEnabled) {
          // DBG_PRINT_TAG(TAG, "Changing mac-address");
          changeCurrentUsedMac();
        }
        init();
      }
      else {
        SETTINGS.setBleMacDefault(_currentUsedMac == _defaultMacAddress);
        SETTINGS.save();
        if (!updateAll()) {
          DBG_PRINT_TAG(TAG, "failed to request all updates..");
        }
      }
    }

    static message_t msg{};
    const bool wasMessageReceived {consumeMessage(msg, _periodMs)};
    if (true == wasMessageReceived) {
      handleMessage(msg);
    }
    // DBG_PRINT_TAG(TAG, "BLE run loop delaying %u ms", _periodMs);
  }
}

void BLE::handleMessage(const message_t& msg) {

  static size_t emptyNameCounter{};
  static uint64_t emptyNameMac{};
  static const uint8_t maxEmptyName{3};

  // DBG_PRINT_TAG(TAG, "handling message: %u", msg.messageCode);

  if(false == isConnected()) {
    return;
  }

  switch (msg.messageCode) {
    case MESSAGE_HANDSET_STATE_CHANGED:
        requestUpdateHandsetInfo();
    break;

    case MESSAGE_HANDSET_INFO_CHANGED: {

      const bool isHandSetNameEmpty {MORPH_DEVICE.get_handsetName().length() == 0};

      if (false == isHandSetNameEmpty) {
        emptyNameCounter = 0;
        break;
      }

      if (emptyNameMac != MORPH_DEVICE.get_handsetMac()) {
        emptyNameCounter = 0;
        emptyNameMac = MORPH_DEVICE.get_handsetMac();
      }
      
      emptyNameCounter++;

      auto bleAsynReq = []() {
        delay(2000);
        BLE_DEVICE.requestUpdateHandsetInfo();
      };

      static AsyncFunctor bleTask(bleAsynReq, "async_requestUpdateHandsetInfo", configMINIMAL_STACK_SIZE*3);

      if (true == isHandSetNameEmpty and emptyNameCounter <= maxEmptyName) {
        if (false == bleTask.isRunning()) {
          DBG_PRINTF("Handset name seems to be empty, let's request it's name again");
          bleTask.start();
        }
      }

      break;
    }

    case MESSAGE_HANDSET_NUMBER_RECEIVED: {
      uint8_t totalDevicesToRequest {msg.data.ui8Param};
      uint8_t indexFrom {};

      DBG_PRINT_TAG(TAG, "%u paired handsets received!", (size_t)totalDevicesToRequest);

      while (totalDevicesToRequest) {
        const size_t devicesToRequset {totalDevicesToRequest > MAX_HANDSET_IN_PAYLOAD ?
        MAX_HANDSET_IN_PAYLOAD : totalDevicesToRequest};
        DBG_PRINT_TAG(TAG, "requesting %u handsets, from index %u", devicesToRequset, indexFrom);
        requestPairedDevicesList(indexFrom, devicesToRequset);
        totalDevicesToRequest -= devicesToRequset;
        indexFrom += devicesToRequset;
      }
      break;
    }

    default:
      assert(false);
    break;
  }
}

bool BLE::isMorphMacUnknown() {
  return _morphMac.equals(ESP32Kolban::BLEAddress{"00:00:00:00:00:00"});
}

bool BLE::isMorphDeviceFound() {
  return MORPH_DEVICE_NAME == _morphDevice.getName();
}

void BLE::onStop() {
  disconnectFromServer();
  deinit();
}

void BLE::setServerDevice(const ESP32Kolban::BLEAdvertisedDevice& device) {
  _morphDevice = device;
}

void BLE::setServerMacType(const esp_ble_addr_type_t type) {
  _macType = type;
}

void BLE::stopScan() {
  DBG_PRINT_TAG(TAG, "Stopping scan..");
  ESP32Kolban::BLEDevice::getScan()->stop();
}

void BLE::startScan(const size_t period) {
  DBG_PRINT_TAG(TAG, "Starting scan..");
  
  ESP32Kolban::BLEDevice::getScan()->clearResults();
  ESP32Kolban::BLEDevice::getScan()->start(period, false);
}

bool BLE::connectToServer() {
  notifyToast(LANGUAGE_PACK[eTitleType::TITLE_EARBUDS_CONNECTION]);
  DBG_PRINT_TAG(TAG, "Connecting to server: %s, type: %u", _morphMac.toString().c_str(), _macType);
  // esp_ble_gap_set_prefer_conn_params(*_morphMac.getNative(), 100, 200, 100, 3100);

  const bool connectionRes {_client->connect(_morphMac, _macType/*, CONNECTION_TIMEOUT_MS*/)};
  DBG_PRINT_TAG(TAG, connectionRes ? "connected" : "failed");

  if (!connectionRes) {
    return false;
  }

  DBG_PRINT_TAG(TAG, "waiting for pairing...");
  const bool isPaired {xSemaphoreTake(pairingSmphr, BLE_PAIRING_TIMEOUT_MS)};

  if (isPaired) {
    DBG_PRINT_TAG(TAG, "paired successfully!");
  }
  else {
    DBG_PRINT_TAG(TAG, "pairing timeout");
    disconnectFromServer();
    return false;
  }

  ESP32Kolban::BLERemoteService* pRemoteService {_client->getService(gaiaServiceUUID)};
  if (nullptr == pRemoteService) {
    DBG_PRINT_TAG(TAG, "Failed to find service UUID: %s", gaiaServiceUUID.toString().c_str());
    disconnectFromServer();
    return false;
  }
  DBG_PRINT_TAG(TAG, "Service found!");

  vTaskDelay(pdMS_TO_TICKS(100));

  _commandChar = pRemoteService->getCharacteristic(commandCharUUID);
  if (nullptr == _commandChar) {
    DBG_PRINT_TAG(TAG, "Failed to find Command Characteristic UUID: %", commandCharUUID.toString().c_str());
    disconnectFromServer();
    return false;
  }
  DBG_PRINT_TAG(TAG, "Command Characteristic found!");

  _responseChar = pRemoteService->getCharacteristic(responseCharUUID);
  if (nullptr == _responseChar) {
    DBG_PRINT_TAG(TAG, "Failed to find Response Characteristic UUID: %s", responseCharUUID.toString().c_str());
    disconnectFromServer();
    return false;
  }
  // DBG_PRINT_TAG(TAG, "Response Characteristic found!");


  if (!_commandChar->canWrite()) {
    DBG_PRINT_TAG(TAG, "Command Characteristic is not writable!");
    disconnectFromServer();
    return false;
  }
  // DBG_PRINT_TAG(TAG, "Command Characteristic is ok!");


  if (!_responseChar->canNotify()) {
    DBG_PRINT_TAG(TAG, "Response Characteristic is not notifiable!");
    disconnectFromServer();
    return false;
  }
  // DBG_PRINT_TAG(TAG, "Response Characteristic is ok!");

  _responseChar->registerForNotify(responseNotifyCb);

  delay(1000);
  if (false == _isProperlyConnected) {
    _isProperlyConnected = true;
    notifyConnectionState(true);
  }

  if (!requestNotificationsRegister(static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_APPLICATION))) {
    DBG_PRINT_TAG(TAG, "Failed to send register for earbuds application feature notificataions!");
  }
  if (!requestNotificationsRegister(static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION))) {
    DBG_PRINT_TAG(TAG, "Failed to send register for audio curation feature notificataions!");
  }

  // DBG_PRINT_TAG(TAG, "Done");
  
  return true;
}

void BLE::responseNotifyCb(ESP32Kolban::BLERemoteCharacteristic* pBLERemoteCharacteristic,
      uint8_t* pData, size_t length, bool isNotify) {


  // DBG_PRINT_TAG(TAG, "Notify callback for characteristic: %s,\r\ndata length: %u\r\n",
    // pBLERemoteCharacteristic->getUUID().toString().c_str(), length);

  if (length < PDU_PACKET_LENGTH) {
    DBG_PRINT_TAG(TAG, "Data length is invalid!");
    return;
  }

  // for (size_t i = 0; i < length; i++) {
  //     DBG.printHex(*(pData + i));
  // }
  // DBG_PRINTLN("");

  PduPacket packet {};
  packet.fillFromArray(pData);

  DBG_PRINTLN();
  // DBG_PRINT_TAG(TAG, "Packet received:\r\nvendor:0x%x%x, featureID: %u, pduType: %u, commandID: %u", packet.get_vendor().firstByte, 
    // packet.get_vendor().secondByte, packet.get_featureID(), packet.get_pduType(), packet.get_specID());

  if (packet.get_vendor() == morphVendorID) {
    // DBG_PRINT_TAG(TAG, "morph vendor packet detected..");
    if (packet.get_featureID() >= static_cast<uint8_t> (eMorphFeatures::MAX)) {
      DBG_PRINT_TAG(TAG, "invalid morph feature id: %u", packet.get_featureID());
      return;
    }
    handleMorphVendorPacket(packet.get_featureID(), packet.get_pduType(), packet.get_specID(), pData, length);
  }
  else if (packet.get_vendor() == v3QCVendorID) {
    // DBG_PRINT_TAG(TAG, "v3QCVendorID packet detected..");
    if (packet.get_featureID() >= static_cast<uint8_t> (eQccV3Features::QCC_V3_FEATURE_MAX)) {
      DBG_PRINT_TAG(TAG, "invalid v3QCVendorID feature id: %u", packet.get_featureID());
      return;
    }
    handleQCCV3Packet(packet.get_featureID(), packet.get_pduType(), packet.get_specID(), pData, length);
  }
}

void BLE::handleQCCV3Packet(const uint8_t featureID, const ePduTypes pduType, const uint8_t specID,
  const uint8_t* data, const size_t length) {

  switch (pduType) {
    case ePduTypes::PDU_TYPE_RESPONSE:
      handleQCCV3Response(featureID, specID, data, length);
    break;

    case ePduTypes::PDU_TYPE_NOTIFICATION:
      handleQCCV3Notification(featureID, specID, data, length);
    break;
  
    default:
      DBG_PRINT_TAG(TAG, "not supported pdu type: %u", pduType);
    break;
  }
}

void BLE::handleQCCV3Response(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length) {

  switch (featureID) {
    case static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION):
      switch (specID) {
        case static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_GET_STATE): {
          if (length < (PDU_PACKET_LENGTH + 2)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const bool ancState {data[PDU_PACKET_LENGTH + 1]};
          DBG_PRINT_TAG(TAG, "new anc state received: %s", ancState ? "on" : "off");
          MORPH_DEVICE.set_anc(ancState);
          BLE_DEVICE.notifyAncState(ancState);
          break;
        }
        case static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_GET_CURR_MODE): {
          if (length < (PDU_PACKET_LENGTH + 4)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t ancMode {data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new anc mode received: %u", ancMode);

          MORPH_DEVICE.set_ancMode(static_cast<eAncMode>(ancMode));
          BLE_DEVICE.notifyAncMode(static_cast<eAncMode>(ancMode));
          break;
        }        
        default:
          DBG_PRINT_TAG(TAG, "unknown commands id: %u", specID);
        break;
      }
    break;
    case static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_APPLICATION):
      switch (specID) {
        case static_cast<uint8_t>(eQccV3Commands_Application::QCC_V3_COMMAND_WHICH_PRIMARY): {
            if (length < (PDU_PACKET_LENGTH + 1)) {
              DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
              break;
            }
            const uint8_t primaryDevice {data[PDU_PACKET_LENGTH]};
            DBG_PRINT_TAG(TAG, "new primary device received: %u", primaryDevice);
            MORPH_DEVICE.set_primaryEarbud(primaryDevice == 0 ? ePrimaryEarbud::EARBUD_LEFT : ePrimaryEarbud::EARBUD_RIGHT);
            BLE_DEVICE.notifyPrimaryEarbud(primaryDevice == 0 ? ePrimaryEarbud::EARBUD_LEFT : ePrimaryEarbud::EARBUD_RIGHT);
            break;
          }
        break;
        default:
          DBG_PRINT_TAG(TAG, "unknown commands id: %u", specID);
        break;
      }
    break;
    default:
      DBG_PRINT_TAG(TAG, "unknown feature: %u", featureID);
    break;
  }
}

void BLE::handleQCCV3Notification(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length) {
        
  switch (featureID) {
    case static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION):
      switch (specID) {
        case static_cast<uint8_t>(eQccV3Notifications_AudioCuration::QCC_V3_NOTIFICATION_STATE_CHANGED): {
          if (length < (PDU_PACKET_LENGTH + 2)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const bool ancState {data[PDU_PACKET_LENGTH + 1]};
          DBG_PRINT_TAG(TAG, "new anc state received: %s", ancState ? "on" : "off");
          MORPH_DEVICE.set_anc(ancState);
          BLE_DEVICE.notifyAncState(ancState);
          break;
        }
        case static_cast<uint8_t>(eQccV3Notifications_AudioCuration::QCC_V3_NOTIFICATION_MODE_CHANGED): {
          if (length < (PDU_PACKET_LENGTH + 4)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t ancMode {data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new anc mode received: %u", ancMode);
          MORPH_DEVICE.set_ancMode(static_cast<eAncMode>(ancMode));
          BLE_DEVICE.notifyAncMode(static_cast<eAncMode>(ancMode));
          break;
        }
        default:
          DBG_PRINT_TAG(TAG, "unknown notification id: %u", specID);
        break;
      }
    break;

    case static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_APPLICATION): {
      switch (specID) {
        case static_cast<uint8_t>(eQccV3Notifications_EarbudApplication::QCC_V3_NOTIFICATION_PRIMARY_ABOUT_TO_CHANGE):
          DBG_PRINT_TAG(TAG, "primary earbud is about to change RECEIVED!"); 
        break;
        case static_cast<uint8_t>(eQccV3Notifications_EarbudApplication::QCC_V3_NOTIFICATION_PRIMARY_CHANGED):
          DBG_PRINT_TAG(TAG, "primary earbud is changed RECEIVED!"); 
        break;
        default:
          DBG_PRINT_TAG(TAG, "unknown notification id: %u", specID);
        break;
      }
    }
    break;

    default:
      DBG_PRINT_TAG(TAG, "unknown feature: %u", featureID);
    break;
  }
}

static std::set<uint64_t> _receivedHandsetsMac;
static size_t _numberOfHandsetsToReceive = 0;

void BLE::handleMorphVendorPacket(const uint8_t featureID, const ePduTypes pduType, const uint8_t specID,
      const uint8_t* data, const size_t length) {
  
  switch (pduType) {
    case ePduTypes::PDU_TYPE_RESPONSE:
      handleMorphResponse(featureID, specID, data, length);
    break;

    case ePduTypes::PDU_TYPE_NOTIFICATION:
      handleMorphNotification(featureID, specID, data, length);
    break;
  
    default:
      DBG_PRINT_TAG(TAG, "not supported pdu type: %u", pduType);
    break;
  }
}

void BLE::handleMorphResponse(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length) {

  switch (featureID) {
    case static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID):
      switch (specID) {
        // volume
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_VOLUME): {
          if (length < (PDU_PACKET_LENGTH + 1)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t volume{data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new volume received: %u", volume);
          MORPH_DEVICE.set_volume(volume);
          BLE_DEVICE.notifyVolume(volume);
          break;
        }
        // battery level
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_BATTERY_LEVEL): {
          if (length < (PDU_PACKET_LENGTH + 2)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t leftLevel{ data[PDU_PACKET_LENGTH]};
          const uint8_t rightLevel{ data[PDU_PACKET_LENGTH + 1]};

          DBG_PRINT_TAG(TAG, "new battery levels received: %u, %u", leftLevel, rightLevel);
          MORPH_DEVICE.set_chargeLeft(leftLevel);
          MORPH_DEVICE.set_chargeRight(rightLevel);
          BLE_DEVICE.notifyEarbudsCharge();
          break;
        }
        // mic state
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_MIC_MUTE_STATE): {
          if (length < (PDU_PACKET_LENGTH + 1)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const bool micState{data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new mic state received: %s", micState ? "on" : "off");
          MORPH_DEVICE.set_micMuteStatus(micState);
          BLE_DEVICE.notifyMicState(micState);
          break;
        }
        // handset connected info
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_HANDSET_CONNECTED): {

          if (length < (PDU_PACKET_LENGTH + MAC_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t* const pMac {&data[PDU_PACKET_LENGTH]};
          const uint8_t* const pBtProfileMask {&data[PDU_PACKET_LENGTH + MAC_LENGTH]};
          const char* const pName {reinterpret_cast<const char*>(&data[PDU_PACKET_LENGTH + MAC_LENGTH + BT_PROFILE_MASK_LENTH])};

          const std::string newHandsetname {std::string(pName, length - MAC_LENGTH - PDU_PACKET_LENGTH - BT_PROFILE_MASK_LENTH)};
          MORPH_DEVICE.set_handsetName(newHandsetname);
          const uint64_t receivedMac {MacUtils::macFromRawBytes(pMac)};
          const uint64_t receivedProfileMask {MacUtils::profileMaskFromRawBytes(pBtProfileMask)};

          MORPH_DEVICE.set_handsetMac(receivedMac);
          MORPH_DEVICE.set_btProfileMask(receivedProfileMask);

          DBG_PRINT_TAG(TAG, "new handset name received: %s", newHandsetname.c_str());
          DBG_PRINT_TAG(TAG, "new handset mac received: 0x%llx", MORPH_DEVICE.get_handsetMac());
          DBG_PRINT_TAG(TAG, "new btProfileMask received: 0x%lx", MORPH_DEVICE.get_btProfileMask());

          PairedDevice device {};
          device.macAddress = receivedMac;
          device.btProfileMask = receivedProfileMask;
          memcpy(device.name, newHandsetname.c_str(), newHandsetname.length() > MAX_DEVICE_NAME_LENGTH ? MAX_DEVICE_NAME_LENGTH : newHandsetname.length());
          
          if (true == MORPH_DEVICE.addPairedDevice(device)) {
            BLE_DEVICE.notifyHandsetInfo();
          }

          break;
        }
        // handset connection status
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_IS_HANDSET_CONNECTED): {
          if (length < (PDU_PACKET_LENGTH + 1)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const bool handsetState{data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new handset state received: %s", handsetState ? "connected" : "disconnected");
          MORPH_DEVICE.set_handsetConnected(handsetState);
          BLE_DEVICE.notifyHandsetState(handsetState);
          break;
        }
        // result of deletion of paired device
        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_DELETE_PAIRED_HANDSET): {
          if (length < (PDU_PACKET_LENGTH + 1 + MAC_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          static const uint8_t DELETED_RESULT{0};
          const bool wasHandsetDeleted{data[PDU_PACKET_LENGTH] == DELETED_RESULT};

          DBG_PRINT_TAG(TAG, "deletion of paired handset result received: %s", wasHandsetDeleted ? "success" : "failed");

          const uint8_t* const pMac {&data[PDU_PACKET_LENGTH + 1]};
          uint64_t macToDeleteFromPaired {MacUtils::macFromRawBytes(pMac)};
          MORPH_DEVICE.deletePairedDevice(macToDeleteFromPaired);
          MORPH_DEVICE.savePairedDeviceListToFlash();
          BLE_DEVICE.notifyHandsetListUpdate();
          break;
        }


        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_REQUEST_NUM_PAIRED_HANDSETS): {

          if (length < (PDU_PACKET_LENGTH + 2)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t handsetsNumber {data[PDU_PACKET_LENGTH]};
          _numberOfHandsetsToReceive = handsetsNumber;
          _receivedHandsetsMac.clear();

          DBG_PRINT_TAG(TAG, "handset number received: %u", (size_t)handsetsNumber);
          BLE_DEVICE.notifyHandsetNumberReceived(handsetsNumber);
          break;
        }

        case static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_REQUEST_PAIRED_HANDSETS): {

          // payload is expected to be 4 bytes + N x 6 bytes
          if (length < (PDU_PACKET_LENGTH) or (length - PDU_PACKET_LENGTH) % MAC_LENGTH) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }

          const size_t devicesNumber {(length - PDU_PACKET_LENGTH) / MAC_LENGTH};
          DBG_PRINT_TAG(TAG, "received %u paired devices", devicesNumber);
          const uint8_t* const pMac {&data[PDU_PACKET_LENGTH]};

          for (size_t i{}; i < devicesNumber; i++) {
            const uint64_t mac {MacUtils::macFromRawBytes(pMac +  i * MAC_LENGTH)};
            _receivedHandsetsMac.insert(mac);
          }

          if (_numberOfHandsetsToReceive == _receivedHandsetsMac.size()) {
            DBG_PRINT_TAG(TAG, "all handsets received!");
            if (true == MORPH_DEVICE.deleteDevicesOutOfSet(_receivedHandsetsMac)) {
              DBG_PRINT_TAG(TAG, "handset list was updated!");
              MORPH_DEVICE.savePairedDeviceListToFlash();
              BLE_DEVICE.notifyHandsetListUpdate();
            }
            else {
              DBG_PRINT_TAG(TAG, "handset list stayed the same");
            }
          }
          break;
        }
        
        default:
          DBG_PRINT_TAG(TAG, "unknown command ID: %u", specID);
        break;
      }
    break;
      
    default:
      DBG_PRINT_TAG(TAG, "unknown feature id: %u", featureID);
      return;
    break;
  }
}



void BLE::handleMorphNotification(const uint8_t featureID, const uint8_t specID,
      const uint8_t* data, const size_t length) {
  
  switch (featureID) {
    case static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID):
      // DBG_PRINT_TAG(TAG, "handling MORPH notification with ID: %u", specID);
      switch (specID) {
        // volume changed
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_VOLUME_CHANGED): {
          if (length < (PDU_PACKET_LENGTH + 1)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          const uint8_t volume{data[PDU_PACKET_LENGTH]};
          DBG_PRINT_TAG(TAG, "new volume received: %u", volume);
          MORPH_DEVICE.set_volume(volume);
          BLE_DEVICE.notifyVolume(volume);
          break;
        }

        // handset connected
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_HANDSET_CONNECTED_RMT): {
          if (length < (PDU_PACKET_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          DBG_PRINT_TAG(TAG, "connected notification received!");
          MORPH_DEVICE.set_handsetConnected(true);
          BLE_DEVICE.notifyHandsetState(true);
          xEventGroupSetBits(_eventGroup, HANDSET_CONNECTED_REMOTELY);
          break;
        }

        // handset disconnected
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_HANDSET_DISCONNECTED_RMT): {
          if (length < (PDU_PACKET_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          DBG_PRINT_TAG(TAG, "disconnected notification received!");
          MORPH_DEVICE.set_handsetConnected(false);
          BLE_DEVICE.notifyHandsetState(false);
          xEventGroupSetBits(_eventGroup, HANDSET_DISCONNECTED_REMOTELY);
          break;
        }

        // connection attempt finished
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_CONNECTION_ATTEMPT_COMPLETED): {
          if (length < (PDU_PACKET_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          DBG_PRINT_TAG(TAG, "connection attempt finished received!");
          xEventGroupSetBits(_eventGroup, HANDSET_CONNECTION_ATTEMPT_COMPLETE);
          break;
        }

        // disconnection attempt finished
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_DISCONNECTION_ATTEMPT_COMPLETED): {
          if (length < (PDU_PACKET_LENGTH)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          DBG_PRINT_TAG(TAG, "disconnection attempt finished received!");
          xEventGroupSetBits(_eventGroup, HANDSET_DISCONNECTION_ATTEMPT_COMPLETE);
          break;
        }

        // MIC state changed
        case static_cast<uint8_t>(eMorphNotifications::MORPH_NOTIFICATION_MIC_STATE_CHANGED): {
          if (length < (PDU_PACKET_LENGTH + 1)) {
            DBG_PRINT_TAG(TAG, "not expected data length : %u", length);
            break;
          }
          DBG_PRINT_TAG(TAG, "mic state changed received");
          const bool micState{data[PDU_PACKET_LENGTH]};
          MORPH_DEVICE.set_micMuteStatus(micState);
          BLE_DEVICE.notifyMicState(micState);
          break;
        }
        
        default:
          DBG_PRINT_TAG(TAG, "unknown notification ID: %u", specID);
        break;
      }
    break;
      
    default:
      DBG_PRINT_TAG(TAG, "unknown feature id: %u", featureID);
      return;
    break;
  }
}

bool BLE::updateAll() {

  bool ret {true};

  if (!requestUpdateChargeLevel()) {
    DBG_PRINT_TAG(TAG, "failed to request battery value!");
    ret = false;
  }

  if (!requestUpdateHandsetState()){
    DBG_PRINT_TAG(TAG, "failed to request handset info");
    ret = false;
  }

  if (!requestUpdateMicState()) {
    DBG_PRINT_TAG(TAG, "failed to request mic state");
    ret = false;
  }

  if (!requestUpdateVolume()) {
    DBG_PRINT_TAG(TAG, "failed to request volume value");
    ret = false;
  }

  if (!requestUpdateAncState()) {
    DBG_PRINT_TAG(TAG, "failed to request ANC state");
    ret = false;
  }

  if (!requestUpdateAncMode()) {
    DBG_PRINT_TAG(TAG, "failed to request ANC mode");
    ret = false;
  }

  if (!requestPrimaryDevice()) {
    DBG_PRINT_TAG(TAG, "failed to request primary device");
    ret = false;
  }

  return ret;
}


void clearEvents() {
  // clear events
  xEventGroupClearBits(_eventGroup, ALL_EVENTS_MASK);
}

bool waitForHandsetDisconnection() {
  clearEvents();
  static const uint32_t eventsToWait {
    HANDSET_DISCONNECTION_ATTEMPT_COMPLETE |
    HANDSET_DISCONNECTED_REMOTELY
  };
  DBG_PRINT_TAG(TAG, "waiting for disconnection attempt results...");
  const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, eventsToWait, pdTRUE, pdTRUE, pdMS_TO_TICKS(BLE_DISCONNECTION_ACK_TIMEOUT))};
  
  DBG_PRINT_TAG(TAG, "events: %u, expected: %u", eventBits, eventsToWait);
  if ((eventsToWait & eventBits) == eventsToWait) {
    DBG_PRINT_TAG(TAG, "disconnected!");
    return true;
  }
  else {
    DBG_PRINT_TAG(TAG, "timeout!");
    return false;
  }
}

bool waitForHandsetConnection() {
  clearEvents();
  static const uint32_t eventsToWait {
    HANDSET_CONNECTION_ATTEMPT_COMPLETE |
    HANDSET_CONNECTED_REMOTELY
  };
  DBG_PRINT_TAG(TAG, "waiting for connection attempt results...");
  const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, eventsToWait, pdTRUE, pdTRUE, pdMS_TO_TICKS(BLE_CONNECTION_ACK_TIMEOUT))};
  DBG_PRINT_TAG(TAG, "events: %u, expected: %u", eventBits, eventsToWait);
  
  if ((eventsToWait & eventBits) == eventsToWait) {
    DBG_PRINT_TAG(TAG, "connected!");
    return true;
  }
  else {
    DBG_PRINT_TAG(TAG, "timeout!");
    return false;
  }
}

bool waitForHandsetInformation() {
  clearEvents();
  static const uint32_t eventsToWait {
    HANDSET_INFORMATION_RECEIVED
  };
  DBG_PRINT_TAG(TAG, "waiting for handset state and info");
  const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, eventsToWait, pdTRUE, pdTRUE, pdMS_TO_TICKS(BLE_HANDSET_GET_INFO_TIMEOUT))};
  DBG_PRINT_TAG(TAG, "events: %u, expected: %u", eventBits, eventsToWait);
  
  if ((eventsToWait & eventBits) == eventsToWait) {
    DBG_PRINT_TAG(TAG, "handset info received");
    return true;
  }
  else {
    DBG_PRINT_TAG(TAG, "timeout!");
    return false;
  }
}

void connectToHandsetTask(void* args) {
  
  PairedDevice* deviceToConnectTo = static_cast<PairedDevice*>(args);
  DBG_PRINT_TAG(TAG, "now trying to connect to device: %s", deviceToConnectTo->name);

  if (true == MORPH_DEVICE.get_handsetConnected() and deviceToConnectTo->macAddress == MORPH_DEVICE.get_handsetMac()) {
    DBG_PRINT_TAG(TAG, "already connected!");
    delete deviceToConnectTo;
    vTaskDelete(nullptr);
  }
  
  BLE_DEVICE.notifyHandsetConnectionStarted();

  if (MORPH_DEVICE.get_handsetConnected()) {

    if (false == BLE_DEVICE.requestDisconnectHandset()) {
        DBG_PRINT_TAG(TAG, "command disconnect handset failed");        
        BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_REQUEST_FAILED]);
        BLE_DEVICE.notifyHandsetConnectionFinished();
        delete deviceToConnectTo;
        vTaskDelete(nullptr);
    }

    const bool wasDisconnected {waitForHandsetDisconnection()};

    if (false == wasDisconnected) {
      BLE_DEVICE.notifyHandsetConnectionFinished();
      BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_TIMEOUT]);
      delete deviceToConnectTo;
      vTaskDelete(nullptr);
    }
  }

  if (false == BLE_DEVICE.requestConnectToHandset(deviceToConnectTo->macAddress, deviceToConnectTo->btProfileMask)) {
      DBG_PRINT_TAG(TAG, "command connect to handset failed");
      BLE_DEVICE.notifyHandsetConnectionFinished();
      BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_REQUEST_FAILED]);
      delete deviceToConnectTo;
      vTaskDelete(nullptr);
  }

  const bool wasConnected {waitForHandsetConnection()};

  if (false == wasConnected) {
    BLE_DEVICE.notifyHandsetConnectionFinished();
    BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_TIMEOUT]);
    delete deviceToConnectTo;
    vTaskDelete(nullptr);
  }

  const bool wasHandsetInfoReceived {waitForHandsetInformation()};

  if (false == wasHandsetInfoReceived) {
    BLE_DEVICE.notifyHandsetConnectionFinished();
    BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_TIMEOUT]);
    delete deviceToConnectTo;
    vTaskDelete(nullptr);
  }

  BLE_DEVICE.notifyHandsetConnectionFinished();

  if (true == MORPH_DEVICE.get_handsetConnected() and MORPH_DEVICE.get_handsetMac() == deviceToConnectTo->macAddress) {
    BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_SUCCESS]);
  }
  else {
    BLE_DEVICE.notifyToast(LANGUAGE_PACK[eTitleType::TITLE_FAILED]);
  }

  const uint16_t deviceIndexInList {GRAPHICS.getPairedDeviceIndexInList(*deviceToConnectTo)};
  DBG_PRINT_TAG(TAG, "deviceIndexInList: %u", deviceIndexInList);
  
  if (DEVICE_NOT_IN_LIST != deviceIndexInList and
    true == SETTINGS.isBeepFeatureEnabled()) {
    BLE_DEVICE.requestBeep(deviceIndexInList + 1);
  }

  delete deviceToConnectTo;
  vTaskDelete(nullptr);
}

void BLE::notifyToast(const char* pString) {
  message_t msg {
    eMessageCode::MESSAGE_TOAST_ADDED,
    .data = {
      .pStringData = pString
    }
  };
  produceMessage(msg);
}

void BLE::notifyAncState(const bool state) {
  message_t msg {
    eMessageCode::MESSAGE_ANC_STATE_CHANGED,
    .data = {
      .booleanParam = state
    }
  };
  produceMessage(msg);
}

void BLE::notifyConnectionState(const bool state) {
  message_t msg {
    eMessageCode::MESSAGE_BLE_CONNECTION_STATE_CHANGED,
    .data = {
      .booleanParam = state
    }
  };
  produceMessage(msg);
}

void BLE::notifyAncMode(const eAncMode mode) {
  message_t msg {
    eMessageCode::MESSAGE_ANC_MODE_CHANGED,
    .data = {
      .ui8Param = mode
    }
  };
  produceMessage(msg);
}

void BLE::notifyPrimaryEarbud(const ePrimaryEarbud value) {
  message_t msg {
    eMessageCode::MESSAGE_PRIMARY_DEVICE_CHANGED,
    .data = {
      .ui8Param = (uint8_t)value
    }
  };
  produceMessage(msg);
}

void BLE::notifyVolume(const uint8_t value) {
  message_t msg {
    eMessageCode::MESSAGE_VOLUME_CHANGED,
    .data = {
      .ui8Param = value
    }
  };
  produceMessage(msg);
}

void BLE::notifyEarbudsCharge() {
  message_t msg {
    eMessageCode::MESSAGE_EARBUDS_CHARGE_LEVEL_CHANGED,
    .data = {}
  };
  produceMessage(msg);
}

void BLE::notifyHandsetInfo() {
  message_t msg {
    eMessageCode::MESSAGE_HANDSET_INFO_CHANGED,
    .data = {}
  };
  produceMessage(msg);
  xEventGroupSetBits(_eventGroup, HANDSET_INFORMATION_RECEIVED);
}

void BLE::notifyHandsetListUpdate() {
  message_t msg {
    eMessageCode::MESSAGE_HANDSET_LIST_CHANGED,
    .data = {}
  };
  produceMessage(msg);
}

void BLE::notifyHandsetConnectionStarted() {
  message_t msg {
    eMessageCode::MESSAGE_HANDSET_CONNECTION_STARTED,
    .data = {}
  };
  produceMessage(msg);
}

void BLE::notifyHandsetConnectionFinished() {
  message_t msg {
    eMessageCode::MESSAGE_HANDSET_CONNECTION_FINISHED,
    .data = {}
  };
  produceMessage(msg);
}

void BLE::notifyHandsetNumberReceived(const uint8_t number) {

  message_t msg {
    eMessageCode::MESSAGE_HANDSET_NUMBER_RECEIVED,
    .data = {
      .ui8Param = number
    }
  };
  produceMessage(msg);
}

void BLE::notifyMicState(const bool state) {
  message_t msg {
    eMessageCode::MESSAGE_MIC_STATE_CHANGED,
    .data = {
      .booleanParam = state
    }
  };
  produceMessage(msg);
}

void BLE::notifyHandsetState(const bool state) {
  message_t msg {
    eMessageCode::MESSAGE_HANDSET_STATE_CHANGED,
    .data = {
      .booleanParam = state
    }
  };
  produceMessage(msg);
  xEventGroupSetBits(_eventGroup, HANDSET_STATE_RECEIVED);
}

void BLE::startHandsetConnectionTask(const PairedDevice& device) {

  PairedDevice* deviceParam = new PairedDevice{device};
  memcpy(deviceParam->name, device.name, MAX_DEVICE_NAME_LENGTH);
  assert (pdTRUE == xTaskCreate(connectToHandsetTask, "connectToHandsetTask", configMINIMAL_STACK_SIZE*4, deviceParam, 1, nullptr));
}

bool BLE::requestDisconnectHandset() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_DISCONNECT_PREV),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestConnectToHandset(const uint64_t mac, const uint32_t mask) {

  uint8_t data[MAC_LENGTH + BT_PROFILE_MASK_LENTH]{};

  MacUtils::macToByteArray(mac, data);
  MacUtils::profileMaskToByteArray(mask, data + MAC_LENGTH);

  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_CONNECT_PREV_HANDSET),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID), data, MAC_LENGTH + BT_PROFILE_MASK_LENTH);
}

bool BLE::requestPrimaryDevice() {
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_Application::QCC_V3_COMMAND_WHICH_PRIMARY),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_APPLICATION));
}

bool BLE::requestDeletePairedHandset(const uint64_t mac) {

  uint8_t data[MAC_LENGTH]{};
  MacUtils::macToByteArray(mac, data);

  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_DELETE_PAIRED_HANDSET),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID), data, MAC_LENGTH);
}


bool BLE::requestPairedDevicesNumber() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_REQUEST_NUM_PAIRED_HANDSETS),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestPairedDevicesList(const size_t indexFrom, const size_t amount) {

  const uint8_t payload[2] {
    static_cast<uint8_t>(indexFrom),
    static_cast<uint8_t>(amount)
  };
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_REQUEST_PAIRED_HANDSETS),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID), payload, 2);
}

bool BLE::requestUpdateVolume() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_VOLUME),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestUpdateAncState() {
  static const uint8_t ancFeature{0x01};
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_GET_STATE),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION), &ancFeature, 1);
}

bool BLE::requestUpdateMicState() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_MIC_MUTE_STATE),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestUpdateHandsetState() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_IS_HANDSET_CONNECTED),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestBeep(const uint8_t numberOfBeeps) {
  const uint8_t payload[] {
      numberOfBeeps, 
  };
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_PLAY_TONE),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID), payload, 1);
}

bool BLE::requestUpdateHandsetInfo() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_HANDSET_CONNECTED),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestUpdateAncMode() {
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_GET_CURR_MODE),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION));
}

bool BLE::requestUpdateChargeLevel() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_GET_BATTERY_LEVEL),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestVolumeChange(const uint8_t value) {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_CHANGE_VOLUME),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID), &value, 1);
}

bool BLE::requestAncStateChange(const bool value) {
  const uint8_t payload[2] {
      0x01, // means ANC is chosen
      value // on/off
  };
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_SET_STATE),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION), payload, 2);
}

bool BLE::requestPairingMode() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_PAIR_HANDSET),
    static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}


bool BLE::requestMicToggle() {
  return sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_TOGGLE_MIC_MUTE_STATE), static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID));
}

bool BLE::requestNotificationsRegister(const uint8_t feature) {
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_Framework::QCC_V3_COMMAND_REGISTER_NOTIFICATIONS),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_FRAMEWORK), &feature, 1);
}

bool BLE::requestAncModeChange(const eAncMode value) {
  const uint8_t writeValue{static_cast<uint8_t>(value)};
  return sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_SET_MODE),
    static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION), &writeValue, 1);
}

bool BLE::sendCommand(const eVendorType vendor, const uint8_t cmd, const uint8_t feature, const uint8_t* data, const size_t dataLen) {

  PduPacket packet {
    {eVendorType::VENDOR_V3QC == vendor? v3QCVendorID : morphVendorID},
    feature,
    ePduTypes::PDU_TYPE_COMMAND,
    cmd
  };

  return sendPacket(_commandChar, packet, data, dataLen);
}

bool BLE::sendPacket(ESP32Kolban::BLERemoteCharacteristic* characteristic, const PduPacket& packet, const uint8_t* data, const size_t dataLen) {

  if (!isConnected()) {
    return false;
  }
  if (!characteristic) {
    return false;
  }
  const size_t totalBytes{PDU_PACKET_LENGTH + dataLen};
  uint8_t* bytes {new uint8_t [totalBytes]};
  memset(bytes, 0, totalBytes);
  memcpy(bytes + PDU_PACKET_LENGTH, data, dataLen);

  packet.toBytesArray(bytes);
  
  const bool writeRes {sendRawData(bytes, totalBytes)};

  delete[] bytes;

  return writeRes;
}

bool BLE::sendRawData(uint8_t* data, const size_t dataLen) {

  if (!_commandChar) {
    return false;
  }
  _commandChar->writeValue(data, dataLen, true);
  const bool writeRes {true};
  return writeRes;
}

void BLE::disconnectFromServer() {
  if (true == _isProperlyConnected) {
    _isProperlyConnected = false;
    notifyConnectionState(false);
  }
  if (_client and _client->isConnected()) {
    DBG_PRINT_TAG(TAG, "Disconnecting ..");
    _client->disconnect();
  }
}

bool BLE::isConnected() const {
  return _isProperlyConnected;
}

void AddvertiseCbHandler::onResult(ESP32Kolban::BLEAdvertisedDevice advertisedDevice) {

  DBG_PRINTF("ble server found: %s, addr type: %u\r\n", advertisedDevice.toString().c_str(), advertisedDevice.getAddressType());

  if (MORPH_DEVICE_NAME == advertisedDevice.getName()) {
      DBG_PRINT_TAG(TAG, "Morph device is found!");
      BLE_DEVICE.stopScan();
      BLE_DEVICE.setServerDevice(advertisedDevice);
      BLE_DEVICE.setServerMacType(advertisedDevice.getAddressType());
  }
}

uint32_t SecurityCbHandler::onPassKeyRequest(){
  DBG_PRINTLN("onPassKeyRequest");
  return 123456;
}

void SecurityCbHandler::onPassKeyNotify(uint32_t pass_key){
  DBG_PRINTF("SecurityCbHandler: The passkey Notify number: %u\r\n", pass_key);
}

bool SecurityCbHandler::onConfirmPIN(uint32_t pass_key){
  DBG_PRINTF("SecurityCbHandler: The passkey YES/NO number:%u\r\n", pass_key);
  return true;
}

bool SecurityCbHandler::onSecurityRequest(){
  DBG_PRINTLN("SecurityCbHandler: Security Request");
  return true;
}

void SecurityCbHandler::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl){

  DBG_PRINT_TAG("SEC", "pair status = %s\r\n", auth_cmpl.success ? "success" : "fail");
  if (!auth_cmpl.success) {
    BLE_DEVICE.disconnectFromServer();
    return;
  }
  xSemaphoreGive(pairingSmphr);
}

void ClientCbHandler::onConnect(ESP32Kolban::BLEClient* pclient) {
  DBG_PRINTLN("onConnect");
}

void ClientCbHandler::onDisconnect(ESP32Kolban::BLEClient* pclient) {
  DBG_PRINTLN("onDisconnect");
  BLE_DEVICE.disconnectFromServer();
}
