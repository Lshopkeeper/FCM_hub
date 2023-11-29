/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-12     LXH       the first version
 */
#define DBG_TAG "task_fun_bump"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <rtthread.h>
#include "task_fun.h"
#include <task_tiu.h>
#include <get_data.h>
#include "fcm_pwm.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
#define THE_1_GEAR_TEMP           g_clod_dev_para.clod_dgear_temp[0]    //档位一最大温度
#define THE_2_GEAR_TEMP           g_clod_dev_para.clod_dgear_temp[1]    //档位二最大温度
#define THE_3_GEAR_TEMP           g_clod_dev_para.clod_dgear_temp[2]    //档位三最大温度
#define THE_4_GEAR_TEMP           g_clod_dev_para.clod_dgear_temp[3]    //档位四最大温度

#define FUN_PWM_SPEED_1           g_clod_dev_para.fun_pwm_speed[0]      //档位一风扇PWM转速
#define FUN_PWM_SPEED_2           g_clod_dev_para.fun_pwm_speed[1]      //档位二风扇PWM转速
#define FUN_PWM_SPEED_3           g_clod_dev_para.fun_pwm_speed[2]      //档位三风扇PWM转速
#define FUN_PWM_SPEED_4           g_clod_dev_para.fun_pwm_speed[3]      //档位四风扇PWM转速

#define BUMP_PWM_SPEED_1          g_clod_dev_para.bump_pwm_speed[0]     //档位一油泵PWM转速
#define BUMP_PWM_SPEED_2          g_clod_dev_para.bump_pwm_speed[1]     //档位二油泵PWM转速
#define BUMP_PWM_SPEED_3          g_clod_dev_para.bump_pwm_speed[2]     //档位三油泵PWM转速
#define BUMP_PWM_SPEED_4          g_clod_dev_para.bump_pwm_speed[3]     //档位四油泵PWM转速


#define FAN_DELAY_TIME            (10*60*1000)     //风扇、油泵延时时间-10分钟
#define FAN_DELAY_LOW_TEMP        50               //约定停止充电后，温度需要降低到多少度的以下停机，目前定50

/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
#pragma pack(1)
typedef struct{

    rt_uint8_t fan_speed;  /*0~100*/
    rt_uint8_t fan_state;  /*0:空闲；1:工作*/

}DEV_FAN_PARA;

typedef struct{

    rt_uint8_t bump_speed;  /*0~100*/
    rt_uint8_t bump_state;  /*0:空闲；1:工作*/

}DEV_BUMP_PARA;

typedef struct{

    rt_uint8_t  flag;        /*是否启用延时*/
    rt_uint32_t start_time;  /*开始时间戳*/
    rt_uint32_t end_time;    /*开始时间戳*/
    rt_uint32_t run_time;    /*持续时间*/

}DEV_FAN_DELAY;

typedef struct{
    /*各个档位最大温度*/
    rt_uint8_t clod_dgear_temp[CLOD_GEAR_TEMP_NUM];
    /*各档位对应风扇PWM转速*/
    rt_uint8_t fun_pwm_speed[CLOD_GEAR_TEMP_NUM];
    /*各档位对应油泵PWM转速*/
    rt_uint8_t bump_pwm_speed[CLOD_GEAR_TEMP_NUM];

    /*风扇信息*/
    DEV_FAN_PARA fun_para;
    /*油泵信息*/
    DEV_BUMP_PARA bump_para;

    /*充电结束需要延时工作一段时间*/
    DEV_FAN_DELAY dev_fan_delay;

}DEV_COLD_PARA;
#pragma pack()

static DEV_COLD_PARA g_clod_dev_para = {0};
/*----------------------------------------------*
 * 内部变量原型说明                             *
 *----------------------------------------------*/
static void update_temp_speed_info(void)
{
    rt_uint8_t i;

    TERM_COMM_DATA *tiu_para;
    tiu_para = task_tiu_para_list_get();

    for(i=0;i<CLOD_GEAR_TEMP_NUM;i++)
    {   /*更新温度*/
        g_clod_dev_para.clod_dgear_temp[i] = tiu_para->tiu_temp_para_set.max_temp[i];
        /*更新风扇pwm*/
        g_clod_dev_para.fun_pwm_speed[i] = tiu_para->tiu_temp_para_set.fun_pwm[i];
        /*更新油泵pwm*/
        g_clod_dev_para.bump_pwm_speed[i] = tiu_para->tiu_temp_para_set.bump_pwm[i];
    }
}
/*判断液冷系统启动条件*/
static bool cold_control_process(void)
{
    rt_uint8_t tiu_state;

    TERM_COMM_DATA *tiu_para;
    tiu_para = task_tiu_para_list_get();
    /*获取当前充电机充电状态*/
    tiu_state = tiu_para->tiu_signal_status.gun_status;

    /*只有当充电机处于绝缘检测和充电完成之间时，才开启液冷*/
    if((tiu_state>TIU_STATE_CONNECT)&&(tiu_state<TIU_STATE_COMPLETE))
    {
        /*更新温度档位和速度信息*/
        update_temp_speed_info();
        g_clod_dev_para.fun_para.fan_state = 1;
        g_clod_dev_para.bump_para.bump_state = 1;

        /*风扇延时停止标识*/
        g_clod_dev_para.dev_fan_delay.flag=1;
        /*风扇延时启动 开始时间*/
        /*SYSTEM_TIME_GET(g_clod_dev_para.dev_fan_delay.start_time);*/

        return TRUE;
    }else{

        if(g_clod_dev_para.dev_fan_delay.flag == 1)
        {
            /*SYSTEM_TIME_GET(g_clod_dev_para.dev_fan_delay.end_time);
            g_clod_dev_para.dev_fan_delay.run_time = g_clod_dev_para.dev_fan_delay.end_time - g_clod_dev_para.dev_fan_delay.start_time;*/
            if((tiu_para->tiu_signal_status.gun_temp-50 < FAN_DELAY_LOW_TEMP)/* || (g_clod_dev_para.dev_fan_delay.run_time > FAN_DELAY_TIME) */ )
            {
                /*风扇 、油泵 停*/
                g_clod_dev_para.fun_para.fan_state = 0;
                g_clod_dev_para.bump_para.bump_state = 0;

                /*清空 风扇延时停止标识*/
                g_clod_dev_para.dev_fan_delay.flag = 0;

                return false;
            }

            return true;
        }
        else
        {
            g_clod_dev_para.fun_para.fan_state = 0;
            g_clod_dev_para.bump_para.bump_state = 0;
        }

    }

    return FALSE;
}
/*根据温度获取风扇速度和油泵速度*/
static void cold_speed_manage(rt_uint8_t* fun_speed,rt_uint8_t* pump_speed )
{
    rt_int8_t cur_max_temp,final_temp;
    static rt_uint8_t last_max_temp = 0;

    TERM_COMM_DATA *tiu_para;
    tiu_para = task_tiu_para_list_get();

    /*选取进油口温度，和出油口温度中最大值*/
    //cur_max_temp = MAX_LIMIT(OIL_IN_TEMP,OIL_OUT_TEMP);
    cur_max_temp = tiu_para->tiu_signal_status.gun_temp;
    if((cur_max_temp>=220)||(cur_max_temp == 0))
    {
        return;
    }
    /*温度有50度偏移*/
    cur_max_temp-=50;

    if((cur_max_temp!=last_max_temp)&&((cur_max_temp+3<=last_max_temp)||(cur_max_temp-3>=last_max_temp))) //避免温度在零界点变换导致速度不断切换
    {                                                                                                     //降温时，只有当降温幅度大于3°时，才变换速度
        final_temp = cur_max_temp;
        last_max_temp = cur_max_temp;

    }else{
        final_temp = last_max_temp;
        final_temp = last_max_temp;

    }
    /*根据温度获取PWM速度*/
    if(final_temp <= THE_1_GEAR_TEMP)
    {
        *fun_speed = FUN_PWM_SPEED_1;
        *pump_speed = BUMP_PWM_SPEED_1;
    }else if(final_temp <= THE_2_GEAR_TEMP){

        *fun_speed = FUN_PWM_SPEED_2;
        *pump_speed = BUMP_PWM_SPEED_2;

    }else if(final_temp <= THE_3_GEAR_TEMP){

        *fun_speed = FUN_PWM_SPEED_3;
        *pump_speed = BUMP_PWM_SPEED_3;

    }else{

        *fun_speed = FUN_PWM_SPEED_4;
        *pump_speed = BUMP_PWM_SPEED_4;
    }
    g_clod_dev_para.fun_para.fan_speed = *fun_speed;
    g_clod_dev_para.bump_para.bump_speed = *pump_speed;
}

void task_fun_bump(void *parameter)
{
    rt_uint8_t fun_speed,pump_speed;

    while(TRUE)
    {
        /*判断是否达到开启液冷条件*/
        if(cold_control_process())
        {
            cold_speed_manage(&fun_speed,&pump_speed);
            /*设置风扇PWM速度*/
            set_pwm_speed(FUN_PWM,fun_speed);
            /*设置油泵PWM速度*/
            set_pwm_speed(BUMP_PWM,pump_speed);

        }
		else
		{
            /*设置风扇PWM速度*/
            set_pwm_speed(FUN_PWM,0);
            /*设置油泵PWM速度*/
            set_pwm_speed(BUMP_PWM,0);
        }
        rt_thread_mdelay(200);
    }
}
rt_uint8_t update_clod_info(CLOD_PARA_TYPE type)
{
    if(FAN_SPEED == type){
        return g_clod_dev_para.fun_para.fan_speed;
    }else if(FAN_STATE == type){
        return g_clod_dev_para.fun_para.fan_state;
    }else if(BUMP_SPEED == type){
        return g_clod_dev_para.bump_para.bump_speed;
    }else if(BUMP_STATE == type){
        return g_clod_dev_para.bump_para.bump_state;
    }else{
        return 0;
    }
}
