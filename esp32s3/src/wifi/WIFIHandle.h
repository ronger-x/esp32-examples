//
// Created by ronger on 2025/2/9.
//

#ifndef WIFI_HANDLE_H
#define WIFI_HANDLE_H

#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define MAX_RETRIES 3
#define CREDENTIALS_FILE "/wifi_config.json"

class WIFIHandle {
public:
    WIFIHandle();
    void begin(); // 初始化Wi-Fi处理
    void connect(); // 尝试连接到Wi-Fi
    void startAccessPoint(); // 启动AP模式
    void handleClient(); // 处理连接到AP的客户端
    void saveCredentials(const String& ssid, const String& password); // 存储Wi-Fi凭据

private:
    struct WifiCredential {
        String ssid;
        String password;
    };

    WebServer server;
    bool apMode = false;
    JsonDocument credentials;

    bool tryConnect(const String& ssid, const String& password); // 尝试连接指定的SSID
    void loadCredentials(); // 从存储中加载SSID和password
    void resetAndRestart(); // 重置设备并重启
    String getMacAddress(); // 获取设备的MAC地址
    void servePortal();
    void handleFormSubmit();
};


#endif //WIFI_HANDLE_H
