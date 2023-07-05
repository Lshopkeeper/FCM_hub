/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-11     LXH       the first version
 */
#ifndef APPLICATIONS_FCM_PWM_H_
#define APPLICATIONS_FCM_PWM_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "rtdevice.h"
#include "fcm_common.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
//FCM PWM数量
#define FCM_PWM_NUM                 2
/*风扇 PWM通道 */
#define PWM_DEV_CHANNEL_FUN         1
/*电子泵 PWM通道 */
#define PWM_DEV_CHANNEL_BUMP        1

#define PWM_PORT_FUN                1
#define PWM_PORT_BUMP               0
//PWM 周期时间(单位：纳秒)
#define PWM_PERIOD                  500000

#define FUN_PWM_PERIOD              500000//180000
#define BUMP_PWM_PERIOD             500000

#define __FCM_PWM_DEV_NAME(i)       #i
#define _FCM_PWM_DEV_NAME(i)        __FCM_PWM_DEV_NAME(pwm##i)
#define FCM_PWM_DEV_NAME(i)         _FCM_PWM_DEV_NAME(i)

#define BUMP_PWM      &g_pwm_para[PWM_DEV_NAME_BUMP]        //电子泵
#define FUN_PWM       &g_pwm_para[PWM_DEV_NAME_FUN]         //风扇

#define PWM_CLIMB_TIME     2000        //
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/
/*PWM占空比速度定义*/
typedef enum
{
    PWM_DUTY_CYCLE_0 = 0,         //占空比 0
    PWM_DUTY_CYCLE_10 = 10,       //占空比 10
    PWM_DUTY_CYCLE_20 = 20,       //占空比 20
    PWM_DUTY_CYCLE_30 = 30,       //占空比 30
    PWM_DUTY_CYCLE_40 = 40,       //占空比 40
    PWM_DUTY_CYCLE_50 = 50,       //占空比 50
    PWM_DUTY_CYCLE_60 = 60,       //占空比 60
    PWM_DUTY_CYCLE_70 = 70,       //占空比 70
    PWM_DUTY_CYCLE_80 = 80,       //占空比 80
    PWM_DUTY_CYCLE_90 = 90,       //占空比 90
    PWM_DUTY_CYCLE_100 = 100      //占空比 100
}G_PWM_DUTY_CYCLE;

/*PWM设备*/
typedef enum
{
    PWM_DEV_NAME_BUMP = 0,   //电子泵
    PWM_DEV_NAME_FUN            //风扇
}G_PWM_DEV_NAME;
/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
typedef struct
{
    struct rt_device_pwm *pwm_dev;      //PWM设备句柄
    int channel;                        //PWM设备通道

    rt_uint8_t pwm_dev_name;            //PWM设备  风扇or电子泵
    rt_uint8_t cur_pwm_duty;            //当前占空比速度

    rt_uint32_t pwm_period;             //PWM周期时间(单位纳秒ns)
    rt_uint32_t pwm_pulse;              //PWM脉冲宽度时间,(单位纳秒ns)

}T_FCM_PWM_PARA;
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
/*FCM PWM初始化*/
extern void pwm_init(void);
/*调节PWM速度占空比*/
extern rt_uint8_t set_pwm_speed(T_FCM_PWM_PARA *pwm_para,rt_uint8_t speed);
extern T_FCM_PWM_PARA g_pwm_para[FCM_PWM_NUM];
#endif /* APPLICATIONS_FCM_PWM_H_ */
