#pragma once

#include <DNSServer.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

class WebPortal {
  public:
    void begin();
    void loop();
    const String &ssid() const;
    const String &password() const;
    uint8_t connectedClients() const;

  private:
    WebServer server_{80};
    DNSServer dns_;
    String ssid_;
    String password_;
    fs::LittleFSFS saveFs_;
    File saveUpload_;
    bool saveStorageReady_ = false;
    bool saveUploadOk_ = false;
    size_t saveUploadBytes_ = 0;
    String saveUploadError_;

    void configureRoutes();
    void sendStatus();
    void sendSaveStorageStatus();
    void sendSaveBackup();
    void handleSaveUpload();
    void finishSaveUpload();
};
