/********************************************************************************
* File Name          : main.c
* Description        : CH32V307VCT6 + SeekFree CH32V307 opensource library
*
* Clock / debug UART / interrupt vector are owned by the SeekFree library:
*   clock : clock_init()  (zf_common_clock.c) -> also sets system_clock
*   debug : debug_init()  (zf_common_debug.c) -> UART3, PB10 TX / PB11 RX, 115200
*   IRQ   : User/zf_isr.c (SeekFree hooks) + User/ch32v30x_it.c (NMI/HardFault)
*
* NOTE: the debug UART is UART3 on PB10/PB11, NOT USART1 on PA9.
********************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"
#include "zf_common_headfile.h"

int main(void)
{
    clock_init(SYSTEM_CLOCK_144M);      /* 144 MHz - see zf_common_clock.h for other values */
    debug_init();                       /* must be kept: sets up the debug UART            */

    printf("\r\nCH32V307VCT6 + FreeRTOS ready, system_clock = %d MHz\r\n",
           system_clock / 1000000);

    if (app_tasks_create() != pdPASS)
    {
        printf("FreeRTOS task creation failed\r\n");
        for (;;)
        {
        }
    }

    vTaskStartScheduler();

    /* The scheduler returns only when it cannot allocate its required objects. */
    printf("FreeRTOS scheduler failed to start\r\n");
    for (;;)
    {
    }
}
