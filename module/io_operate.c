/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-17     LXH       the first version
 */
#define DBG_TAG "io_operate"
#define DBG_LVL LOG_LVL_DBG
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <rtdbg.h>
#include "io_operate.h"

/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
 void gpio_init(void)
{
 /*PB3是特殊引脚，先关闭JTAG功能，以作为普通IO口使用·*/
   __HAL_RCC_AFIO_CLK_ENABLE();
   __HAL_AFIO_REMAP_SWJ_NOJTAG();      //关闭JTAG
   DBGMCU->CR  &= ~((uint32_t)1<<5);   //关闭异步跟踪，否则PB3将一直读出0

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RUN_LED_Pin|ERROR_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : RUN_LED_Pin ERROR_LED_Pin */
  GPIO_InitStruct.Pin = RUN_LED_Pin|ERROR_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : KIN1_Pin */
  GPIO_InitStruct.Pin = KIN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KIN1_GPIO_Port, &GPIO_InitStruct);
}
 rt_uint8_t get_io_state(rt_uint8_t index)
 {
     rt_uint8_t state = 0;

     /* 合法判断 */
     if (index >= SYS_DI_COUNT)
     {
         return FALSE;
     }

     switch ( index )
     {
         case DIGITAL_KIN1:
             state = HAL_GPIO_ReadPin(KIN1_GPIO_Port, KIN1_Pin);  //state==1：KIN状态为断开,液位正常；state==0：KIN状态为短接,液位过低故障
             state = state ? FALSE : TRUE;
             break;
          case DIGITAL_KIN2:
             state = HAL_GPIO_ReadPin(KIN2_GPIO_Port, KIN2_Pin);  //state==1：KIN状态为断开,；state==0：KIN状态为短接,
             break;
          case DIGITAL_KIN3:
             state = HAL_GPIO_ReadPin(KIN3_GPIO_Port, KIN3_Pin);  //state==1：KIN状态为断开,；state==0：KIN状态为短接,
             break;
          case DIGITAL_KIN4:
             state = HAL_GPIO_ReadPin(KIN4_GPIO_Port, KIN4_Pin);  //state==1：KIN状态为断开,；state==0：KIN状态为短接,
             break;
     }
     //return (state ? FALSE : TRUE);
     return state;
}
