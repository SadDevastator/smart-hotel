#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqtt_config.h"

// ==================== MQTT CLIENT CLASS ====================

class MQTTClient {
public:
    MQTTClient();
    
    // Initialize WiFi and MQTT connections
    bool begin();
    
    // Call in loop() to maintain connections
    void loop();
    
    // Check connection status
    bool isConnected();
    
    // Publish face recognition event
    bool publishRecognition(const char* name, float confidence);
    
    // Publish unknown face event
    bool publishUnknown(float confidence);
    
    // Publish status update
    bool publishStatus(bool modelReady);
    
    // Publish heartbeat
    bool publishHeartbeat();
    
    // Set callback for control commands
    void setControlCallback(void (*callback)(const char* command));

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    unsigned long lastHeartbeat;
    unsigned long lastReconnectAttempt;
    unsigned long lastRecognitionTime;
    String lastRecognizedPerson;
    
    void (*controlCallback)(const char* command);
    
    bool connectWiFi();
    bool connectMQTT();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    
    static MQTTClient* instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

// ==================== IMPLEMENTATION ====================

MQTTClient* MQTTClient::instance = nullptr;

MQTTClient::MQTTClient() : mqttClient(wifiClient) {
    lastHeartbeat = 0;
    lastReconnectAttempt = 0;
    lastRecognitionTime = 0;
    lastRecognizedPerson = "";
    controlCallback = nullptr;
    instance = this;
}

bool MQTTClient::begin() {
    Serial.println("[MQTT] Initializing...");
    
    // Connect to WiFi
    if (!connectWiFi()) {
        return false;
    }
    
    // Configure MQTT
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(512);
    
    // Connect to MQTT broker
    if (!connectMQTT()) {
        Serial.println("[MQTT] Initial connection failed, will retry...");
    }
    
    return true;
}

bool MQTTClient::connectWiFi() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println();
        Serial.println("[WiFi] Connection failed!");
        return false;
    }
}

bool MQTTClient::connectMQTT() {
    if (!WiFi.isConnected()) {
        return false;
    }
    
    Serial.printf("[MQTT] Connecting to %s:%d...\n", MQTT_BROKER, MQTT_PORT);
    
    bool connected = false;
    if (strlen(MQTT_USER) > 0) {
        connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
    } else {
        connected = mqttClient.connect(MQTT_CLIENT_ID);
    }
    
    if (connected) {
        Serial.println("[MQTT] Connected!");
        
        // Subscribe to control topic
        mqttClient.subscribe(MQTT_TOPIC_CONTROL);
        Serial.printf("[MQTT] Subscribed to: %s\n", MQTT_TOPIC_CONTROL);
        
        // Publish online status
        publishStatus(true);
        
        return true;
    } else {
        Serial.printf("[MQTT] Connection failed, rc=%d\n", mqttClient.state());
        return false;
    }
}

void MQTTClient::loop() {
    unsigned long now = millis();
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        if (now - lastReconnectAttempt >= RECONNECT_CHECK_MS) {
            lastReconnectAttempt = now;
            Serial.println("[WiFi] Reconnecting...");
            connectWiFi();
        }
        return;
    }
    
    // Check MQTT connection
    if (!mqttClient.connected()) {
        if (now - lastReconnectAttempt >= MQTT_RECONNECT_MS) {
            lastReconnectAttempt = now;
            connectMQTT();
        }
        return;
    }
    
    // Process MQTT messages
    mqttClient.loop();
    
    // Send periodic heartbeat
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        lastHeartbeat = now;
        publishHeartbeat();
    }
}

bool MQTTClient::isConnected() {
    return mqttClient.connected();
}

bool MQTTClient::publishRecognition(const char* name, float confidence) {
    if (!mqttClient.connected()) {
        return false;
    }
    
    unsigned long now = millis();
    
    // Debounce: don't publish same person within threshold
    if (lastRecognizedPerson == name && 
        (now - lastRecognitionTime) < RECOGNITION_DEBOUNCE_MS) {
        return true;  // Skip but return success
    }
    
    lastRecognizedPerson = name;
    lastRecognitionTime = now;
    
    // Build JSON payload
    StaticJsonDocument<256> doc;
    doc["name"] = name;
    doc["confidence"] = confidence;
    doc["timestamp"] = now / 1000;  // Seconds since boot
    doc["device"] = MQTT_CLIENT_ID;
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    bool success = mqttClient.publish(MQTT_TOPIC_FACE_RECOGNIZED, buffer);
    
    if (success) {
        Serial.printf("[MQTT] Published recognition: %s (%.1f%%)\n", name, confidence * 100);
    } else {
        Serial.println("[MQTT] Failed to publish recognition");
    }
    
    return success;
}

bool MQTTClient::publishUnknown(float confidence) {
    if (!mqttClient.connected()) {
        return false;
    }
    
    unsigned long now = millis();
    
    // Don't spam unknown detections
    if (lastRecognizedPerson == "unknown" && 
        (now - lastRecognitionTime) < (RECOGNITION_DEBOUNCE_MS * 2)) {
        return true;
    }
    
    lastRecognizedPerson = "unknown";
    lastRecognitionTime = now;
    
    StaticJsonDocument<128> doc;
    doc["confidence"] = confidence;
    doc["timestamp"] = now / 1000;
    doc["device"] = MQTT_CLIENT_ID;
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    return mqttClient.publish(MQTT_TOPIC_FACE_UNKNOWN, buffer);
}

bool MQTTClient::publishStatus(bool modelReady) {
    if (!mqttClient.connected()) {
        return false;
    }
    
    StaticJsonDocument<256> doc;
    doc["status"] = "online";
    doc["uptime"] = millis() / 1000;
    doc["model_ready"] = modelReady;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["ip"] = WiFi.localIP().toString();
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    return mqttClient.publish(MQTT_TOPIC_STATUS, buffer, true);  // Retained
}

bool MQTTClient::publishHeartbeat() {
    if (!mqttClient.connected()) {
        return false;
    }
    
    StaticJsonDocument<128> doc;
    doc["uptime"] = millis() / 1000;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    return mqttClient.publish(MQTT_TOPIC_HEARTBEAT, buffer);
}

void MQTTClient::setControlCallback(void (*callback)(const char* command)) {
    controlCallback = callback;
}

void MQTTClient::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (instance != nullptr) {
        instance->handleMessage(topic, payload, length);
    }
}

void MQTTClient::handleMessage(char* topic, byte* payload, unsigned int length) {
    // Null-terminate payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.printf("[MQTT] Received: %s -> %s\n", topic, message);
    
    // Parse control commands
    if (strcmp(topic, MQTT_TOPIC_CONTROL) == 0) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error && doc.containsKey("command")) {
            const char* command = doc["command"];
            
            if (controlCallback != nullptr) {
                controlCallback(command);
            }
            
            // Handle built-in commands
            if (strcmp(command, "status") == 0) {
                publishStatus(true);
            } else if (strcmp(command, "restart") == 0) {
                Serial.println("[MQTT] Restart command received");
                delay(1000);
                ESP.restart();
            }
        }
    }
}

#endif  // MQTT_CLIENT_H
