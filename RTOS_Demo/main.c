#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "lib_include.h"
/*-----------------------------------------------------------*/
#define TMR2_QUEUE_SIZE 2
QueueHandle_t xTmr2Queue;
typedef struct tmrTimerQueueMessage
{
    BaseType_t xMessageID; /*<< The command being sent to the timer service task. */
    BaseType_t xInfo; /*<< The command being sent to the timer service task. */

} TaskMessage_t;


/**
 * @brief 使用vTaskDelay的任务
 *
 * @param pvParameters
 */
static void prvTask1(void *pvParameters) {
    (void) pvParameters;
    for (;;) {
        printf("hello from task 1\n");
        vTaskDelay(configTICK_RATE_HZ);
    }
}

/**
 * @brief 使用xQueue的任务
 *
 * @param pvParameters
 */
static void prvTask2(void *pvParameters) {
    (void) pvParameters;
    TaskMessage_t xMessage;
    for( ;; )
	{
		if( xQueueReceive( xTmr2Queue, &xMessage, portMAX_DELAY) == pdPASS )
		{
			printf("Task2 got <ID:%d,INFO:%d> from queue\n", (int) xMessage.xMessageID, (int )xMessage.xInfo);
		}
	}
}

int main(void)
{
    UART_DeInit(UART1);
    UART_Init(UART1, UART1_PORT_P16_P17, UART_MODE_10B_ASYNC, 19200);

    TIM_TimerInit(TIM2,TIM_TM_AUTO_LOAD, 1000);
    PLIC_EnableIRQ(TIMER2_IRQn);
    PLIC_SetPriority(TIMER2_IRQn,1);
    TIM_ClrIntFlag(TIM2);
    TIM_EnableIRQ(TIM2);

    printf("START...\n");
    xTmr2Queue = xQueueCreate( (UBaseType_t) TMR2_QUEUE_SIZE, ( UBaseType_t ) sizeof( TaskMessage_t ) );
    xTaskCreate( prvTask1, "task1",  128, NULL,  tskIDLE_PRIORITY + 1, NULL );
    xTaskCreate( prvTask2, "task2",  128, NULL,  tskIDLE_PRIORITY + 1, NULL );

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    while(1) {
        // Shall never goes here!
    }
}



void TIMER2_IrqHandler(void)
{
    static int nTmr2Count = 0;
    TaskMessage_t xMessage;
    TIM_ClrIntFlag(TIM2);
    nTmr2Count ++;
    if ( nTmr2Count % 1000 == 0 )
    {
        xMessage.xMessageID = 1;
        xMessage.xInfo = nTmr2Count;
        if ( xQueueIsQueueFullFromISR(xTmr2Queue) != pdTRUE)
        {
            xQueueSendFromISR( xTmr2Queue, &xMessage, NULL );
        }
    }
}
