/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-18     LXH       the first version
 */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fault_manage.h"
#include "fcm_pwm.h"
#include "task_tiu.h"
#include "get_data.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
static void period_process_temp_state(void)
{
    rt_uint8_t cur_max_temp = 0;
    TERM_COMM_DATA *tiu_para;

    /*获取当前出油口,进油口最大温度*/
    cur_max_temp = MAX_LIMIT(OIL_IN_TEMP,OIL_OUT_TEMP);

    tiu_para = task_tiu_para_list_get();

    //PT1000实际温度有50偏移
    if((cur_max_temp-50) >= 95)//过温故障
    {
        tiu_para->fcm_remote_signal_fault.sys_overtemp = 0x10;

    }else if((cur_max_temp-50) >= 85){//过温告警

        tiu_para->fcm_remote_signal_fault.sys_overtemp = 0x01;

    }else{ //温度正常

        tiu_para->fcm_remote_signal_fault.sys_overtemp = 0x00;
    }
}

static void period_process_fun_bump_state(void)
{
    TERM_COMM_DATA *tiu_para;

    tiu_para = task_tiu_para_list_get();
	static UINT8 fun_fault_cnt = 0, bump_fault_cnt = 0;

    /*已经输出给风扇PWM*/
	if(tiu_para->fcm_remote_measure.fun_speed != 0){
		if(tiu_para->fcm_remote_measure.fun_pwm_speed == 0){  //实际转速等于0
			if(fun_fault_cnt < 50){
				fun_fault_cnt++;
				tiu_para->fcm_remote_signal_fault.fun_fault = 0;
				rt_thread_mdelay(50);
			}else{
				tiu_para->fcm_remote_signal_fault.fun_fault = 1;
			}
		}else{  //实际转速不等于0
			fun_fault_cnt = 0;
			tiu_para->fcm_remote_signal_fault.fun_fault = 0;
		}
	}else{
		/*未输出给风扇PWM*/
		fun_fault_cnt = 0;
    	tiu_para->fcm_remote_signal_fault.fun_fault = 0;
	}
	
	/*已经输出给电子泵PWM*/
	if(tiu_para->fcm_remote_measure.bump_speed != 0){
		if(tiu_para->fcm_remote_measure.bump_pwm_speed == 0){
			if(bump_fault_cnt < 50){
				bump_fault_cnt++;
				tiu_para->fcm_remote_signal_fault.bump_fault = 0;
				rt_thread_mdelay(50);
			}else{
				tiu_para->fcm_remote_signal_fault.bump_fault = 1;
			}
		}else{
			bump_fault_cnt = 0;
			tiu_para->fcm_remote_signal_fault.bump_fault = 0;
		}
	}else{
		/*未输出给电子泵PWM*/
		bump_fault_cnt = 0;
    	tiu_para->fcm_remote_signal_fault.bump_fault = 0;
	}

}

static void period_process_press_leakage_state(void)
{
	static UINT8 fault_cnt = 0;
    TERM_COMM_DATA *tiu_para;

    tiu_para = task_tiu_para_list_get();
    /*液位信息*/
    tiu_para->fcm_remote_signal_fault.fluid_divulge = g_get_ad_para.liquid_level;
    /*油泵工作时*/
    if(g_pwm_para[PWM_PORT_BUMP].cur_pwm_duty > 0)
    {
    	//20230704 因自研和外购的差距圣工要求液压改完6，最大不超过7！！！
    	//过7会有问题，超出设备承载
        if(g_get_ad_para.oil_press > 60){
            /*压力告警*/
			if(fault_cnt < 25){
				fault_cnt++;
				tiu_para->fcm_remote_signal_fault.press_alarm = 0;
				rt_thread_mdelay(25);
			}else{
				tiu_para->fcm_remote_signal_fault.press_alarm = 0x01;
			}
        }else{
            /*压力正常*/
			fault_cnt = 0;
            tiu_para->fcm_remote_signal_fault.press_alarm = 0;
        }
    }
}

static void period_check_fault_manage(void)
{
	//温度
    period_process_temp_state();
	//风扇和油泵
    period_process_fun_bump_state();
	//液压
    period_process_press_leakage_state();
}

/*故障检查任务*/
void fault_manage_task(void* parameter)
{

    while(TRUE)
    {
        /*周期检查故障*/
        period_check_fault_manage();

        rt_thread_mdelay(50);
    }
}
