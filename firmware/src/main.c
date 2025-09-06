#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

// Optional: if you ever use HAL_Delay, SystemClock_Config should be set.
// For a "do nothing" firmware, HAL_Init() is safe and enough.

int main(void)
{
    HAL_Init(); // set up SysTick and HAL state
    // (No clock change, no peripherals — ultra-minimal)

    while (1)
    {
        // idle loop — we’ll add JSON UART in the next step
    }
}
