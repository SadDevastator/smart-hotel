#include "hal_mqtt.h"
#include <WiFi.h>
#include "../hal_wifi/hal_wifi.h"
#include "../../../app_cfg.h"

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static const char* g_broker;
static int g_port;

/**
 * @brief MQTT message callback - handles incoming messages
 */
static void MQTT_MessageCallback(char* topic, uint8_t* payload, unsigned int length) {
    char message[128] = {0};
    if (length >= sizeof(message)) {
        length = sizeof(message) - 1;
    }
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.printf("[MQTT RX] %s: %s\n", topic, message);
    
    // Add your message handling here if needed
    // For minimal functionality, just print received messages
}

/**
 * @brief Attempt MQTT reconnection
 */
static void MQTT_Reconnect(void) {
    if (!WIFI_IsConnected()) {
        return;  // Don't attempt if WiFi is down
    }
    
    if (!mqttClient.connected()) {
        String id = "ESP32-" + "FacialRecognition";  // Unique client ID
        if (mqttClient.connect(id.c_str())) {
            Serial.println("[MQTT] Connected");
            // Subscribe to topics here if needed
            // mqttClient.subscribe("your/topic");
        }
    }
}

/**
 * @brief Initialize MQTT client
 */
void MQTT_Init(const char* broker, int port) {
    g_broker = broker;
    g_port = port;
    mqttClient.setServer(g_broker, g_port);
    mqttClient.setCallback(MQTT_MessageCallback);
}

/**
 * @brief Main MQTT loop - call this frequently
 */
void MQTT_Loop(void) {
    if (WIFI_IsConnected()) {
        if (!mqttClient.connected()) {
            MQTT_Reconnect();
        }
        mqttClient.loop();
    }
}

/**
 * @brief Publish message to MQTT topic
 */
void MQTT_Publish(const char* topic, const char* payload) {
    if (!WIFI_IsConnected() || !mqttClient.connected()) {
        Serial.println("[MQTT] Publish failed - not connected");
        return;
    }

    if (mqttClient.publish(topic, payload)) {
        Serial.printf("[MQTT TX] %s: %s\n", topic, payload);
    } else {
        Serial.println("[MQTT] Publish failed");
    }
}

/**
 * @brief Check if MQTT is connected
 */
bool MQTT_IsConnected(void) {
    return mqttClient.connected();
}

/**
 * @brief Subscribe to MQTT topic
 */
void MQTT_Subscribe(const char* topic) {
    if (MQTT_IsConnected()) {
        mqttClient.subscribe(topic);
        Serial.printf("[MQTT] Subscribed: %s\n", topic);
    }
}