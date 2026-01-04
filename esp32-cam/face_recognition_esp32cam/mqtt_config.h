#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

// ==================== MQTT CONFIGURATION ====================

// WiFi Credentials
#define WIFI_SSID           "your_wifi_ssid"
#define WIFI_PASSWORD       "your_wifi_password"

// MQTT Broker Settings
#define MQTT_BROKER         "mqtt.yourdomain.com"
#define MQTT_PORT           1883
#define MQTT_CLIENT_ID      "esp32cam-kiosk-01"
#define MQTT_RECONNECT_MS   5000

// MQTT Authentication (leave empty if not required)
#define MQTT_USER           ""
#define MQTT_PASSWORD       ""

// ==================== MQTT TOPICS ====================

// Face Recognition Events
// Topic: hotel/kiosk/<device_id>/face/recognized
// Payload: {"name": "person_name", "confidence": 0.95, "timestamp": 1234567890}
#define MQTT_TOPIC_FACE_RECOGNIZED  "hotel/kiosk/cam01/face/recognized"

// Unknown face detected (confidence below threshold)
// Topic: hotel/kiosk/<device_id>/face/unknown
// Payload: {"confidence": 0.45, "timestamp": 1234567890}
#define MQTT_TOPIC_FACE_UNKNOWN     "hotel/kiosk/cam01/face/unknown"

// Camera status (online/offline, errors)
// Topic: hotel/kiosk/<device_id>/status
// Payload: {"status": "online", "uptime": 12345, "model_ready": true}
#define MQTT_TOPIC_STATUS           "hotel/kiosk/cam01/status"

// Heartbeat (periodic status update)
// Topic: hotel/kiosk/<device_id>/heartbeat
// Payload: {"uptime": 12345, "free_heap": 123456}
#define MQTT_TOPIC_HEARTBEAT        "hotel/kiosk/cam01/heartbeat"

// Control topic (subscribe to receive commands)
// Topic: hotel/kiosk/<device_id>/control
// Payload: {"command": "restart|capture|status"}
#define MQTT_TOPIC_CONTROL          "hotel/kiosk/cam01/control"

// ==================== TIMING CONFIGURATION ====================

// Heartbeat interval in milliseconds
#define HEARTBEAT_INTERVAL_MS       30000

// Status publish on recognition (debounce)
#define RECOGNITION_DEBOUNCE_MS     2000

// WiFi/MQTT reconnection check interval
#define RECONNECT_CHECK_MS          10000

#endif  // MQTT_CONFIG_H
