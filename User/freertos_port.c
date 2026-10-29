#include "FreeRTOS.h"
#include "task.h"

#include "ch32v30x.h"

#define FREERTOS_SYSTICK_STE   ( 1UL << 0 )
#define FREERTOS_SYSTICK_STIE  ( 1UL << 1 )
#define FREERTOS_SYSTICK_STCLK ( 1UL << 2 )
#define FREERTOS_SYSTICK_STRE  ( 1UL << 3 )

void freertos_application_irq(uint32_t ulMcause) __attribute__((weak));

void vPortSetupTimerInterrupt(void)
{
    const uint64_t ullTicksPerPeriod =
        (uint64_t)configCPU_CLOCK_HZ / (uint64_t)configTICK_RATE_HZ;

    configASSERT(ullTicksPerPeriod > 0ULL);

    /* Use the QingKe V4 64-bit SysTick at HCLK, with auto-reload enabled. */
    SysTick->CTLR = 0U;
    SysTick->CNT = 0ULL;
    SysTick->CMP = ullTicksPerPeriod;
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->CTLR = FREERTOS_SYSTICK_STE |
                    FREERTOS_SYSTICK_STIE |
                    FREERTOS_SYSTICK_STCLK |
                    FREERTOS_SYSTICK_STRE;
}

void freertos_risc_v_application_interrupt_handler(void)
{
    uint32_t ulMcause;
    uint32_t ulCauseNumber;

    __asm volatile("csrr %0, mcause" : "=r"(ulMcause));
    ulCauseNumber = ulMcause & 0x7FFFFFFFUL;

    if (ulCauseNumber == (uint32_t)SysTicK_IRQn)
    {
        portYIELD_FROM_ISR(xTaskIncrementTick());
        return;
    }

    freertos_application_irq(ulMcause);
}

void freertos_application_irq(uint32_t ulMcause)
{
    (void)ulMcause;
    configASSERT(0U);
}
