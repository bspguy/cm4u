#include "stm32f4xx.h"   /* or your MCU's device header */
#include "core_cm4.h"
#include "cm4u_core.h"

/*
 * Minimal example: blink (pseudo) using DWT-based delay
 * and show how to check Thread/Handler mode.
 */

static void delay_us_blocking(uint32_t us, uint32_t core_hz)
{
    cm4u_delay_us(us, core_hz);
}

int main(void)
{
    const uint32_t core_hz = 168000000u; /* adjust to your clock */

    /* Initialize DWT cycle counter */
    (void)cm4u_dwt_init();

    /* Example: check start mode */
    if (cm4u_in_thread_mode()) {
        /* We are in Thread mode here (normal) */
    }

    while (1) {
        uint32_t primask = cm4u_critical_enter();
        /* critical section work here */
        cm4u_critical_exit(primask);

        /* Do some fake "work" and profile it */
        uint32_t start = cm4u_profile_cycles_start();
        __NOP();
        uint32_t spent = cm4u_profile_cycles_end(start);
        (void)spent; /* set breakpoint to watch spent cycles */

        /* Short blocking delay */
        delay_us_blocking(10u, core_hz);
    }
}
