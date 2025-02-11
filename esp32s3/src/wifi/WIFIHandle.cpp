//
// Created by ronger on 2025/2/9.
//

#include <nvs_flash.h>

#include "WIFIHandle.h"
#include "WiFi.h"

// 定义NVS相关常量
const char* WIFIHandle::NVS_NAMESPACE = "wifi_config";
const char* WIFIHandle::SSID_KEY = "ssid_%d";
const char* WIFIHandle::PASS_KEY = "pass_%d";
const char* WIFIHandle::COUNT_KEY = "net_count";

WIFIHandle::WIFIHandle() : server(80)
{
}

void WIFIHandle::begin()
{
    Serial.println("Initializing Wi-Fi...");
    // 加载凭证列表数据
    loadCredentials();
    // 连接 Wi-Fi
    connect();
}

void WIFIHandle::connect()
{
    if (credentials.empty())
    {
        Serial.println("No stored credentials, starting AP mode");
        startAccessPoint();
        return;
    }

    Serial.printf("Found %d stored networks\n", credentials.size());

    // 按存储顺序遍历凭证
    for (const auto& cred : credentials)
    {
        if (cred.ssid.isEmpty())
        {
            Serial.println("Skipped empty SSID");
            continue;
        }

        Serial.printf("Attempting: %s\n", cred.ssid.c_str());

        // 带重试机制的连接尝试
        for (int retry = 0; retry < MAX_RETRIES; ++retry)
        {
            if (tryConnect(cred.ssid, cred.password))
            {
                Serial.println("Connected successfully!");
                Serial.printf("IP Address: %s\n",
                              WiFi.localIP().toString().c_str());
                WiFi.setAutoConnect(true); // 设置自动连接
                return; // 连接成功，退出函数
            }

            // 指数退避重试策略
            const uint32_t delayTime = 1000 * (1 << retry);
            Serial.printf("Retry %d in %dms\n", retry + 1, delayTime);
            delay(delayTime);
        }
    }

    // 所有尝试失败
    Serial.println("All networks failed, starting AP mode");
}

void WIFIHandle::startAccessPoint()
{
    apMode = true;
    String apSSID = "esp32-" + getMacAddress();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str());

    server.on("/", HTTP_GET, [this]() { servePortal(); });
    server.on("/configure", HTTP_POST, [this]() { handleFormSubmit(); });
    server.begin();

    Serial.println("AP Mode Started");
    Serial.println("SSID: " + apSSID);
    Serial.println("IP: " + WiFi.softAPIP().toString());
}

void WIFIHandle::handleClient() {
    if (apMode) {
        server.handleClient();
    }
}

void WIFIHandle::saveCredentials(const String& ssid, const String& password)
{
    nvs_handle_t handle;
    esp_err_t err;

    // 打开NVS命名空间
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        Serial.printf("NVS open failed: %s\n", esp_err_to_name(err));
        return;
    }

    // 获取当前存储的网络数量
    uint8_t count = 0;
    nvs_get_u8(handle, COUNT_KEY, &count);

    // 检查是否已存在相同SSID
    bool exists = false;
    char key_buffer[16];
    for (uint8_t i = 0; i < count; ++i)
    {
        snprintf(key_buffer, sizeof(key_buffer), SSID_KEY, i);
        char stored_ssid[32];
        size_t len = 32;
        if (nvs_get_str(handle, key_buffer, stored_ssid, &len) == ESP_OK)
        {
            if (strcmp(stored_ssid, ssid.c_str()) == 0)
            {
                exists = true;
                // 更新密码
                snprintf(key_buffer, sizeof(key_buffer), PASS_KEY, i);
                nvs_set_str(handle, key_buffer, password.c_str());
                break;
            }
        }
    }

    // 存储新凭证
    if (!exists)
    {
        snprintf(key_buffer, sizeof(key_buffer), SSID_KEY, count);
        nvs_set_str(handle, key_buffer, ssid.c_str());

        snprintf(key_buffer, sizeof(key_buffer), PASS_KEY, count);
        nvs_set_str(handle, key_buffer, password.c_str());

        count++;
        nvs_set_u8(handle, COUNT_KEY, count);
    }

    // 提交并关闭
    nvs_commit(handle);
    nvs_close(handle);

    // 重新加载凭证
    loadCredentials();
}

bool WIFIHandle::tryConnect(const String& ssid, const String& password)
{
    WiFi.disconnect(true); // 确保清除之前的连接
    delay(100); // 等待清理完成

    Serial.printf("Connecting to %s...", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    constexpr unsigned long timeout = 15000; // 15秒超时
    const unsigned long start = millis();

    while (millis() - start < timeout)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConnected!");
            return true;
        }

        // 非阻塞式检测（每秒检测一次）
        if ((millis() - start) % 1000 == 0)
        {
            Serial.print(".");
        }
        delay(10); // 保持系统响应
    }

    Serial.printf("\nConnection failed (Status: %d)\n", WiFi.status());
    return false;
}

void WIFIHandle::loadCredentials()
{
    credentials.clear();
    nvs_handle_t handle;

    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
    {
        Serial.println("No stored credentials");
        return;
    }

    // 获取网络数量
    uint8_t count = 0;
    nvs_get_u8(handle, COUNT_KEY, &count);
    if (count == 0)
    {
        nvs_close(handle);
        return;
    }

    // 循环读取所有凭证
    for (uint8_t i = 0; i < count; ++i)
    {
        char key_buffer[16];
        char ssid[32];
        char password[64];
        snprintf(key_buffer, sizeof(key_buffer), SSID_KEY, i);
        size_t len = sizeof(ssid);
        if (nvs_get_str(handle, key_buffer, ssid, &len) != ESP_OK)
        {
            continue;
        }

        snprintf(key_buffer, sizeof(key_buffer), PASS_KEY, i);
        len = sizeof(password);
        if (nvs_get_str(handle, key_buffer, password, &len) == ESP_OK)
        {
            credentials.push_back({String(ssid), String(password)});
        }
    }

    nvs_close(handle);
}

void WIFIHandle::resetAndRestart() {
    delay(1000);
    ESP.restart();
}

String WIFIHandle::getMacAddress() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macStr[9];
    snprintf(macStr, sizeof(macStr), "%02X%02X%02X",
             mac[3], mac[4], mac[5]);
    return String(macStr);
}

void WIFIHandle::servePortal()
{
    String html = R"(
        <form action='/configure' method='post'>
            SSID: <input type='text' name='ssid'><br>
            Password: <input type='password' name='password'><br>
            <input type='submit' value='Connect'>
        </form>
    )";
    server.send(200, "text/html", html);
}

void WIFIHandle::handleFormSubmit()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() > 0)
    {
        saveCredentials(ssid, password);
        server.send(200, "text/plain", "Credentials saved. Connecting...");
        delay(1000);
        if (tryConnect(ssid, password))
        {
            resetAndRestart();
        }
    }
}
