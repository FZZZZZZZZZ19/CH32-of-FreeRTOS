#include "app_tasks.h"

#include "task.h"
#include "zf_common_headfile.h"

#define APP_WORKER_STACK_WORDS  ( 256U )
#define APP_MONITOR_STACK_WORDS ( 512U )

static volatile uint32_t s_ulWorkCount;

static void prvWorkerTask(void *pvParameters);
static void prvMonitorTask(void *pvParameters);

BaseType_t app_tasks_create(void)
{
    TaskHandle_t xWorkerTask = NULL;

    if (xTaskCreate(prvWorkerTask,
                    "worker",
                    APP_WORKER_STACK_WORDS,
                    NULL,
                    1U,
                    &xWorkerTask) != pdPASS)
    {
        return pdFAIL;
    }

    if (xTaskCreate(prvMonitorTask,
                    "monitor",
                    APP_MONITOR_STACK_WORDS,
                    NULL,
                    2U,
                    NULL) != pdPASS)
    {
        vTaskDelete(xWorkerTask);
        return pdFAIL;
    }

    return pdPASS;
}

static void prvWorkerTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        ++s_ulWorkCount;
        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}

static void prvMonitorTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)pvParameters;

    for (;;)
    {
        printf("FreeRTOS running; worker count = %lu\r\n",
               (unsigned long)s_ulWorkCount);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000U));
    }
}
