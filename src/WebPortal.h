#pragma once

#include <DNSServer.h>
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

    void configureRoutes();
    void sendStatus();
};
