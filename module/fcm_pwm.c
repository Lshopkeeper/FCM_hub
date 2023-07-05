/******************************************************************************,

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_pwm.c
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月11日
  最近修改   :
  功能描述   : FCM PWM任务
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月11日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/
#define DBG_TAG "pwm"
#define DBG_LVL LOG_LVL_DBG
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <rtdbg.h>
#include "fcm_pwm.h"
#include "watchdog.h"
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
ALIGN(RT_ALIGN_SIZE)
T_FCM_PWM_PARA g_pwm_para[FCM_PWM_NUM] = {0};
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
static rt_uint8_t fun_pwm_dev_init(void)
{
    int res = 0;

    g_pwm_para[PWM_PORT_FUN].pwm_dev_name = PWM_DEV_NAME_FUN;        //PWM设备：风扇
    g_pwm_para[PWM_PORT_FUN].channel = PWM_DEV_CHANNEL_FUN;          //PWM通道
    g_pwm_para[PWM_PORT_FUN].pwm_period = FUN_PWM_PERIOD;            //PWM周期
    g_pwm_para[PWM_PORT_FUN].pwm_pulse = 0;                          //PWM脉冲宽度
    g_pwm_para[PWM_PORT_FUN].cur_pwm_duty = PWM_DUTY_CYCLE_0;        //PWM占空比速度

    g_pwm_para[PWM_PORT_FUN].pwm_dev = (struct rt_device_pwm *)rt_device_find(FCM_PWM_DEV_NAME(3));
    if(NULL == g_pwm_para[PWM_PORT_FUN].pwm_dev)
    {
        LOG_E("Find device s% is failed!\n",FCM_PWM_DEV_NAME(PWM_DEV_INDEX_FUN));
        return RT_ERROR;
    }
    /*设置 PWM 的周期时间，脉冲宽度时间  占空比为0或者30%*/
    res |= rt_pwm_set(g_pwm_para[PWM_PORT_FUN].pwm_dev, g_pwm_para[PWM_PORT_FUN].channel,g_pwm_para[PWM_PORT_FUN].pwm_period ,g_pwm_para[PWM_PORT_FUN].pwm_pulse);
    /*使能PWM*/
    res |= rt_pwm_enable(g_pwm_para[PWM_PORT_FUN].pwm_dev, g_pwm_para[PWM_PORT_FUN].channel);

    if(res != RT_EOK)
    {
        LOG_E("init fun pwm failed\n");
        return RT_ERROR;
    }
    return RT_EOK;
}
static rt_uint8_t bump_pwm_dev_init(void)
{
    int res = 0;

    g_pwm_para[PWM_PORT_BUMP].pwm_dev_name = PWM_DEV_NAME_BUMP;     //PWM设备：电子泵
    g_pwm_para[PWM_PORT_BUMP].channel = PWM_DEV_CHANNEL_BUMP;       //PWM通道
    g_pwm_para[PWM_PORT_BUMP].pwm_period = BUMP_PWM_PERIOD;         //PWM周期
    g_pwm_para[PWM_PORT_BUMP].pwm_pulse = 0;                        //PWM脉冲宽度
    g_pwm_para[PWM_PORT_BUMP].cur_pwm_duty = PWM_DUTY_CYCLE_0;      //PWM占空比速度

    g_pwm_para[PWM_PORT_BUMP].pwm_dev = (struct rt_device_pwm *)rt_device_find(FCM_PWM_DEV_NAME(2));
    if(NULL == g_pwm_para[PWM_PORT_BUMP].pwm_dev)
    {
        LOG_E("Find device s% is failed!\n",FCM_PWM_DEV_NAME(PWM_DEV_INDEX_ELEPUMP));
        return RT_ERROR;
    }
    /*设置 PWM 的周期时间，脉冲宽度时间  占空比为0或者30%*/
    res |= rt_pwm_set(g_pwm_para[PWM_PORT_BUMP].pwm_dev, g_pwm_para[PWM_PORT_BUMP].channel,g_pwm_para[PWM_PORT_BUMP].pwm_period ,g_pwm_para[PWM_PORT_BUMP].pwm_pulse);
    /*使能PWM*/
    res |= rt_pwm_enable(g_pwm_para[PWM_PORT_BUMP].pwm_dev, g_pwm_para[PWM_PORT_BUMP].channel);

    if(res != RT_EOK)
    {
        LOG_E("init fun pwm failed\n");
        return RT_ERROR;
    }
    return RT_EOK;
}
// FCM CAN口初始化
void pwm_init(void)
{
    if(fun_pwm_dev_init() || bump_pwm_dev_init())
    {
        LOG_E(" init failed!\n");
        return;
    }
}

/*************************************************************************************************************
    * @brief 更改PWM占空比   占空比=脉冲宽度/周期 = Pulse/Period
    * @param pwm_para PWM结构体数据
    * @param DutyCyc 占空比速度
    * @param uxFlag 是否更改周期标志。uxFlag=1：同时更改周期和脉冲宽度, uxFlag=0：只更改脉冲宽度（占空比）。
    * @param xExtInfo 周期
    * @return RT_EOK 成功；RT_ERROR 失败
    **********************************************************************************************************/
static rt_uint8_t change_pwm_duty_and_cycle(T_FCM_PWM_PARA *pwm_para,rt_uint8_t duty,rt_uint8_t flag,rt_uint32_t ext_info)
{
    int res;
    rt_uint32_t pulse = 0;
    rt_uint32_t period = 0;
    if(pwm_para == NULL)
    {
        return RT_ERROR;
    }
    if(flag == 1)//同时修改脉冲宽度和周期
    {
        pwm_para->pwm_period = ext_info;
    }
    period = pwm_para->pwm_period;
    pulse = period*duty/100; //脉冲宽度
    /*设置新的PWM周期和脉冲宽度*/
    res = rt_pwm_set(pwm_para->pwm_dev, pwm_para->channel, period, pulse);
    if(res != RT_EOK)
    {
        LOG_E("set pwm failed\n");
        return RT_ERROR;
    }
    pwm_para->cur_pwm_duty = duty; //当前PWM占空比速度
    return RT_EOK;
}

/*************************************************************************************************************
    * @brief 调节PWM速度
    * @param pwm_para PWM结构体数据
    * @param uxSpeed 占空比速度
    * @return RT_EOK 成功；RT_ERROR 失败
    **********************************************************************************************************/
rt_uint8_t set_pwm_speed(T_FCM_PWM_PARA *pwm_para,rt_uint8_t speed)
{
    rt_uint8_t i,index,duty_cyc;
    rt_uint8_t flag = 0;
    rt_uint8_t result = RT_ERROR;


    if(speed>pwm_para->cur_pwm_duty)  //电子泵PWM增速爬坡使用
    {
        index = (speed-pwm_para->cur_pwm_duty)/5;  //原来10
        flag = 1;
    }
    switch(pwm_para->pwm_dev_name)
    {
        case PWM_DEV_NAME_FUN://风扇 PWM 调速
        {
            result = change_pwm_duty_and_cycle(pwm_para,speed,0,0);
            break;
        }
        case PWM_DEV_NAME_BUMP://电子泵 PWM 增速需要爬坡
        {
            if(flag == 1) //PWM增速  需要爬坡
            {
                for(i=0;i<index;i++)
                {
                    duty_cyc = pwm_para->cur_pwm_duty+5; //PWM_DUTY_CYCLE_10;    //每次调速只增加10%，且需等待2S
                    result = change_pwm_duty_and_cycle(pwm_para,duty_cyc,0,0);
                    rt_thread_delay(2000);
                }
                flag = 0;
            }else {         //PWM减速 无需考虑爬坡
                result = change_pwm_duty_and_cycle(pwm_para,speed,0,0);
            }
            break;
        }
        default:
        break;
    }
    return result;
}
