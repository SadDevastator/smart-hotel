#include "hal/communication/hal_wifi/hal_wifi.h"
#include "hal/communication/hal_mqtt/hal_mqtt.h"
#include "app_cfg.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize WiFi (MQTT auto-initializes when connected)
    WIFI_Init(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    // Process WiFi (handles reconnection)
    WIFI_Process();
    
    // Process MQTT (handles reconnection & messages)
    MQTT_Loop();
    
    // Publish example
    if (WIFI_IsConnected() && MQTT_IsConnected()) {
        MQTT_Publish("test/topic", "Hello");
        delay(5000);
    }
}