#include "RTOS.h"
#include "../LED/LED.h"
// Global task handle definitions
TaskHandle_t xTaskOneHandle = NULL;
TaskHandle_t xTaskTwoHandle = NULL;
static TaskHandle_t xWorker = NULL;

SemaphoreHandle_t xSemaphore_;
BaseType_t xReturned = pdFALSE ;
EventBits_t uxBits;


EventGroupHandle_t xCreatedEventGroup;

QueueHandle_t commandQueue; 

#define BIT_0	( 1 << 0 )
#define BIT_1	( 1 << 1 )
#define BIT_2	( 1 << 2 )
#define BIT_3	( 1 << 3 )

#define BIT_4	( 1 << 4 )

#define BIT_5	( 1 << 5 )



void SemaphoreCreateCounting (u_int8_t num , u_int8_t init)
{
    xSemaphore_ = xSemaphoreCreateCounting(num,init);
}


void task1_func(void *pvParameters)
{


    for(;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Let others start

                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_0 );/* The bits being set. */
                        Serial.println("SENSEOR READY");

                        vTaskDelete(NULL);


    }
}

/*

task3 take 



*/





void task2_func(void *pvParameters)
{
    for(;;)
    {
        Serial.print(".");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Start after Low has mutex
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_1 );/* The bits being set. */
                        Serial.println("common READY");


                        vTaskDelete(NULL);




    }
}


void task3_func(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Start after Low has mutex
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_2 );/* The bits being set. */
                        Serial.println("user READY");
                                                vTaskDelete(NULL);




    }
}

void task4_func(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Start after Low has mutex
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_3 );/* The bits being set. */
                        Serial.println("storage READY");
                        vTaskDelete(NULL);



    }
}

void task5_func(void *pvParameters)
{
    xCreatedEventGroup = xEventGroupCreate();
    const TickType_t xTicksToWait = 6000 / portTICK_PERIOD_MS;

    const TickType_t xPhase2Timeout = 6000 / portTICK_PERIOD_MS;  // 6 seconds!
    TickType_t xPhase2Deadline = 0;  // When the 6-second timer started
    bool phase_one_flag = false ; 


    for(;;)
    {
        if( xCreatedEventGroup == NULL )
        {
            /* The event group was not created because there was insufficient
            FreeRTOS heap available. */
        }
        else
        {


               uxBits = xEventGroupWaitBits(
               xCreatedEventGroup,   /* The event group being tested. */
               BIT_0 | BIT_4, /* The bits within the event group to wait for. */
               pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */ // pdFALSE  dont clear
               pdFALSE,       /* Don't wait for both bits, either bit will do. */   //pdFALSE OR , PdTRUE AND
               xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */

                    if( (( uxBits & BIT_0 )&&( uxBits & BIT_1 ) != 0 ) && phase_one_flag == false)
                    {
                        Serial.println("PHASE ONE END");
                        phase_one_flag = true;
                        xPhase2Deadline = xTaskGetTickCount() + xPhase2Timeout;
                    }else{
            
                            TickType_t xTicksRemaining = 0;

                            if (xTaskGetTickCount() < xPhase2Deadline)
                                xTicksRemaining = xPhase2Deadline - xTaskGetTickCount();
                            else
                                xTicksRemaining = 0;  

                   if( ( uxBits & ( BIT_0 |BIT_1 |BIT_2 |BIT_3 ) ) == ( BIT_0 |BIT_1 |BIT_2 |BIT_3  ) && phase_one_flag == 1 )
                    {
                        Serial.println("ALL MDEULES READY");
                    }
                    else if (xTicksRemaining == 0)
                    {
                        Serial.println("TIMEOUT! Not all modules ready after 6 seconds");
                        Serial.print("Current bits: 0x");
                        Serial.println(xEventGroupGetBits(xCreatedEventGroup), HEX);

                        phase_one_flag = false;  // example: restart from phase 1
                    }
                    else
                    {
                        Serial.println("Still waiting for remaining modules...");
                    }

        }
    }
    }
}

void taskUI(void *pvParameters)
{
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize

    commandQueue = xQueueCreate(10, sizeof(int));

    Serial.println("\n=== System Ready ===");
    Serial.println("Commands: 1=Start, 2=Stop, 3=Increase, 4=Decrease");
    Serial.print("> ");

    for(;;)
    {
        // This is the magic: only do work when serial data is available
        if (Serial.available() > 0) {
        }

        // Let other tasks run — very important!
        vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize
    }
}








void taskCTR(void *pvParameters)
{
    int receivedCommand = 0 ;
    
    for(;;)
    {
        if (xQueueReceive(commandQueue, &receivedCommand, portMAX_DELAY) == pdPASS)
                {
                    printf("Control Task: Received command %d\n", receivedCommand);

                    switch (receivedCommand)
                    {
                        case 1:
                            printf(" -> Starting system...\n");
                            // Your start logic
                            break;
                        case 2:
                            printf(" -> Stopping system...\n");
                            // Your stop logic
                            break;
                        case 3:
                            printf(" -> Increasing value...\n");
                            // Increase something
                            break;
                        case 4:
                            printf(" -> Decreasing value...\n");
                            // Decrease something
                            break;
                        default:
                            printf(" -> Unknown command!\n");
                            break;
                    }
                }else{ printf(" -> NOTHING RECEIVED\n");
}
            }
}















void Task_WOR(void *pvParameters)
{
    int  SendValue = 0 ; 

    for(;;)
    {
        SendValue = ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
        Serial.printf("HI From WORKER %d \n ",SendValue);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize

    }
}

void Task_CTR(void *pvParameters)
{
    int  SendValue = 0 ; 

    for(;;)
    {
        for(int i = 0 ; i < 3 ; i ++)
        {
            xTaskNotifyGive(xWorker);
            SendValue = 1 ; 
        }
        
        Serial.println("HI From CTRL");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize
    }
}

void Consumer(void *pvParameters)
{
    int  receivedCommand = 0 ; 

    for(;;)
    {
      if (xQueueReceive(commandQueue, &receivedCommand, portMAX_DELAY) == pdPASS)
       {
            Serial.printf(" RECEIVED %d\n",receivedCommand);

       }
        vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize
    }
}



void LED_TOGGLE(void *pvParameters)
{

    LED_init(2);


    for(;;)
    {
               uxBits = xEventGroupWaitBits(
               xCreatedEventGroup,   /* The event group being tested. */
               BIT_0 | BIT_1| BIT2, /* The bits within the event group to wait for. */
               pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */ // pdFALSE  dont clear
               pdFALSE,       /* Don't wait for both bits, either bit will do. */   //pdFALSE OR , PdTRUE AND
               portMAX_DELAY);/* Wait a maximum of 100ms for either bit to be set. */

                while( (( uxBits & BIT_1 ) != 0))
                {
                    LED_ON(2);
                    Serial.println(" LED ON FAST ");

                    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for serial to stabilize

                    LED_OFF(2);
                    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for serial to stabilize

                }

                while( (( uxBits & BIT_0 ) != 0 ) )
                {
                    LED_ON(2);
                    Serial.println(" LED ON SLOW ");

                    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize

                    LED_OFF(2);
                    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for serial to stabilize

                }

                while( (( uxBits & BIT_2 ) != 0 )&&(( uxBits & BIT_0 ) == 0)&&(( uxBits & BIT_1 ) == 0) )
                {
                    LED_ON(2);
                    Serial.println(" LED ON STEADY ");

                    vTaskDelay(pdMS_TO_TICKS(500));  // Wait for serial to stabilize

                    LED_OFF(2);
                    vTaskDelay(pdMS_TO_TICKS(500));  // Wait for serial to stabilize

                }


        }

    }



void PRINT_MESSAGE(void *pvParameters)
{
    int counter = 0 ;

    for(;;)
    {
        if( xSemaphoreTake( xSemaphore_, ( TickType_t ) 10 ) == pdTRUE )
        {
            counter ++ ;
            Serial.printf(" HI from taskB counter is %d\n ", counter );

        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for serial to stabilize
    }
}






void BUTTON_TASK_B(void *pvParameters)
{
    int reading ; 
    pinMode(32,INPUT_PULLDOWN);

    for(;;)
    {
        reading =  digitalRead(32);

        if (reading == HIGH) {
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_1 );/* The bits being set. */
            Serial.println(" BUTTON B PRESSED");
            Serial.println(" HIGH TEMP");

        }

        vTaskDelay(pdMS_TO_TICKS(200));  // Wait for serial to stabilize

    }

}

void BUTTON_TASK_A(void *pvParameters)
{
    int reading ; 
    pinMode(33,INPUT_PULLUP);

    for(;;)
    {
        reading =  digitalRead(33);

        if (reading == LOW) {
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_0 );/* The bits being set. */
            Serial.println(" BUTTON  A PRESSED");
            Serial.println(" LOW BATTERY");

        }

        vTaskDelay(pdMS_TO_TICKS(200));  // Wait for serial to stabilize

    }

}


void BUTTON_TASK_C(void *pvParameters)
{
    int reading ; 
    pinMode(35,INPUT_PULLDOWN);

    for(;;)
    {
        reading =  digitalRead(35);

        if (reading == HIGH) {
                    uxBits = xEventGroupSetBits(
                                 xCreatedEventGroup,    /* The event group being updated. */
                                 BIT_2 );/* The bits being set. */
            Serial.println(" BUTTON  C PRESSED");
            Serial.println(" USER OVERRIDE");

        }

        vTaskDelay(pdMS_TO_TICKS(200));  // Wait for serial to stabilize

    }

}







void create_tasks(int input)
{

    const char* names[] = {"PRUDCER ONE", "PRUDCER TWO", "Consumer","STORAGE","MANAGER"};
    UBaseType_t priorities[] = {3,2,2,2,2};
    UBaseType_t pramters[]   = {1, 2, 3, 4,5};

    for (int i=0; i < input; i ++ )
    {
        if(i==0)
        {
                    xReturned = xTaskCreate(
                    LED_TOGGLE,
                    names[i], 
                    2000,
                    (void *)pramters[i],    // Parameter = 1
                    priorities[i],
                    &xWorker);  // Use global handle

                    if(xReturned == pdPASS) {
                    Serial.printf("%s created\n", names[i]);
                    }

        }else if (i == 1)
        {
                    xReturned = xTaskCreate(
                    BUTTON_TASK_A,
                    names[i], 
                    2000,
                    (void *)pramters[i],    // Parameter = 1
                    priorities[i],
                    NULL);  // Use global handle

                    if(xReturned == pdPASS) {
                    Serial.printf("%s created\n", names[i]);
                    }

        }
        else if (i == 2)
        {
                    xReturned = xTaskCreate(
                    BUTTON_TASK_B,
                    names[i], 
                    2000,
                    (void *)pramters[i],    // Parameter = 1
                    priorities[i],
                    NULL);  // Use global handle

                    if(xReturned == pdPASS) {
                    Serial.printf("%s created\n", names[i]);
                    }

        }
        else if (i == 3)
        {
                    xReturned = xTaskCreate(
                    BUTTON_TASK_C,
                    names[i], 
                    2000,
                    (void *)pramters[i],    // Parameter = 1
                    priorities[i],
                    NULL);  // Use global handle

                    if(xReturned == pdPASS) {
                    Serial.printf("%s created\n", names[i]);
                    }

        }
        else if (i == 4)
        {
                    xReturned = xTaskCreate(
                    task5_func,
                    names[i], 
                    2000,
                    (void *)pramters[i],    // Parameter = 1
                    priorities[i],
                    NULL);  // Use global handle

                    if(xReturned == pdPASS) {
                    Serial.printf("%s created\n", names[i]);
                    }

        }


    }
}
