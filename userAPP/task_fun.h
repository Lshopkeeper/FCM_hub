/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-12     LXH       the first version
 */
#ifndef APPLICATIONS_USERAPP_TASK_FUN_H_
#define APPLICATIONS_MODULE_TASK_FUN_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
//#include "stdbool.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
#define CLOD_GEAR_TEMP_NUM           4
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/
typedef enum{
    FAN_SPEED = 0,
    FAN_STATE,
    BUMP_SPEED,
    BUMP_STATE
}CLOD_PARA_TYPE;
/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/

extern void task_fun_bump(void *parameter);
extern rt_uint8_t update_clod_info(CLOD_PARA_TYPE type);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_USERAPP_TASK_FUN_H_ */
