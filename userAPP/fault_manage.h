/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-18     LXH       the first version
 */
#ifndef APPLICATIONS_USERAPP_FAULT_MANAGE_H_
#define APPLICATIONS_USERAPP_FAULT_MANAGE_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "rtdevice.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "fcm_common.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern void fault_manage_task(void* parameter);
#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_USERAPP_FAULT_MANAGE_H_ */
