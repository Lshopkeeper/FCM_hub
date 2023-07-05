/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-12     LXH       the first version
 */
#ifndef APPLICATIONS_USERAPP_GET_DATA_H_
#define APPLICATIONS_USERAPP_GET_DATA_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fcm_adc.h"
#include "fcm_can.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

// 平均采样次数
#define ADC_SAMPAL_COUNTS                10
// 采样电阻
#define BOARD_FETCH_R_VALUE              1000
// 温度偏移
#define BOARD_FETCH_TEMP_OFFSET          50
// 跟随器电压
#define BOARD_FETCH_BASE_VOLTAGE        2.48
#define MCU_VERF_VOLTAGE                3.27

/*pt1000采集温度，通道1，通道2*/
#define FCM_PT_TEMP_CHANNEL_NUM    FCM_ADC_CHANNEL_NUM-1
/*压力传感器AD采集通道*/
#define FCM_PRES_SENSOR_CHANNEL    2

#define SPEED_GROUP_NUM            2    //速度采样组，风扇速度，电子泵速度

#define WORK_MODE           g_get_ad_para.work_mode      //风扇工作模式
#define OIL_IN_TEMP         g_get_ad_para.temp_inoil     //进油口温度
#define OIL_OUT_TEMP        g_get_ad_para.temp_outiol    //出油口温度

#define MAX_LIMIT(a, b)         (a>b? a:b)                     //取两者最大
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
typedef struct{

    rt_uint8_t temp_inoil;          //进油口温度

    rt_uint8_t temp_outiol;         //出油口温度

    rt_uint32_t oil_press;          //出油口压力

    rt_uint8_t liquid_level;        //液位信息

}G_GET_TEMP_PRESS_PARA;

/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern G_GET_TEMP_PRESS_PARA g_get_ad_para;

extern void get_status_task(void *parameter);
#ifdef __cplusplus
}
#endif
#endif /* APPLICATIONS_USERAPP_GET_DATA_H_ */
