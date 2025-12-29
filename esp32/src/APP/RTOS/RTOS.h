#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"



extern QueueHandle_t commandQueue; 
extern EventGroupHandle_t xCreatedEventGroup;


void vtask_one_create( void );
void vtask_two_create( void );
void SemaphoreCreateCounting (u_int8_t num , u_int8_t init);
void create_print_task(int input);
extern SemaphoreHandle_t xSemaphore_;
void create_tasks(int input);
