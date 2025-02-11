//
// Created by ronger on 2025/2/9.
//

#include "WIFIHandle.h"
#include "WiFi.h"


WIFIHandle::WIFIHandle() : server(80) {}

void WIFIHandle::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    loadCredentials();
}

void WIFIHandle::connect() {
    if (credentials.isNull() || credentials.size() == 0) {
        startAccessPoint();
        return;
    }

    for (JsonPair kv : credentials.as<JsonObject>()) {
        String ssid = kv.key().c_str();
        String password = kv.value().as<String>();
        
        for (int i = 0; i < MAX_RETRIES; i++) {
            if (tryConnect(ssid, password)) {
                Serial.println("Connected successfully!");
                return;
            }
            delay(5000);
        }
    }
    
    startAccessPoint();
}

bool WIFIHandle::tryConnect(const String& ssid, const String& password) {
    Serial.printf("Trying to connect to: %s\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(500);
    }
    return false;
}

void WIFIHandle::startAccessPoint() {
    apMode = true;
    String apSSID = "esp32-" + getMacAddress();
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str());
    
    server.on("/", HTTP_GET, [this](){ servePortal(); });
    server.on("/configure", HTTP_POST, [this](){ handleFormSubmit(); });
    server.begin();
    
    Serial.println("AP Mode Started");
    Serial.println("SSID: " + apSSID);
    Serial.println("IP: " + WiFi.softAPIP().toString());
}

void WIFIHandle::servePortal() {
    String html = R"(
        <form action='/configure' method='post'>
            SSID: <input type='text' name='ssid'><br>
            Password: <input type='password' name='password'><br>
            <input type='submit' value='Connect'>
        </form>
    )";
    server.send(200, "text/html", html);
}

void WIFIHandle::handleFormSubmit() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0) {
        saveCredentials(ssid, password);
        server.send(200, "text/plain", "Credentials saved. Connecting...");
        delay(1000);
        if (tryConnect(ssid, password)) {
            resetAndRestart();
        }
    }
}
