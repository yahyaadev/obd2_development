#include "stm32f4xx_hal.h"
#include <string.h>

UART_HandleTypeDef huart2;

/* ===== SysTick: needed for HAL_Delay ===== */
void SysTick_Handler(void) { HAL_IncTick(); }

/* ===== USART2 init (PA2/PA3, AF7) ===== */
static void MX_USART2_UART_Init(void) {
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_2 | GPIO_PIN_3; // PA2 TX, PA3 RX
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  g.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &g);

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
}

/* ==== Minimal clock (HSI 16 MHz is fine for UART) ==== */
void SystemClock_Config(void) { /* keep default HSI */ }
void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_USART2_UART_Init();

  // LD2 (PA5) for visible heartbeat
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef ld = {0};
  ld.Pin = GPIO_PIN_5;
  ld.Mode = GPIO_MODE_OUTPUT_PP;
  ld.Pull = GPIO_NOPULL;
  ld.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &ld);

  const char *boot = "BOOT\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t *)boot, (uint16_t)strlen(boot), HAL_MAX_DELAY);

  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  HAL_Delay(200);

  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  HAL_Delay(200);

  while (1) {
    static uint32_t seq = 0;
    uint32_t t = HAL_GetTick();
    int rpm = 1650;
    float speed_kph = 42.1f;
    int coolant_c = 78;
    float batt_v = 12.1f;

    printf("{\"seq\":%lu,\"tx_ms\":%lu,\"rpm\":%d,\"speed_kph\":%.1f,\"coolant_c\":%d,\"batt_v\":"
           "%.1f}\r\n",
           (unsigned long)seq++, (unsigned long)t, rpm, speed_kph, coolant_c, batt_v);

    HAL_Delay(100); // ~10 Hz
  }
}
