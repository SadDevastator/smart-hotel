#include <Arduino.h>
#include "APP/RTOS/RTOS.h"

#define Printers_Num  4
void setup()  
{
    Serial.begin(9600);

    xSemaphore_=  xSemaphoreCreateCounting(10,0);
    xCreatedEventGroup = xEventGroupCreate();

    create_tasks(4);
}

void loop() 
{
    delay(100);
  
}