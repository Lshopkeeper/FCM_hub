/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-11     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG

#include <rtdbg.h>
#include <rtthread.h>

#include <fcm_common.h>

#include "fcm_can.h"
#include <get_data.h>
#include "watchdog.h"
#include "fcm_inputcapture.h"
#include "fcm_pwm.h"
#include "io_operate.h"
#include <task_tiu.h>
#include "task_fun.h"
#include "fault_manage.h"
//任务栈
//CAN收发任务
ALIGN(RT_ALIGN_SIZE)
static char fcm_can_recv_stack[1024]__attribute__((section (".ccmram"))) = {0};
static struct rt_thread fcm_can_recv __attribute__((section (".ccmram")));
ALIGN(RT_ALIGN_SIZE)
static char fcm_can_send_stack[2048]__attribute__((section (".ccmram"))) = {0};
static struct rt_thread fcm_can_send __attribute__((section (".ccmram")));

//信息采集
ALIGN(RT_ALIGN_SIZE)
static char gather_status_stack[512] = {0};
static struct rt_thread gather_status;

//风扇控制
ALIGN(RT_ALIGN_SIZE)
static char fun_bump_control_stack[512] = {0};
static struct rt_thread fun_bump_control;

//故障管理
ALIGN(RT_ALIGN_SIZE)
static char fault_manage_stack[512] = {0};
static struct rt_thread fault_manage;

ALIGN(RT_ALIGN_SIZE)
static char recv_tiu_stack[1024] = {0};
static struct rt_thread recv_tiu;

ALIGN(RT_ALIGN_SIZE)
static char send_tiu_stack[2048] = {0};
static struct rt_thread send_tiu;

//任务线程列表
T_TASK_LIST gtTaskList[]=
{
        /*TCB地址   -           线程名称  -        线程入口函数       -          函数参数    -   线程栈起始地址  -             线程栈大小  -                   优先级-时间片-动态任务-启动创建        */
        /*CAN收发任务*/
        {&fcm_can_recv,      "can_rx",             rev_tiu_can_task,    (void *)0,    fcm_can_recv_stack,      sizeof(fcm_can_recv_stack),      6,   5,    FALSE,   TRUE},
        {&fcm_can_send,      "Can_tx",             send_tiu_can_task,   (void *)0,    fcm_can_send_stack,      sizeof(fcm_can_send_stack),      6,   5,    FALSE,   TRUE},

        {&gather_status,     "get_temp",           get_status_task,     RT_NULL,      gather_status_stack,     sizeof(gather_status_stack),     9,   10,   FALSE,   TRUE},
        {&fun_bump_control,  "fun_pwm",            task_fun_bump,       RT_NULL,      fun_bump_control_stack,  sizeof(fun_bump_control_stack),  9,   10,   FALSE,   TRUE},
        {&fault_manage,      "fault",              fault_manage_task,   RT_NULL,      fault_manage_stack,      sizeof(fault_manage_stack),      9,   10,   FALSE,   TRUE},

		/*TIU通信相关任务*/
        {&recv_tiu,          "tiu_rx",             recv_tiu_task,       RT_NULL,      recv_tiu_stack,          sizeof(recv_tiu_stack),          10,   10,    FALSE,   TRUE},
        {&send_tiu,          "tiu_tx",             send_tiu_task,       RT_NULL,      send_tiu_stack,          sizeof(send_tiu_stack),          10,   10,    FALSE,   TRUE}
};
//任务/组件初始化
static void init_module(void)
{
    uint8_t dip_addr=0;

    //外设初始化
    gpio_init();
    
    dip_addr=get_io_state(DIGITAL_KIN4);//KIN4为高位
    dip_addr<<=1;
    dip_addr|=get_io_state(DIGITAL_KIN3);//kin3为低位
    
    can_init(dip_addr);
    adc_init();
    pwm_init();
    Init_PwmInput();
    //Init_InputCap();
//    wdt_init();

    init_tiu_task(dip_addr);
}
//系统任务创建
static void create_tasks(void)
{
    for(rt_uint8_t i = 0; 0 != gtTaskList[i].name[0]; i++ )
    {
        if(TRUE == gtTaskList[i].ucStartupCreate)
        {
            if(TRUE == gtTaskList[i].ucDynamicTh)
            {
                gtTaskList[i].thread = rt_thread_create(gtTaskList[i].name, gtTaskList[i].entry,
                    gtTaskList[i].parameter, gtTaskList[i].stack_size,
                    gtTaskList[i].priority, gtTaskList[i].tick);
                if (RT_NULL != gtTaskList[i].thread)
                {
                    rt_thread_startup(gtTaskList[i].thread);
                    LOG_I("Dynamic task %s startup!\n", gtTaskList[i].name);
                }
            }
            else
            {
                rt_thread_init(gtTaskList[i].thread, gtTaskList[i].name, gtTaskList[i].entry,
                    (void *)gtTaskList[i].parameter, (void *)gtTaskList[i].stack_start,
                    gtTaskList[i].stack_size, gtTaskList[i].priority, gtTaskList[i].tick);
                rt_thread_startup(gtTaskList[i].thread);
                LOG_I("Static task %s startup!\n", gtTaskList[i].name);
            }
        }
    }
}

void FcmIdleHook()
{
    rt_tick_t tick = 0;
    static rt_tick_t ulLastLightTick = 0;
    static UINT8 ucLightState = 0;

    SYSTEM_TIME_GET(tick);

    if(500 < TIME_INTERVAL( ulLastLightTick, tick ))
    {
        /* set LED0 pin level to high or low */
        //rt_pin_write(LED_PIN_RUN, (ucLightState++) % 2);
        HAL_GPIO_WritePin(GPIOC, RUN_LED_Pin, (ucLightState++) % 2);
        ulLastLightTick = tick;
    }
}

void RegistIdleHookFunc()
{
    rt_thread_idle_sethook(FcmIdleHook);
}

int main(void)
{
    //RCC->CSR |= (1<<24);
    init_module();
    ADC_Calibration();
//    uint8_t buf[8] = {0};
//    buf[3] = LL_BYTE(RCC->CSR);
//    buf[2] = LH_BYTE(RCC->CSR);
//    buf[1] = HL_BYTE(RCC->CSR);
//    buf[0] = HH_BYTE(RCC->CSR);
//
//    send_msg_by_can(0x99,8,buf);
//    rt_thread_mdelay(15000);

    RegistIdleHookFunc();
    create_tasks();
    while (1)
    {
        //send_msg_by_can(0x99,8,0);
        if(FALSE == stop_feed_dog_flag)
        {
//            feed_dog();
        }
        TOGGLE_LED();
        rt_thread_mdelay(200);
    }

    return RT_EOK;
}
