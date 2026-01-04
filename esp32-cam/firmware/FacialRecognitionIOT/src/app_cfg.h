#ifndef APP_CFG_H
#define APP_CFG_H

/* =========================
 * Standard Definitions
 * ========================= */
#define STD_ON   1
#define STD_OFF  0

#define INIT_VALUE 0 
/* =========================
 * Module Enables
 * ========================= */
#define GPIO_ENABLED        STD_ON
#define SENSORH_ENABLED     STD_ON
#define POT_ENABLED         STD_ON
#define UART_ENABLED        STD_ON
#define ALARM_ENABLED       STD_OFF
#define CHATAPP_ENABLED     STD_OFF
#define SPI_ENABLED         STD_OFF
#define I2C_ENABLED         STD_OFF
#define LED_ENABLED         STD_ON
#define LITTLEFS_ENABLED    STD_OFF
#define LM35_ENABLED        STD_ON
#define WIFI_ENABLED        STD_ON
#define MQTT_ENABLED        STD_ON
#define DHT22_ENABLED       STD_ON
#define LDR_1_ENABLED       STD_ON
#define MQ5_1_ENABLED       STD_ON
/* =========================
 * Debug Flags
 * ========================= */
#define GPIO_DEBUG          STD_ON
#define SENSORH_DEBUG       STD_ON
#define POT_DEBUG           STD_OFF
#define UART_DEBUG          STD_ON
#define ALARM_DEBUG         STD_OFF
#define CHATAPP_DEBUG       STD_OFF
#define SPI_DEBUG           STD_OFF
#define I2C_DEBUG           STD_OFF
#define LED_DEBUG           STD_OFF
#define LITTLEFS_DEBUG      STD_OFF
#define LM35_DEBUG          STD_ON
#define WIFI_DEBUG          STD_ON
#define MQTT_DEBUG          STD_ON
#define DHT22_DEBUG         STD_ON
#define LDR_1_DEBUG         STD_ON
#define MQ5_1_DEBUG         STD_ON
/* =========================
 * UART Configuration
 * ========================= */
#define UART_BAUD_RATE      9600
#define UART_FRAME_LENGTH   SERIAL_8N1
#define UART_TX_PIN         17
#define UART_RX_PIN         16


/* =========================
 * SPI Configuration
 * ========================= */
#define SPI_BUS             SPI_VSPI_BUS
#define SPI_SCK_PIN         18
#define SPI_MISO_PIN        19
#define SPI_MOSI_PIN        23
#define SPI_CS_PIN          5
#define SPI_MODE            SPI_MODE0
#define SPI_FREQUENCY       8000000
#define SPI_BIT_ORDER       MSBFIRST


/* =========================
 * I2C Configuration
 * ========================= */
#define I2C_BUS             I2C0_BUS
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22
#define I2C_FREQUENCY       1000000


// ===================
// Camera Hardware Configuration
// ===================
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// ===========================
// SENSOR SELECTION - Choose your sensor here!
// ===========================
// Uncomment ONLY ONE sensor type:

#define SENSOR_OV2640        // OV2640 - Has hardware JPEG encoder
// #define SENSOR_RHYX_M21_45   // RHYX M21-45 (GC2415) - NO hardware JPEG, limited resolution

// ===========================
// Sensor Configuration Profiles
// ===========================

#ifdef SENSOR_OV2640
    #define SENSOR_NAME "OV2640"
    #define SENSOR_HAS_JPEG true
    #define INITIAL_PIXEL_FORMAT PIXFORMAT_RGB565   // RGB565 for inference
    #define INITIAL_FRAME_SIZE FRAMESIZE_QVGA       // 320x240 for face recognition
    #define INITIAL_GRAB_MODE CAMERA_GRAB_LATEST
    #define INITIAL_JPEG_QUALITY 10
    #define INITIAL_FB_COUNT 2
    #define XCLK_FREQ_HZ 20000000                   // 20MHz
    #define SENSOR_DESCRIPTION "OV2640 - Hardware JPEG encoder, QVGA (320x240) for inference"
    
    // Crop region for 96x96 model input (centered square from 320x240)
    #define CROP_SIZE 240                           // Crop 240x240 from center
    #define CROP_X_OFFSET 40                        // (320 - 240) / 2
    #define CROP_Y_OFFSET 0                         // (240 - 240) / 2
#endif

#ifdef SENSOR_RHYX_M21_45
    #define SENSOR_NAME "RHYX M21-45 (GC2415)"
    #define SENSOR_HAS_JPEG false
    #define INITIAL_PIXEL_FORMAT PIXFORMAT_RGB565
    #define INITIAL_FRAME_SIZE FRAMESIZE_240X240    // 240x240 ONLY!
    #define INITIAL_GRAB_MODE CAMERA_GRAB_WHEN_EMPTY
    #define INITIAL_JPEG_QUALITY 12
    #define INITIAL_FB_COUNT 2
    #define XCLK_FREQ_HZ 20000000                   // 20MHz
    #define SENSOR_DESCRIPTION "RHYX M21-45 - NO hardware JPEG, native 240x240 resolution"
    
    // No cropping needed - native 240x240 output
    #define CROP_SIZE 240
    #define CROP_X_OFFSET 0
    #define CROP_Y_OFFSET 0
#endif

// ===========================
// Validate Sensor Selection
// ===========================
#if !defined(SENSOR_OV2640) && !defined(SENSOR_RHYX_M21_45)
    #error "No sensor selected! Please uncomment ONE sensor type in cam_config.h"
#endif

#if defined(SENSOR_OV2640) && defined(SENSOR_RHYX_M21_45)
    #error "Multiple sensors selected! Please uncomment ONLY ONE sensor type in cam_config.h"
#endif

// ===========================
// Camera Initialization Function
// ===========================
esp_err_t initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    
    // Apply sensor-specific configuration
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = INITIAL_PIXEL_FORMAT;
    config.frame_size = INITIAL_FRAME_SIZE;
    config.grab_mode = INITIAL_GRAB_MODE;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = INITIAL_JPEG_QUALITY;
    config.fb_count = INITIAL_FB_COUNT;

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return err;
    }

    // Get sensor for configuration
    sensor_t* s = esp_camera_sensor_get();
    if (!s) {
        Serial.println("Failed to get sensor!");
        return ESP_FAIL;
    }

    Serial.printf("Detected Sensor PID: 0x%x\n", s->id.PID);
    Serial.printf("Configured for: %s\n", SENSOR_NAME);
    Serial.printf("Description: %s\n", SENSOR_DESCRIPTION);

    // Apply sensor-specific tuning
#ifdef SENSOR_OV2640
    // OV2640 optimal settings for face recognition
    s->set_brightness(s, 1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 1);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_gain_ctrl(s, 1);
    s->set_vflip(s, 0);
    s->set_hmirror(s, 0);
    s->set_lenc(s, 1);  // Enable lens correction
    Serial.println("✓ OV2640 sensor tuning applied (optimized for face recognition)");
#endif

#ifdef SENSOR_RHYX_M21_45
    // RHYX M21-45 minimal tuning (this sensor doesn't support many OV-series settings)
    // The 240x240 native resolution is actually ideal for face recognition models!
    Serial.println("✓ RHYX M21-45 configuration applied (native 240x240 - ideal for ML!)");
#endif

    return ESP_OK;
}

// ===========================
// Get Frame Dimensions
// ===========================
inline int getFrameWidth() {
#ifdef SENSOR_OV2640
    return 320;  // QVGA width
#else
    return 240;  // 240x240
#endif
}

inline int getFrameHeight() {
#ifdef SENSOR_OV2640
    return 240;  // QVGA height
#else
    return 240;  // 240x240
#endif
}



/* =========================
 * POT Configuration
 * ========================= */
#define POT_PIN             34
#define POT_RESOLUTION      12
#define MIN_POT_VALUE       0
#define MAX_POT_VALUE       ((1 << POT_RESOLUTION) - 1)


/* =========================
 * Alarm Configuration
 * ========================= */
#define ALARM_LED_HIGH_PIN          16
#define ALARM_LED_LOW_PIN           17
#define ALARM_LED_DIMMER_PWM_CH     2

#define FIRST_ALARM_STATE           NORMAL_ALARM
#define ALARM_LOW_THRESHOLD_PERCENT 30
#define ALARM_HIGH_THRESHOLD_PERCENT 80

#define MIN_PERCENTAGE              0
#define MAX_PERCENTAGE              100


/* =========================
 * PWM Configuration
 * ========================= */
#define PWM_FREQ            5000
#define PWM_RESOLUTION      8
#define PWM_CHANNEL         0


/* =========================
 * LM35 Temperature Sensor
 * ========================= */
#define LM35_ADC_PIN        33
#define LM35_VREF           3.3f
#define LM35_ADC_MAX        4095.0f


/* =========================
 * Ultrasonic Sensor (US)
 * ========================= */
#define US_TRIG_PIN         5
#define US_ECHO_PIN         18
#define SOUND_SPEED_CM_US   0.0343f
#define US_ECHO_TIMEOUT_US  30000UL


/* =========================
 * LED Configuration
 * ========================= */
#define LED_1_PIN           2
#define LED_2_PIN           5
#define LED_3_PIN           32


/* =========================
 * SENSORS Configuration
 * ========================= */

#define DHT22_PIN           14
#define DHT22_TYPE         DHT22

#define MQ5_PIN             33
#define LDR_PIN             35


#define ADC_MIN_RAW 0
#define ADC_MAX_RAW 4095

#define MQ5_MIN_MAPPED 0
#define MQ5_MAX_MAPPED 255

#define MQ5_MIN_RAW  0
#define MQ5_MAX_RAW  4095

#define ADC_RESOLUTION 12
#define LDR_SAMPLE_COUNT 5

/* =========================
 * WiFi Configuration
 * ========================= */
#define WIFI_SSID           "WE_8C5F0A"
#define WIFI_PASSWORD       "j8m13979"


/* =========================
 * MQTT Configuration
 * ========================= */
#define MQTT_BROKER         "mqtt.saddevastator.qzz.io"
#define MQTT_PORT           1883
#define MQTT_RECONNECT_MS   5000
/* =========================
 * MQTT Topics
 * ========================= */
// MQTT Topics

#define ROOM_TOPIC_LED1_CTRL    "hotel/room101/led1/control"
#define ROOM_TOPIC_LED2_CTRL    "hotel/room101/led2/control"
#define ROOM_TOPIC_LED1_STATUS  "room/led1/status"
#define ROOM_TOPIC_LED2_STATUS  "room/led2/status"
#define ROOM_TOPIC_LDR_RAW      "room/ldr/raw"
#define ROOM_TOPIC_LDR_PERCENT  "room/ldr/percentage"
#define ROOM_TOPIC_MODE_CTRL    "room/mode/control"      // Set mode: AUTO/MANUAL/OFF
#define ROOM_TOPIC_MODE_STATUS  "room/mode/status"       // Current mode status
#define ROOM_TOPIC_AUTO_DIM     "room/auto_dim/control"  // Deprecated - use mode instead

#define MQTT_TOPIC_TEMP         "hotel/room101/thermostat/temperature"
#define MQTT_TOPIC_HUMIDITY     "home/thermostat/humidity"
#define MQTT_TOPIC_TARGET       "home/thermostat/target"
#define MQTT_TOPIC_HEATING      "home/thermostat/heating"
#define MQTT_TOPIC_LUMINOSITY   "home/thermostat/luminosity"
#define MQTT_TOPIC_GAS          "home/thermostat/gas"
#define MQTT_TOPIC_CONTROL      "home/thermostat/control"
#define MQTT_TOPIC_SET_SPEED    "home/thermostat/fanspeed"



/* =========================
 * Thermostat Configuration
 * ========================= */
#define THERMOSTAT_UPDATE_RATE_MS   1000
#define THERMOSTAT_PUBLISH_RATE_MS  5000
#define THERMOSTAT_TEMP_DEADBAND    0.5f

#define TEMP_MIN            15.0f
#define TEMP_MAX            35.0f
#define HUMIDITY_MIN        20.0f
#define HUMIDITY_MAX        90.0f


/* =========================
 * System Configuration
 * ========================= */
#define SERIAL_BAUD_RATE    115200


/* =========================
 * SMS
 * ========================= */
#define SMS_RECIPIENT "+201120076894"

#endif /* APP_CFG_H */