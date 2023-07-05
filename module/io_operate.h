/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-17     LXH       the first version
 */
#ifndef APPLICATIONS_MODULE_IO_OPERATE_H_
#define APPLICATIONS_MODULE_IO_OPERATE_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "fcm_common.h"
#include "fcm_can.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
/* 运行灯 ---------------------------------------------*/
#define RUN_LED_Pin GPIO_PIN_13
#define RUN_LED_GPIO_Port GPIOC
#define TOGGLE_LED()      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)
/* 错误灯 ---------------------------------------------*/
#define ERROR_LED_Pin GPIO_PIN_14
#define ERROR_LED_GPIO_Port GPIOC

/* 开入 ---------------------------------------------*/
#define KIN1_Pin GPIO_PIN_3
#define KIN1_GPIO_Port GPIOB

#define KIN2_Pin GPIO_PIN_2
#define KIN2_GPIO_Port GPIOD

#define KIN3_Pin GPIO_PIN_12
#define KIN3_GPIO_Port GPIOC

#define KIN4_Pin GPIO_PIN_11
#define KIN4_GPIO_Port GPIOC

/* 输入捕获 ---------------------------------------------*/
#define FUN_INPUT_PIN   GET_PIN(B, 10)
#define ELE_INPUT_PIN   GET_PIN(B, 11)
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/
/* 开入序号定义 */
typedef enum
{
    DIGITAL_KIN1 = 0,       // DI1，液位传感器
    DIGITAL_KIN2,           // DI2，
    DIGITAL_KIN3,           // DI3，
    DIGITAL_KIN4,           // DI4，
    SYS_DI_COUNT,           //开入量总个数
}DIGITAL_IN_INDEX;
/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern void gpio_init(void);
extern rt_uint8_t get_io_state(rt_uint8_t ucIndex);

#ifdef __cplusplus
}
#endif
#endif /* APPLICATIONS_MODULE_IO_OPERATE_H_ */
