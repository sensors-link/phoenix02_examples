#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Kernel includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "lib_include.h"

#include <stdio.h>
#include <stdbool.h>

#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainQUEUE_SEND_FREQUENCY_MS pdMS_TO_TICKS(1000)
#define mainQUEUE_LENGTH (1)

static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);
static QueueHandle_t xQueue = NULL;

/*-----------------------------------------------------------*/

static void prvQueueSendTask(void *pvParameters) {
  TickType_t xNextWakeTime;
  const unsigned long ulValueToSend = 100UL;
  BaseType_t xReturned;

  /* Remove compiler warning about unused parameter. */
  (void)pvParameters;

  /* Initialise xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    /* Place this task in the blocked state until it is time to run again. */
    vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);

    /* Send to the queue - causing the queue receive task to unblock and
            toggle the LED.  0 is used as the block time so the sending
       operation will not block - it shouldn't need to block as the queue should
       always be empty at this point in the code. */
    xReturned = xQueueSend(xQueue, &ulValueToSend, 0U);
    configASSERT(xReturned == pdPASS);
  }
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask(void *pvParameters) {
  unsigned long ulReceivedValue;
  const unsigned long ulExpectedValue = 100UL;
  const char *const pcPassMessage = "Blink\r\n";
  const char *const pcFailMessage = "Unexpected value received\r\n";
  /* Remove compiler warning about unused parameter. */
  (void)pvParameters;

  for (;;) {
    /* Wait until something arrives in the queue - this task will block
            indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
            FreeRTOSConfig.h. */
    xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

    /*  To get here something must have been received from the queue, but
            is it the expected value?  If it is, toggle the LED. */
    if (ulReceivedValue == ulExpectedValue) {
      _DBG_(pcPassMessage);
    } else {
      _DBG_(pcFailMessage);
    }
  }
}


int main(void)
{

	debug_frmwrk_init();
	_DBG_("Hello world");
    /* Create the queue. */
  xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

  xTaskCreate(
      prvQueueReceiveTask, /* The function that implements the task. */
      "Rx", /* The text name assigned to the task - for debug only as it is not
               used by the kernel. */
      configMINIMAL_STACK_SIZE *
          2U, /* The size of the stack to allocate to the task. */
      NULL,   /* The parameter passed to the task - not used in this case. */
      mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
      NULL); /* The task handle is not required, so NULL is passed. */

  xTaskCreate(prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE * 2U, NULL,
              mainQUEUE_SEND_TASK_PRIORITY, NULL);

  /* Start the tasks and timer running. */
  vTaskStartScheduler();

  for (;;)
    ;
}
