#ifndef APP_CFG_H
#define APP_CFG_H

// General Definitions
#define STD_ON 1
#define STD_OFF 0

// Module definitions
#define GPIO_ENABLED STD_ON
#define SENSORH_ENABLED STD_ON
#define POT_ENABLED STD_ON
#define UART_ENABLED STD_ON
#define ALARM_ENABLED STD_OFF
#define CHATAPP_ENABLED STD_OFF
#define SPI_ENABLED STD_OFF
#define I2C_ENABLED STD_OFF
#define LED_ENABLED STD_ON
#define LITTELFS_ENABLED STD_OFF
#define LM35_ENABLED STD_ON
#define WIFI_ENABLED STD_ON
// Debug Definitions
#define GPIO_DEBUG STD_ON
#define SENSORH_DEBUG STD_ON
#define POT_DEBUG STD_ON
#define ALARM__DEBUG STD_OFF
#define UART_DEBUG STD_ON
#define CHATAPP_DEBUG STD_OFF
#define SPI_DEBUG STD_OFF
#define I2C_DEBUG STD_OFF
#define LED_DEBUG STD_ON
#define LITTELFS_DEBUG STD_OFF
#define LM35_DEUG STD_ON
#define WIFI_DEBUG STD_ON
// UART Configuration
#define UART_BUAD_RATE 9600
#define UART_FRAMELENGTH SERIAL_8N1
#define UART_TXPIN 17
#define UART_RXPIN 16
// #define UART_MAXLENGH 1

// SPI CONFIGRATION
#define SPI_BUS SPI_VSPI_BUS
#define SPI_SCK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 23
#define SPI_CS_PIN 5
//    SPI_MODE0,SPI_MODE1,SPI_MODE2,SPI_MODE3
#define SPI_MODE SPI_MODE0
#define SPI_FREQUANCY 8000000
// MSBFIRST or LSBFIRST
#define SPI_BIT_ORDER MSBFIRST

// I2C CONFIGRATION
#define I2C_BUS I2C0_BUS
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#define I2C_FREQUANCY 1000000
// MSBFIRST or LSBFIRST
#define SPI_BIT_ORDER MSBFIRST

// Pin Configuration
#define POT_PIN 34
#define ALARM_LED_HIGH_PIN 16
#define ALARM_LED_LOW_PIN 17
#define ALARM_LED_DIMMER_PWM_CHANNEL 2

#define POT_RESOLUTION 12

#define FRIST_ALARM_STATE NORMAL_ALARM

#define ALARM_LOW_THRESHOLD_PERCENTAGE 30
#define ALARM_HIGH_THRESHOLD_PERCENTAGE 80

#define MIN_PERCENTAGE 0
#define MAX_PERCENTAGE 100

#define MIN_POT_VALUE 0
#define MAX_POT_VALUE (pow(2, POT_RESOLUTION) - 1)

#define PWM_FREQ 5000 // 5 kHz
#define PWM_RESOLUTION 8
#define PWM_CHANNEL 0

// LM35
#define LM35_ADC_PIN 33
#define LM35_VREF 3.3f
#define LM35_ADC_MAX 4095.0f

// UCS
#define US_TRIG_PIN 5
#define US_ECHO_PIN 18
#define SOUND_SPEED_CM_PER_US 0.0343f
#define US_ECHO_TIMEOUT_US 30000UL
// LED

#define LED_1_PIN 34
#define LED_2_PIN 35
#define LED_3_PIN 32

#define SSID "WE_8C5F0A"
#define PASSWORD "j8m13979"

// Standard definitions
#define STD_ON  1
#define STD_OFF 0

// WiFi Configuration
#define SSID     "WE_8C5F0A"
#define PASSWORD "j8m13979"

// Debug Flags
#define WIFI_DEBUG      STD_ON
#define LED_DEBUG       STD_OFF
#define POT_DEBUG       STD_OFF
#define MQTT_DEBUG      STD_ON

// Feature Enables
#define LED_ENABLED     STD_ON
#define POT_ENABLED     STD_ON
#define MQTT_ENABLED    STD_ON

// POT Configuration
#define POT_PIN         25  // Default POT pin (can be overridden)
#define POT_RESOLUTION  12  // 12-bit ADC (0-4095)

// Thermostat Configuration
#define THERMOSTAT_UPDATE_RATE_MS     1000  // Update sensors every 1 second
#define THERMOSTAT_MQTT_PUBLISH_MS    5000  // Publish to MQTT every 5 seconds
#define THERMOSTAT_TEMP_DEADBAND      0.5f  // Temperature deadband in °C

// Temperature Ranges
#define TEMP_MIN    15.0f   // Minimum temperature (°C)
#define TEMP_MAX    35.0f   // Maximum temperature (°C)
#define HUMIDITY_MIN 20.0f  // Minimum humidity (%)
#define HUMIDITY_MAX 90.0f  // Maximum humidity (%)

// MQTT Configuration
#define MQTT_BROKER         "broker.hivemq.com"
#define MQTT_PORT           1883
#define MQTT_KEEPALIVE      60
#define MQTT_RECONNECT_MS   5000

// SMS Recipient 
#define SMS_RECIPIENT "+201120076894"


// System Configuration
#define SERIAL_BAUD_RATE    115200


#endif
