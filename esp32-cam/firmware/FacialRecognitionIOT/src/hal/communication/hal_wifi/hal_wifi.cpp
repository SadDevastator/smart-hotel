#include <Arduino.h>
#include <WiFi.h>
#include "../../../app_cfg.h"
#include "hal_wifi.h"
#include "../hal_mqtt/hal_mqtt.h"

static const char* g_ssid = NULL;
static const char* g_password = NULL;
static bool g_connected = false;
static bool g_mqttInitialized = false;
static unsigned long g_lastReconnect = 0;

#define RECONNECT_INTERVAL_MS 5000
#define CONNECT_TIMEOUT_MS 15000

/**
 * @brief Initialize WiFi with credentials
 */
void WIFI_Init(const char* ssid, const char* password) {
    g_ssid = ssid;
    g_password = password;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(g_ssid, g_password);
    
    Serial.println("[WiFi] Connecting...");
    g_lastReconnect = millis();
}

/**
 * @brief Process WiFi state - call this in loop()
 */
void WIFI_Process(void) {
    wl_status_t status = WiFi.status();
    
    // Handle connection state
    if (status == WL_CONNECTED && !g_connected) {
        // Just connected
        g_connected = true;
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
        
        // Initialize MQTT once WiFi is up
        if (!g_mqttInitialized) {
            MQTT_Init(MQTT_BROKER, MQTT_PORT);
            g_mqttInitialized = true;
        }
    }
    else if (status != WL_CONNECTED && g_connected) {
        // Just disconnected
        g_connected = false;
        Serial.println("[WiFi] Disconnected!");
    }
    
    // Auto-reconnect if disconnected
    if (!g_connected && (millis() - g_lastReconnect >= RECONNECT_INTERVAL_MS)) {
        Serial.println("[WiFi] Reconnecting...");
        WiFi.disconnect();
        delay(100);
        WiFi.begin(g_ssid, g_password);
        g_lastReconnect = millis();
    }
}

/**
 * @brief Check if WiFi is connected
 */
bool WIFI_IsConnected(void) {
    return g_connected;
}

/**
 * @brief Get WiFi signal strength
 */
int WIFI_GetRSSI(void) {
    if (g_connected) {
        return WiFi.RSSI();
    }
    return 0;
}