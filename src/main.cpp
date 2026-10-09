#include <Arduino.h>
#include <M5Unified.h>

#include "EmbeddedAssets.generated.h"
#include "WebPortal.h"

namespace {
constexpr char kVersion[] = "v0.2.9-prototype";
constexpr uint8_t kFacePin = 11;
constexpr uint8_t kM5Pin = 12;
constexpr uint8_t kNormalBrightness = 80;
constexpr uint8_t kDimBrightness = 12;
constexpr uint32_t kDimAfterMs = 30000;
constexpr uint32_t kMinimumSplashMs = 2500;

WebPortal portal;
bool showingQr = false;
bool dimmed = false;
uint32_t lastInteractionAt = 0;
uint32_t lastClientCount = UINT32_MAX;

bool pressed(uint8_t pin) {
    static uint32_t lastPress[49] = {};
    if (digitalRead(pin) != LOW || millis() - lastPress[pin] < 280) return false;
    lastPress[pin] = millis();
    return true;
}

void drawSplash() {
    auto &d = M5.Display;
    d.fillScreen(TFT_BLACK);
    d.drawJpg(kSplashJpg, kSplashJpgSize, 0, 0);
}

void drawStatus() {
    auto &d = M5.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextColor(TFT_GREEN, TFT_BLACK);
    d.setTextSize(2);
    d.setCursor(6, 4);
    d.println("PocketBoy S3");

    d.setTextSize(1);
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(6, 29);
    d.println("JOIN WI-FI");
    d.setTextColor(TFT_YELLOW, TFT_BLACK);
    d.setTextSize(2);
    d.setCursor(6, 39);
    d.println(portal.ssid());

    d.setTextSize(1);
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(6, 59);
    d.println("PASSWORD");
    d.setTextColor(TFT_YELLOW, TFT_BLACK);
    d.setTextSize(2);
    d.setCursor(6, 69);
    d.println(portal.password());

    d.setTextSize(1);
    d.setTextColor(TFT_GREEN, TFT_BLACK);
    d.setCursor(6, 94);
    d.println("http://pocketboy");
    d.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    d.setCursor(6, 108);
    d.printf("Chrome controller lab   P:%u", portal.connectedClients());
    d.setTextColor(TFT_DARKGREY, TFT_BLACK);
    d.setCursor(6, 122);
    d.printf("Face: QR   M5: wake   %s", kVersion);
}

void drawQr() {
    auto &d = M5.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextColor(TFT_GREEN, TFT_BLACK);
    d.setTextSize(2);
    d.setCursor(6, 4);
    d.println("Wi-Fi QR");
    d.setTextSize(1);
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(6, 29);
    d.println("Scan to join");
    d.setTextColor(TFT_YELLOW, TFT_BLACK);
    d.setCursor(6, 44);
    d.println(portal.ssid());
    d.setCursor(6, 59);
    d.println(portal.password());
    const String payload = "WIFI:T:WPA;S:" + portal.ssid() + ";P:" + portal.password() + ";H:false;;";
    d.qrcode(payload.c_str(), 115, 5, 120, 4, false);
    d.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    d.setCursor(6, 109);
    d.println("Face: back");
}

void wakeDisplay() {
    lastInteractionAt = millis();
    if (dimmed) {
        dimmed = false;
        M5.Display.setBrightness(kNormalBrightness);
    }
}
} // namespace

void setup() {
    Serial.begin(115200);
    auto config = M5.config();
    M5.begin(config);
    M5.Display.setRotation(3);
    M5.Display.setBrightness(kNormalBrightness);
    const uint32_t splashStartedAt = millis();
    drawSplash();
    M5.Power.setExtOutput(false);
    pinMode(kFacePin, INPUT_PULLUP);
    pinMode(kM5Pin, INPUT_PULLUP);
    portal.begin();
    const uint32_t splashElapsed = millis() - splashStartedAt;
    if (splashElapsed < kMinimumSplashMs) delay(kMinimumSplashMs - splashElapsed);
    lastInteractionAt = millis();
    drawStatus();
}

void loop() {
    portal.loop();
    const uint32_t now = millis();
    if (pressed(kFacePin)) {
        wakeDisplay();
        showingQr = !showingQr;
        if (showingQr) drawQr(); else drawStatus();
    }
    if (pressed(kM5Pin)) {
        wakeDisplay();
        if (!showingQr) drawStatus();
    }

    const uint32_t clients = portal.connectedClients();
    if (clients != lastClientCount) {
        lastClientCount = clients;
        if (!showingQr && !dimmed) drawStatus();
    }
    if (!dimmed && now - lastInteractionAt >= kDimAfterMs) {
        dimmed = true;
        M5.Display.setBrightness(kDimBrightness);
    }
    delay(2);
}
