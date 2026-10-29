#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* CH32V307VCT6 runs at 144 MHz; the RTOS tick uses HCLK / 1000. */
#define configCPU_CLOCK_HZ                    ( 144000000UL )
#define configTICK_RATE_HZ                    ( 1000U )
#define configUSE_PREEMPTION                   1
#define configUSE_TIME_SLICING                 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configMAX_PRIORITIES                   8U
#define configMINIMAL_STACK_SIZE               192U
#define configMAX_TASK_NAME_LEN                16U
#define configTOTAL_HEAP_SIZE                  ( 12U * 1024U )
#define configTICK_TYPE_WIDTH_IN_BITS          TICK_TYPE_WIDTH_32_BITS
#define configSTACK_DEPTH_TYPE                 uint32_t

#define configSUPPORT_DYNAMIC_ALLOCATION       1
#define configSUPPORT_STATIC_ALLOCATION        0
#define configAPPLICATION_ALLOCATED_HEAP       0
#define configUSE_MALLOC_FAILED_HOOK           1
#define configCHECK_FOR_STACK_OVERFLOW         2

#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configUSE_TIMERS                       0
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                   0
#define configUSE_TASK_NOTIFICATIONS           1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES  1U
#define configUSE_EVENT_GROUPS                 1
#define configUSE_STREAM_BUFFERS               1
#define configUSE_CO_ROUTINES                  0
#define configUSE_PORTABLE_LONG_LONG           0
#define configUSE_TRACE_FACILITY               0
#define configGENERATE_RUN_TIME_STATS          0
#define configUSE_APPLICATION_TASK_TAG         0
#define configQUEUE_REGISTRY_SIZE              0U

#define configENABLE_FPU                       1
#define configENABLE_VPU                       0
#define configISR_STACK_SIZE_WORDS             256U

/* QingKe V4 SysTick is configured in freertos_port.c; there is no CLINT MTIME. */
#define configMTIME_BASE_ADDRESS               0UL
#define configMTIMECMP_BASE_ADDRESS            0UL

#define INCLUDE_vTaskPrioritySet               1
#define INCLUDE_uxTaskPriorityGet              1
#define INCLUDE_vTaskDelete                    1
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_vTaskDelay                     1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_uxTaskGetStackHighWaterMark    1
#define INCLUDE_eTaskGetState                  1

#define configASSERT(x)                        \
    do                                          \
    {                                           \
        if ((x) == 0)                           \
        {                                       \
            __asm volatile("csrc mstatus, 8"); \
            for (;;)                            \
            {                                   \
                __asm volatile("nop");         \
            }                                   \
        }                                       \
    } while (0)

#endif /* FREERTOS_CONFIG_H */
