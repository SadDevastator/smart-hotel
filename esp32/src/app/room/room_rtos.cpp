#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "room_rtos.h"
#include "room_logic.h"
#include "room_config.h"
#include "../../hal/communication/hal_mqtt/hal_mqtt.h"
#include "../../hal/sensors/hal_rfid/hal_rfid.h"
#include "../../hal/hal_led/hal_led.h"
// Task handles
TaskHandle_t room_sensor_task_handle = NULL;
TaskHandle_t room_control_task_handle = NULL;
TaskHandle_t room_mqtt_task_handle = NULL;
TaskHandle_t room_button_task_handle = NULL;

// Queue handles
QueueHandle_t room_mqtt_rx_queue = NULL;
QueueHandle_t room_mqtt_tx_queue = NULL;

// Mutex handles
SemaphoreHandle_t room_status_mutex = NULL;
SemaphoreHandle_t room_mutex;


//////////////////////// RFID 
TaskHandle_t room_rfid_task_handle = NULL;
QueueHandle_t room_rfid_event_queue = NULL;


// Internal function prototypes
static void Room_RTOS_WiFiConnect(void);
static void Room_RTOS_MQTTConnect(void);
static void Room_RTOS_MQTTCallback(char* topic, byte* payload, unsigned int length);

void Room_RTOS_Init(void)
{
    ROOM_DEBUG_PRINTLN("Room RTOS: Initializing...");
    
    // Create mutex
    room_status_mutex = xSemaphoreCreateMutex();
    
    // Create queues
    room_mqtt_rx_queue = xQueueCreate(ROOM_MQTT_QUEUE_SIZE, sizeof(Room_MQTTMessage_t));
    room_mqtt_tx_queue = xQueueCreate(ROOM_MQTT_QUEUE_SIZE, sizeof(Room_MQTTMessage_t));
    
    room_mutex = xSemaphoreCreateMutex();
    
    room_rfid_event_queue = xQueueCreate(5, sizeof(Room_RFID_Event_t));

    // Create tasks
    xTaskCreate(
        Room_RTOS_SensorTask,
        "SensorTask",
        ROOM_TASK_STACK_SIZE_SMALL,
        NULL,
        ROOM_TASK_PRIORITY_MEDIUM,
        &room_sensor_task_handle
    );
    
    xTaskCreate(
        Room_RTOS_ControlTask,
        "ControlTask",
        ROOM_TASK_STACK_SIZE_SMALL,
        NULL,
        ROOM_TASK_PRIORITY_MEDIUM,
        &room_control_task_handle
    );
    
    
    xTaskCreate(
        Room_RTOS_ButtonTask,
        "ButtonTask",
        4096,
        NULL,
        ROOM_TASK_PRIORITY_MEDIUM,
        &room_button_task_handle
    );


    xTaskCreate(
    Room_RTOS_RFIDTask,
    "RFIDTask",
    4096,                 // RFID + SPI + String = BIG stack
    NULL,
    ROOM_TASK_PRIORITY_MEDIUM,
    &room_rfid_task_handle
    );

    
    ROOM_DEBUG_PRINTLN("Room RTOS: Initialized");
}

// ============================================================================
// Sensor Task - Reads LDR and updates values
// ============================================================================
void Room_RTOS_SensorTask(void* parameter)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(5000); // 100ms
    
    while (1) {
        // Update LDR reading
        if (xSemaphoreTake(room_status_mutex, portMAX_DELAY)) {
            Room_Logic_UpdateLDR();
            xSemaphoreGive(room_status_mutex);
        }
        
        // Publish LDR data every 5 seconds
        static uint8_t counter = 0;
        if (++counter >= 50) { // 50 * 100ms = 5000ms
            counter = 0;
            Room_RTOS_PublishLDRData();
        }
        
        vTaskDelayUntil(&last_wake_time, frequency);
    }
}

// ============================================================================
// Control Task - Handles auto-dimming logic
// ============================================================================
void Room_RTOS_ControlTask(void* parameter)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(100); // 100ms
    
    while (1) {
        // Update auto mode if enabled
        if (xSemaphoreTake(room_status_mutex, portMAX_DELAY)) {
            Room_Logic_UpdateAutoMode();
            xSemaphoreGive(room_status_mutex);
        }
        



        Room_RFID_Event_t rfid_event;

        if (xQueueReceive(room_rfid_event_queue, &rfid_event, 0) == pdTRUE) {

            switch (rfid_event.type) {

                case RFID_EVENT_AUTH_GRANTED:
                    ROOM_DEBUG_PRINT("[RFID] Access granted: ");
                    ROOM_DEBUG_PRINTLN(rfid_event.uid);
                    break;

                case RFID_EVENT_AUTH_DENIED:
                    ROOM_DEBUG_PRINT("[RFID] Access denied: ");
                    ROOM_DEBUG_PRINTLN(rfid_event.uid);
                    break;

                default:
                    break;
            }
        }

        vTaskDelayUntil(&last_wake_time, frequency);
    }
}

// ============================================================================
// MQTT Task - Handles MQTT connection and message processing
// ============================================================================
Room_MQTTMessage_t tx_message;
Room_MQTTMessage_t rx_message;

void Room_RTOS_MQTTWarrper(void )
{
            
        // Process outgoing messages
        if (xQueueReceive(room_mqtt_tx_queue, &tx_message, 0) == pdTRUE) {
            MQTT_Publish(tx_message.topic, tx_message.payload);
            ROOM_DEBUG_PRINT("Published: ");
            ROOM_DEBUG_PRINT(tx_message.topic);
            ROOM_DEBUG_PRINT(" = ");
            ROOM_DEBUG_PRINTLN(tx_message.payload);
        }
        
        // Process incoming messages
        if (xQueueReceive(room_mqtt_rx_queue, &rx_message, 0) == pdTRUE) {
            if (xSemaphoreTake(room_status_mutex, portMAX_DELAY)) {
                Room_Logic_ProcessMQTTMessage(rx_message.topic, rx_message.payload);

                if (strcmp(rx_message.topic, ROOM_TOPIC_MODE_CTRL) == 0) {
                    Room_RTOS_PublishModeStatus();
                } else if (strcmp(rx_message.topic, ROOM_TOPIC_LED1_CTRL) == 0) {
                    Room_RTOS_PublishLEDStatus(ROOM_LED_1);
                } else if (strcmp(rx_message.topic, ROOM_TOPIC_LED2_CTRL) == 0) {
                    Room_RTOS_PublishLEDStatus(ROOM_LED_2);
                } else if (strcmp(rx_message.topic, ROOM_TOPIC_AUTO_DIM) == 0) {
                    Room_RTOS_PublishModeStatus();  // Auto-dim maps to mode
                }
                xSemaphoreGive(room_status_mutex);

            }  

            }
            
            // Publish status update
}

// ============================================================================
// Button Task - Handles button input
// ============================================================================
void Room_RTOS_ButtonTask(void* parameter)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(1000); // 50ms
    
    while (1) {
        // Process button presses
        if (xSemaphoreTake(room_status_mutex, portMAX_DELAY)) {
            Room_Logic_ProcessButtons();
            xSemaphoreGive(room_status_mutex);
        }
        
        vTaskDelayUntil(&last_wake_time, frequency);
    }
}

void Room_RTOS_RFIDTask(void *parameter)
{
    ROOM_DEBUG_PRINTLN("[RFID TASK] Starting...");

    if (!RFID_INIT()) {
        ROOM_DEBUG_PRINTLN("[RFID TASK] RFID init failed!");
        vTaskDelete(NULL);
    }

    Room_RFID_Event_t event;
    String uid;

    while (1) {

        if (RFID_IsNewCardPresent()) {

            if (RFID_ReadCard(&uid, NULL, NULL)) {

                memset(&event, 0, sizeof(event));
                strncpy(event.uid, uid.c_str(), sizeof(event.uid) - 1);
                event.type = RFID_EVENT_CARD_DETECTED;

                xQueueSend(room_rfid_event_queue, &event, 0);

                if (RFID_CheckAuthorization(uid)) {
                    event.type = RFID_EVENT_AUTH_GRANTED;
                    LED_ON(ACCESS_CONTROL);

                } else {
                    event.type = RFID_EVENT_AUTH_DENIED;
                }

                xQueueSend(room_rfid_event_queue, &event, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200)); // RFID polling interval
    }
}


// ============================================================================
// Queue Management Functions
// ============================================================================

bool Room_RTOS_SendMQTTMessage(const Room_MQTTMessage_t* message)
{
    if (room_mqtt_tx_queue == NULL || message == NULL) {
        return false;
    }
    return xQueueSend(room_mqtt_tx_queue, message, pdMS_TO_TICKS(100)) == pdTRUE;
}

bool Room_RTOS_ReceiveMQTTMessage(Room_MQTTMessage_t* message, uint32_t timeout_ms)
{
    if (room_mqtt_rx_queue == NULL || message == NULL) {
        return false;
    }
    return xQueueReceive(room_mqtt_rx_queue, message, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void Room_RTOS_PublishLEDStatus(Room_LED_t led)
{
    Room_MQTTMessage_t message;
    Room_LED_State_t state = Room_Logic_GetLEDState(led);
    
    if (led == ROOM_LED_1) {
        strcpy(message.topic, ROOM_TOPIC_LED1_STATUS);
    } else {
        strcpy(message.topic, ROOM_TOPIC_LED2_STATUS);
    }
    
    strcpy(message.payload, (state == ROOM_LED_ON) ? "ON" : "OFF");
    message.length = strlen(message.payload);
    
    Room_RTOS_SendMQTTMessage(&message);
}

void Room_RTOS_PublishLDRData(void)
{
    Room_MQTTMessage_t message;
    //uint16_t raw_value = Room_Logic_GetLDRRaw();
    uint16_t percentage = Room_Logic_GetLDRPercentage();
    
    // Publish raw value
    /*
    strcpy(message.topic, ROOM_TOPIC_LDR_RAW);
    sprintf(message.payload, "%d", raw_value);
    message.length = strlen(message.payload);
    Room_RTOS_SendMQTTMessage(&message);
    */
    // Publish percentage
    strcpy(message.topic, ROOM_TOPIC_LDR_PERCENT);
    sprintf(message.payload, "%d", percentage);
    message.length = strlen(message.payload);
    Room_RTOS_SendMQTTMessage(&message);
}

void Room_RTOS_PublishModeStatus(void)
{
    Room_MQTTMessage_t message;
    
    strcpy(message.topic, ROOM_TOPIC_MODE_STATUS);
    strcpy(message.payload, Room_Logic_GetModeString());
    message.length = strlen(message.payload);
    
    Room_RTOS_SendMQTTMessage(&message);
}

// ============================================================================
// Internal Functions
// ============================================================================



