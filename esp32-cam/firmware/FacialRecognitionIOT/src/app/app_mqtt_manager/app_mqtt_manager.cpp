/**
 * @file app_mqtt_manager.cpp
 * @brief Application Layer - MQTT Manager Implementation
 */

#include "app_mqtt_manager.h"
#include "../../app_cfg.h"
#include "../../hal/hal_mqtt/hal_mqtt.h"
#include <Arduino.h>
#include <time.h>

namespace app {

static char s_currentTopic[256] = {0};
static int s_totalFrames = 0;
static int s_totalRecognized = 0;

bool mqttManagerInit() {
    // Format the topic
    snprintf(s_currentTopic, sizeof(s_currentTopic), 
             "%s/%s", MQTT_TOPIC_BASE, MQTT_LOCATION);
    
    Serial.printf("[App MQTT] Topic: %s\n", s_currentTopic);
    
    return hal::mqttInit();
}

/**
 * @brief Format current timestamp in ISO 8601 format
 * @return Timestamp string buffer (static, reused each call)
 */
static const char* getTimestampISO8601() {
    static char timestampBuf[32];
    
    // Get current time
    time_t now = time(nullptr);
    struct tm* timeinfo = gmtime(&now);
    
    // Format as ISO 8601
    strftime(timestampBuf, sizeof(timestampBuf), 
             "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    
    return timestampBuf;
}

/**
 * @brief Create JSON payload for face detection
 * @return JSON string (static, reused each call)
 */
static const char* formatFaceDetectionPayload(const FaceResult& result) {
    static char payloadBuf[512];
    
    // Create JSON manually (no external library required)
    snprintf(payloadBuf, sizeof(payloadBuf),
             "{"
             "\"person_name\":\"%s\","
             "\"confidence_score\":%.3f,"
             "\"timestamp\":\"%s\","
             "\"recognized\":%s,"
             "\"location\":\"%s\""
             "}",
             result.label,
             result.confidence,
             getTimestampISO8601(),
             result.recognized ? "true" : "false",
             MQTT_LOCATION);
    
    return payloadBuf;
}

bool publishFaceDetection(const FaceResult& result) {
    // Only publish if recognized (or if configured otherwise)
#ifdef PUBLISH_ONLY_RECOGNIZED
    if (!result.recognized) {
        return true;  // Skip but don't fail
    }
#endif
    
    const char* payload = formatFaceDetectionPayload(result);
    
    Serial.printf("[App MQTT] Publishing: %s\n", payload);
    
    return hal::mqttPublish(s_currentTopic, payload);
}

bool publishStatistics(int framesProcessed, int faceRecognized) {
    static char payload[256];
    
    snprintf(payload, sizeof(payload),
             "{"
             "\"frames_processed\":%d,"
             "\"faces_recognized\":%d,"
             "\"timestamp\":\"%s\","
             "\"location\":\"%s\""
             "}",
             framesProcessed,
             faceRecognized,
             getTimestampISO8601(),
             MQTT_LOCATION);
    
    // Publish to stats topic
    static char statsTopic[256];
    snprintf(statsTopic, sizeof(statsTopic), 
             "%s/%s/stats", MQTT_TOPIC_BASE, MQTT_LOCATION);
    
    return hal::mqttPublish(statsTopic, payload);
}

const char* getCurrentTopic() {
    return s_currentTopic;
}

bool isMqttReady() {
    return hal::mqttIsConnected();
}

void mqttManagerProcess() {
    hal::mqttProcess();
}

}  // namespace app
