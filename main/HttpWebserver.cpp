#include <HttpWebserver.hpp>
#include <esp_wifi.h>
#include <Utils/StringUtils.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include <Utils/TimeUtils.hpp>
#include <ESP32.hpp>
#include <Settings.hpp>
#include <Utils/HttpUtils.hpp>
#include <HTTPServer.hpp>
#include <esp_ota_ops.h>
#include <sys/param.h>
#include "Events.hpp"
#include <WiFi.h>
#include <SmartCase.hpp>
#include <AsyncFunctor.hpp>
#include <ScreenlockImageType.hpp>

using namespace Languages;

enum class eOtaStatus {
    OTA_NONE = 0,
    OTA_SUCCESS = 1,
    OTA_INVALID_FILE = 2,
    OTA_TIMEOUT = 3,
    OTA_FAILED_BEGIN = 4,
    OTA_FAILED_FINISH = 5,
    OTA_FAILED_CHECK_PARTITION = 6,
};
static eOtaStatus _flashStatus {eOtaStatus::OTA_NONE};

static const char* const TAG = "WEB_SERVER";
static const char* const languageKey {"lang"};
static const char* const imageKey {"image"};
static const char* const ssidKey {"ssid"};
static const char* const passwordKey {"pass"};
static const char* const beepFeatureKey {"beep"};

const char POST_METHOD[] = "POST";
const char GET_METHOD[] = "GET";

#define DEVICE_DEFAULT_WIFI_IP_ADDRESS  IPAddress(192, 168, 0, 105)
#define DEVICE_DEFAULT_WIFI_GATEWAY     IPAddress(192, 168, 1, 144)
#define DEVICE_DEFAULT_WIFI_SUBNET_MASK IPAddress(255, 255, 255, 0)

#define MAX_RX_BUFFER_LENGTH        2048UL
#define SCAN_TIMEOUT_MS             10000 UL
#define KEEP_ALIVE_PERIOD_S         (60 * 5)
#define WIFI_SSID_MAX_LENGTH        32
#define WIFI_SSKEY_MAX_LENGTH       63
#define HTTP_SERVER_MAX_CONNECTIONS 2
#define HTTP_SERVER_DEFAULT_PORT    80
#define FILE_TIMEOUT_MS             10000UL

// root page
extern const uint8_t ROOT_HTML_START[] asm("_binary_root_html_start");
extern const uint16_t root_html_length;
// styleheets
extern const uint8_t STYLESHEET_CSS_START[] asm("_binary_stylesheet_css_start");
extern const uint16_t stylesheet_css_length;
// settings page
extern const uint8_t SETTINGS_HTML_START[] asm("_binary_settings_html_start");
extern const uint16_t settings_html_length;
// save page
extern const uint8_t SAVE_HTML_START[] asm("_binary_save_html_start");
extern const uint16_t save_html_length;
// OTA page
extern const uint8_t OTA_HTML_START[] asm("_binary_ota_html_start");
extern const uint16_t ota_html_length;
// js
extern const uint8_t SCRIPT_JS_START[] asm("_binary_script_js_start");
extern const uint16_t script_js_length;

HttpWebserver::HttpWebserver()
    :   ITask("Webserver", configMINIMAL_STACK_SIZE * 6, 10), 
        _insecureServer{new HTTPServer(HTTP_SERVER_DEFAULT_PORT, HTTP_SERVER_MAX_CONNECTIONS)},
        _notFoundNode{new ResourceNode("", "GET", &notFoundHandler)},
        _rootNode{new ResourceNode("/", "GET", &rootHandler)},
        _settingsGetNode {new ResourceNode("/settings", "GET", &settingsHandler)},
        _settingsPostNode {new ResourceNode("/settings", "POST", &settingsHandler)},
        _otaGetNode {new ResourceNode("/ota", "GET", &otaHandler)},
        _otaPostNode {new ResourceNode("/ota", "POST", &otaHandler)},
        _closeNode {new ResourceNode("/closewifi", "GET", &closeWebserverHandler)},
        _cssHandler{new ResourceNode("/stylesheet.css", "GET", &cssHandler)},
        _jsHandler{new ResourceNode("/script.js", "GET", &jsHandler)},
        _updateHandler{new ResourceNode("/update", "POST", &updateHandler)},
        _statusHandler{new ResourceNode("/status", "GET", &statusHandler)} {
    
    setPeriod(1);
    _insecureServer->setDefaultNode(_notFoundNode);
    _insecureServer->registerNode(_rootNode);
    _insecureServer->registerNode(_settingsGetNode);
    _insecureServer->registerNode(_settingsPostNode);
    _insecureServer->registerNode(_otaGetNode);
    _insecureServer->registerNode(_otaPostNode);
    _insecureServer->registerNode(_closeNode);
    _insecureServer->registerNode(_cssHandler);
    _insecureServer->registerNode(_jsHandler);
    _insecureServer->registerNode(_updateHandler);
    _insecureServer->registerNode(_statusHandler);
}


bool HttpWebserver::isActive()const {
    return _isActive;
}

void HttpWebserver::notifyStateChanged(const bool isRunning) {

    message_t msg {
        .messageCode = MESSAGE_WEBSERVER_STATE_CHANGED,
        .data = {
            .booleanParam = isRunning
        }
    };
    produceMessage(msg);
}

void HttpWebserver::notifyOtaFailed() {
    message_t msg {
        .messageCode = MESSAGE_OTA_FAILED,
        .data = {}
    };
    produceMessage(msg);
}

void HttpWebserver::notifyOtaStarted() {
    message_t msg {
        .messageCode = MESSAGE_OTA_STARTED,
        .data = {}
    };
    produceMessage(msg);
}

void HttpWebserver::prepareWifiModule() {
    if (!WiFi.config(DEVICE_DEFAULT_WIFI_IP_ADDRESS, DEVICE_DEFAULT_WIFI_GATEWAY,
        DEVICE_DEFAULT_WIFI_SUBNET_MASK)) {

      DBG_PRINT_TAG(TAG, "WI-FI Initialization failed.");
    }
    const bool wifiRes {WiFi.softAP(getApName().c_str()  , "12345678")};
    DBG_PRINT_TAG(TAG, "%s\r\n", wifiRes ? "WIFI OK" : "WIFI FAILED");
}

string HttpWebserver::getApName()const {
    return stringUtils::format("MORPH_CASE_%u", ESP32Utils::chipID());
}

void HttpWebserver::run(void* args) {
    DBG_PRINT_TAG(TAG, "started!");
    prepareWifiModule();
    updateKeepAlive();
    _insecureServer->start();
    _isActive = true;
    notifyStateChanged(true);

    while(1) {
        _insecureServer->loop();
        if (true == isKeepAliveTimeout()) {
            DBG_PRINT_TAG(TAG, "keepalive timeout!");
            stop();
        }
        delay(_periodMs);
    }
}

void HttpWebserver::onStop() {
    _insecureServer->stop();
    WiFi.softAPdisconnect(true);
    _isActive = false;
    notifyStateChanged(false);
}

void HttpWebserver::stopDelayed() {
    auto delayedStopTask = [](void* arg) {
        HttpWebserver* pHttpWebserver {static_cast<HttpWebserver*>(arg)};
        vTaskDelay(pdMS_TO_TICKS(50));
        pHttpWebserver->stop();
        vTaskDelete(NULL);
    };
    assert (pdTRUE == xTaskCreate(delayedStopTask, "delayedStopTask", configMINIMAL_STACK_SIZE*3, this, 1, NULL));
}

void HttpWebserver::closeWebserverHandler(HTTPRequest * req, HTTPResponse * res) {

    res->setHeader("Content-Type", "text/html");
    std::string reply {"Webserver is stopped..."};
    res->write(reinterpret_cast<const uint8_t*>(reply.c_str()), reply.length());
    WEBSERVER.stopDelayed();
}

void HttpWebserver::notFoundHandler(HTTPRequest * req, HTTPResponse * res) {
    WEBSERVER.updateKeepAlive();
    res->setStatusCode(HTTPD_404_NOT_FOUND);
    res->setStatusText("Not Found");
    res->setHeader("Content-Type", "text/html");
    res->printf("%s %s resource not found", req->getMethod().c_str(), req->getRequestString().c_str());
}

void HttpWebserver::rootHandler(HTTPRequest * req, HTTPResponse * res) {
    WEBSERVER.updateKeepAlive();
    DBG_PRINT_TAG(TAG, "rootHandler...");
    std::string reply {reinterpret_cast<const char*>(ROOT_HTML_START)};
    assert (1 == stringUtils::replace(reply, "sn_placeholder", stringUtils::format("%u", ESP32Utils::chipID())));
    assert (1 == stringUtils::replace(reply, "hw_placeholder", SETTINGS.hw()));
    assert (1 == stringUtils::replace(reply, "fw_placeholder", SETTINGS.fwVersion()));
    // assert (1 == stringUtils::replace(reply, "ssid_placeholder", SETTINGS.ssid()));
    assert (1 == stringUtils::replace(reply, "battery_placeholder", 
        stringUtils::format("%u %%", static_cast<size_t>(SMARTCASE.get_batteryPercent()))));


    res->setHeader("Content-Type", "text/html");
    res->write(reinterpret_cast<const uint8_t*>(reply.c_str()), reply.length());
}

void HttpWebserver::cssHandler(HTTPRequest * req, HTTPResponse * res) {
    WEBSERVER.updateKeepAlive();
    DBG_PRINT_TAG(TAG, "cssHandler...");
    res->setHeader("Content-Type", "text/css");
    res->write(STYLESHEET_CSS_START, stylesheet_css_length);
}

void HttpWebserver::saveHandler(HTTPRequest * req, HTTPResponse * res) {
    WEBSERVER.updateKeepAlive();
    DBG_PRINT_TAG(TAG, "saveHandler...");
    res->setHeader("Content-Type", "text/html");
    res->write(SAVE_HTML_START, save_html_length);
}

void HttpWebserver::otaHandler(HTTPRequest * req, HTTPResponse * res) {
    DBG_PRINT_TAG(TAG, "otaHandler...");
    _flashStatus = eOtaStatus::OTA_NONE;
    res->setHeader("Content-Type", "text/html");
    res->write(OTA_HTML_START, ota_html_length);
}

void HttpWebserver::settingsHandler(HTTPRequest * req, HTTPResponse * res) {
    WEBSERVER.updateKeepAlive();
    DBG_PRINT_TAG(TAG, "settingsHandler...");
    res->setHeader("Content-Type", "text/html");

    if (POST_METHOD == req->getMethod()) {
        std::string content {};
        content.resize(req->getContentLength() + 1);
        req->readChars(const_cast<char*>(content.data()), req->getContentLength());
        DBG_PRINT_TAG(TAG, "cont: %s\r\n", content.c_str());

        std::string ssidReceived{}, passReceived{}, languageReceived{}, beepReceived{}, imageReceived{};

        if (false == HttpUtils::getValueFromForm(content, languageKey, languageReceived)) {
            DBG_PRINT_TAG(TAG, "no language received!");
            res->setStatusCode(HTTPD_400_BAD_REQUEST);
            res->setStatusText("ERROR");
            res->setHeader("Content-Type", "text/html");
            res->println("Language setting not found");
            return;
        }

        if (false == HttpUtils::getValueFromForm(content, imageKey, imageReceived)) {
            DBG_PRINT_TAG(TAG, "no image received!");
            res->setStatusCode(HTTPD_400_BAD_REQUEST);
            res->setStatusText("ERROR");
            res->setHeader("Content-Type", "text/html");
            res->println("Image setting not found");
            return;
        }

        HttpUtils::getValueFromForm(content, beepFeatureKey, beepReceived);

        // if (false == HttpUtils::getValueFromForm(content, ssidKey, ssidReceived)) {
        //     DBG_PRINT_TAG(TAG, "no ssid received!");
        //     res->setStatusCode(HTTPD_400_BAD_REQUEST);
        //     res->setStatusText("ERROR");
        //     res->setHeader("Content-Type", "text/html");
        //     res->println("Ssid not found");
        //     return;
        // }

        // if (false == HttpUtils::getValueFromForm(content, passwordKey, passReceived)) {
        //     DBG_PRINT_TAG(TAG, "no password received!");
        //     res->setStatusCode(HTTPD_400_BAD_REQUEST);
        //     res->setStatusText("ERROR");
        //     res->setHeader("Content-Type", "text/html");
        //     res->println("Password not found");
        //     return;
        // }
        uint32_t languageCode{};
        if (false == stringUtils::toUint32(languageReceived, languageCode) or 
            eLanguage::LANGUAGE_MAX <= languageCode) {
            DBG_PRINT_TAG(TAG, "Invalid language code received: %s", languageReceived);
            res->setStatusCode(HTTPD_400_BAD_REQUEST);
            res->setStatusText("ERROR");
            res->setHeader("Content-Type", "text/html");
            res->println("Language code error");
            return;
        }

        uint32_t imageCode{};
        if (false == stringUtils::toUint32(imageReceived, imageCode) or 
            eScreenlockImage::IMAGE_MAX <= imageCode) {
            DBG_PRINT_TAG(TAG, "Invalid image code received: %s", imageReceived);
            res->setStatusCode(HTTPD_400_BAD_REQUEST);
            res->setStatusText("ERROR");
            res->setHeader("Content-Type", "text/html");
            res->println("Image code error");
            return;
        }

        // SETTINGS.setSsid(ssidReceived);
        // SETTINGS.setPass(passReceived);
        SETTINGS.setBeepFeatureEnabled("on" == beepReceived);
        SETTINGS.setLanguage(static_cast<uint8_t>(languageCode));
        SETTINGS.setLockScreenImage(static_cast<uint8_t>(imageCode));
        SETTINGS.save();

        LANGUAGES.setLanguage(static_cast<eLanguage>(languageCode));
        GRAPHICS.updateLanguage();
        GRAPHICS.updateLockscreenImage();

        res->write(SAVE_HTML_START, save_html_length);
    }
    else {
        res->write(SETTINGS_HTML_START, settings_html_length);
    } 
}

void HttpWebserver::jsHandler(HTTPRequest * req, HTTPResponse * res) {

    DBG_PRINT_TAG(TAG, "jsHandler");
    WEBSERVER.updateKeepAlive();
    DBG_PRINT_TAG(TAG, "cssHandler...");
    res->setHeader("Content-Type", "text/javascript");
    res->write(SCRIPT_JS_START, script_js_length);
}

void HttpWebserver::updateHandler(HTTPRequest * req, HTTPResponse * res) {

    DBG_PRINT_TAG(TAG, "updateHandler");
    WEBSERVER.updateKeepAlive();

    esp_ota_handle_t otaHandle;
    const size_t contentLength = req->getContentLength();
    DBG_PRINT_TAG(TAG, "contentLength: %u", contentLength);

    uint8_t rxBuff [MAX_RX_BUFFER_LENGTH] {};
    size_t totalContentReceived {};
    size_t receivedLength {};
    const esp_partition_t* updatePartition = esp_ota_get_next_update_partition(NULL);
    assert(updatePartition != NULL);

    _flashStatus = eOtaStatus::OTA_NONE;
	uint32_t lastTimeReceived {};
    bool isReqBodyStarted{};
    lastTimeReceived = TimeUtils::nowMs();

	while (!req->requestComplete() && (false == TimeUtils::isPeriodPassed(lastTimeReceived, FILE_TIMEOUT_MS))) {
		receivedLength = req->readBytes(rxBuff, MIN(contentLength, MAX_RX_BUFFER_LENGTH));
        if (0 == receivedLength) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        lastTimeReceived = TimeUtils::nowMs();
        totalContentReceived += receivedLength;
        DBG_PRINT_TAG(TAG, "totalContentReceived: %.2f%", 100.0F * totalContentReceived / contentLength);

        uint8_t* pFileData{rxBuff};
        size_t fwPartLength {receivedLength};

		if (!isReqBodyStarted) {
		    isReqBodyStarted = true;
            pFileData = reinterpret_cast<uint8_t*>(strstr(reinterpret_cast<char*>(rxBuff), "\r\n\r\n"));
            if (nullptr == pFileData) {
                DBG_PRINT_TAG(TAG, "unexpected body format!");
                WEBSERVER._isOtaRunning = false;
                WEBSERVER.notifyOtaFailed();
                return;
            }

            pFileData += 4;
            fwPartLength = receivedLength - (pFileData - rxBuff);
			DBG_PRINT_TAG(TAG, "OTA file size: %u", contentLength);
			if (ESP_OK != esp_ota_begin(updatePartition, OTA_SIZE_UNKNOWN, &otaHandle) ) {
				DBG_PRINT_TAG(TAG, "Error With OTA Begin, Cancelling OTA!!!");
                _flashStatus = eOtaStatus::OTA_FAILED_BEGIN;
                WEBSERVER._isOtaRunning = false;
                WEBSERVER.notifyOtaFailed();
                return;
			}
		}

        if(ESP_OK != esp_ota_write(otaHandle, pFileData, fwPartLength)) {
            DBG_PRINT_TAG(TAG, "esp_ota_write failed!");
            _flashStatus = eOtaStatus::OTA_INVALID_FILE;
            WEBSERVER._isOtaRunning = false;
            WEBSERVER.notifyOtaFailed();
            return;
        }
        if (!WEBSERVER._isOtaRunning) {
            WEBSERVER._isOtaRunning = true;
            WEBSERVER.notifyOtaStarted();
        }
	}

    if (false == req->requestComplete()) {
        DBG_PRINT_TAG(TAG, "timeout...");
        _flashStatus = eOtaStatus::OTA_TIMEOUT;
        WEBSERVER._isOtaRunning = false;
        WEBSERVER.notifyOtaFailed();
        return;
    }

	DBG_PRINT_TAG(TAG, "finishing OTA..");

    if (ESP_OK == esp_ota_end(otaHandle)) {
		// Lets update the partition
		if(ESP_OK == esp_ota_set_boot_partition(updatePartition))  {
			const esp_partition_t* bootPartition = esp_ota_get_boot_partition();
			_flashStatus = eOtaStatus::OTA_SUCCESS;
            DBG_PRINT_TAG(TAG, "Next boot partition subtype %d at offset 0x%x", bootPartition->subtype, bootPartition->address);
            auto rebootTask = []() {
                delay(3000);
                SMARTCASE.disableDisplayManager();
                SMARTCASE.safeSwitchOff(LANGUAGE_PACK[eTitleType::TITLE_OTA_COMPLETED]);
                WEBSERVER._isOtaRunning = false;
            };
            static AsyncFunctor delayedRebooter(rebootTask, "ota_reboot", configMINIMAL_STACK_SIZE*3, 10);
            delayedRebooter.start();
            return;
        }
        else {
			DBG_PRINT_TAG(TAG, "Failed to check flashed partitions!");
            _flashStatus = eOtaStatus::OTA_FAILED_CHECK_PARTITION;
            WEBSERVER._isOtaRunning = false;
            WEBSERVER.notifyOtaFailed();
            return;
		}
	}
    else {
		DBG_PRINT_TAG(TAG, "!!! OTA End Error !!!");
        _flashStatus = eOtaStatus::OTA_FAILED_FINISH;
        WEBSERVER._isOtaRunning = false;
        WEBSERVER.notifyOtaFailed();
        return;
	}
}

void HttpWebserver::statusHandler(HTTPRequest * req, HTTPResponse * res) {

    DBG_PRINT_TAG(TAG, "statusHandler");
    DynamicJsonDocument jsonDoc(1024);
    JsonObject rootObject = jsonDoc.to<JsonObject>();

    //set result properties
    rootObject["status"] = static_cast<int>(_flashStatus);
    rootObject["compile_time"] = __TIME__;
    rootObject["compile_date"] = __DATE__;
    rootObject["battery"] = SMARTCASE.get_batteryPercent();
    rootObject[languageKey] = SETTINGS.language();
    rootObject[imageKey] = SETTINGS.lockScreenImage();
    rootObject[ssidKey] = SETTINGS.ssid();
    rootObject[passwordKey] = SETTINGS.pass();
    rootObject[beepFeatureKey] = SETTINGS.isBeepFeatureEnabled();

    std::string reply{""};
    serializeJson(jsonDoc, reply);
    res->setHeader("Content-Type", "application/json");
    res->write(reinterpret_cast<const uint8_t*>(reply.c_str()), reply.length());
}

void HttpWebserver::updateKeepAlive() {
    _lastKeepAliveTs = TimeUtils::nowMs();
}

bool HttpWebserver::isKeepAliveTimeout()const {
    return TimeUtils::isPeriodPassed(_lastKeepAliveTs, KEEP_ALIVE_PERIOD_S*1000);
}
