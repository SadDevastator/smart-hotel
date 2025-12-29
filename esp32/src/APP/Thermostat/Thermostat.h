#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <stdint.h>

// Thermostat modes
typedef enum {
    THERMOSTAT_MODE_OFF = 0,
    THERMOSTAT_MODE_AUTO,
    THERMOSTAT_MODE_MANUAL
} Thermostat_Mode_t;

// Fan speeds
typedef enum {
    FAN_SPEED_OFF = 0,
    FAN_SPEED_LOW,
    FAN_SPEED_MEDIUM,
    FAN_SPEED_HIGH
} Fan_Speed_t;

// Thermostat status structure
typedef struct {
    float temperature;      // Current temperature (from POT1)
    float humidity;         // Current humidity (from POT2)
    float target_temp;      // Target temperature (from POT3)
    Fan_Speed_t fan_speed;  // Current fan speed
    Thermostat_Mode_t mode; // Operating mode
    bool heating;           // Heating status
} Thermostat_Status_t;

// API Functions
void Thermostat_Init(void);
void Thermostat_Process(void);
void Thermostat_SetMode(Thermostat_Mode_t mode);
void Thermostat_SetFanSpeed(Fan_Speed_t speed);
void Thermostat_SetTargetTemp(float temp);
Thermostat_Status_t Thermostat_GetStatus(void);
void Thermostat_PublishData(void);

#endif // THERMOSTAT_H