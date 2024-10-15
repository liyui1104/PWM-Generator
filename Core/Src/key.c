#include "key.h"

/**
 * @brief EXTI interrupt callback handler.
 *        This function is triggered when an external interrupt occurs on EXTI lines.
 *        It increments the external variable Frequency if line 1 triggers the interrupt,
 *        or increments DutyCycle if any of the EXTI lines [15:10] trigger the interrupt.
 * @param GPIO_Pin Specifies the pin connected to the EXTI line that triggered the interrupt.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // Check if the interrupt was triggered by EXTI line 1
  if (GPIO_Pin == KEY0_Pin)
  {
    // Adjust Frequency by 50Hz increments, loop back at 50Hz
    Frequency += 50;
    if (Frequency > 200)
    {
      Frequency = 50;
    }

    // Reconfigure TIM2 frequency
    __HAL_TIM_SET_PRESCALER(&htim2, (72000000 / (Frequency * 2000)) - 1);

    // Need restart the TIM channel to make sure the change take effect
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  }
  // Check if the interrupt was triggered by any of the EXTI lines [10:15]
  else if (GPIO_Pin == KEY1_Pin)
  {
    // Adjust DutyCycle by 20% increments, loop back at 0%
    DutyCycle += 1;
    if (DutyCycle > 100)
    {
      DutyCycle = 0;
    }

    // Reconfigure TIM2 duty cycle
    // CRR (pulse) should in [0,1999]
    uint32_t pulse = (htim2.Init.Period + 1) * DutyCycle / 100 - 1;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pulse);
  }
}
