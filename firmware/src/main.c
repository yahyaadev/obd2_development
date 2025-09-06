#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ====== Globals ====== */
UART_HandleTypeDef huart2;

/* ====== Prototypes we must define ====== */
void SystemClock_Config(void);
void Error_Handler(void);

/* ====== printf -> USART2 ====== */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}

/* ====== USART2 init (115200, PA2/PA3) ====== */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* GPIO, clocks & AF7 for PA2/PA3 */
void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/* ====== Your JSON loop (~10 Hz) ====== */
void loop_json(void)
{
    static uint32_t seq = 0;
    char json_buf[160];
    while (1)
    {
        uint32_t ms = HAL_GetTick();
        uint32_t rpm = 1400u + (ms / 100u) % 600u;
        float speed_kph = 35.0f + ((ms / 50u) % 200u) / 10.0f;
        int32_t coolant_c = 70 + ((ms / 1000u) % 20u);
        float batt_v = 12.0f + ((ms / 2000u) % 15u) / 10.0f;

        int n = snprintf(json_buf, sizeof(json_buf),
                         "{\"seq\":%lu,\"tx_ms\":%lu,\"rpm\":%lu,\"speed_kph\":%.1f,"
                         "\"coolant_c\":%ld,\"batt_v\":%.1f}\n",
                         (unsigned long)seq, (unsigned long)ms, (unsigned long)rpm,
                         speed_kph, (long)coolant_c, batt_v);

        if (n > 0 && n < (int)sizeof(json_buf))
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)json_buf, (uint16_t)n, HAL_MAX_DELAY);
        }
        seq++;
        HAL_Delay(100); // ~10 Hz
    }
}

/* ====== Minimal clock + error stubs ======
   Keeps default HSI 16 MHz; fine for UART @115200 and HAL_Delay. */
void SystemClock_Config(void)
{
    /* Intentionally minimal: running on reset HSI clock. */
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    { /* hang here so we can see failures */
    }
}

/* ====== main() ====== */
int main(void)
{
    HAL_Init();            // HAL & SysTick
    SystemClock_Config();  // (minimal for now)
    MX_USART2_UART_Init(); // UART2 ready for printf

    // Start streaming JSON lines to ST-LINK VCP (ttyACM* on the Pi)
    loop_json();
}
