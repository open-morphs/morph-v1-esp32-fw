#pragma once

#include <SingletonTemplate.hpp>
#include "stdint.h"
#include "esp_http_server.h"
#include "freertos/event_groups.h"
#include "HTTPServer.hpp"
#include "ITask.hpp"
#include "ArduinoJson-v6.18.4.h"
#include "MessageProducer.hpp"

using namespace httpsserver;

struct HttpWebserver : Singleton<HttpWebserver>, public ITask, MessageProducer {
    friend class Singleton;
    bool isActive()const;
    bool isKeepAliveTimeout()const;
    void updateKeepAlive();

    bool isOtaRunning() const {
        return _isOtaRunning;
    }

    private:
        void run(void* args) override;
        void onStop() override;
        void stopDelayed();
        HttpWebserver();
        void prepareWifiModule();
        static void rootHandler(HTTPRequest * req, HTTPResponse * res);
        static void closeWebserverHandler(HTTPRequest * req, HTTPResponse * res);
        static void saveHandler(HTTPRequest * req, HTTPResponse * res);
        static void settingsHandler(HTTPRequest * req, HTTPResponse * res);
        static void jsHandler(HTTPRequest * req, HTTPResponse * res);
        static void cssHandler(HTTPRequest * req, HTTPResponse * res);
        static void notFoundHandler(HTTPRequest * req, HTTPResponse * res);
        static void otaHandler(HTTPRequest * req, HTTPResponse * res);
        static void updateHandler(HTTPRequest * req, HTTPResponse * res);
        static void statusHandler(HTTPRequest * req, HTTPResponse * res);
        std::string getApName()const;
        void notifyStateChanged(const bool isRunning);
        void notifyOtaFailed();
        void notifyOtaStarted();

    private:
        HTTPServer* _insecureServer = nullptr;
        ResourceNode* _notFoundNode = nullptr;
        ResourceNode* _rootNode = nullptr;
        ResourceNode* _settingsGetNode = nullptr;
        ResourceNode* _settingsPostNode = nullptr;
        ResourceNode* _otaGetNode = nullptr;
        ResourceNode* _otaPostNode = nullptr;
        ResourceNode* _closeNode = nullptr;
        ResourceNode* _cssHandler = nullptr;
        ResourceNode* _jsHandler = nullptr;
        ResourceNode* _updateHandler = nullptr;
        ResourceNode* _statusHandler = nullptr;
        bool _isActive = false;
        uint32_t _lastKeepAliveTs = 0;
        bool _isOtaRunning = false;
};

#define WEBSERVER HttpWebserver::instance()