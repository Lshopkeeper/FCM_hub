/******************************************************************************

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_inputcapture.c
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月14日
  最近修改   :
  功能描述   : 定时器输入捕获
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月14日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/
#define DBG_TAG "fcm_inputcapture"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fcm_inputcapture.h"
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
static rt_timer_t timer1;

rt_uint16_t PwmCnt1 = 0;
rt_uint16_t PwmCnt2 = 0;
rt_uint16_t fun_pwm_value = 0;
rt_uint16_t ele_pwm_value = 0;

TIM_HandleTypeDef htim2;
T_TIM_INPUTCAP_INFO gtInputCapInfo[INPUT_CAPTURE_NUM] = {0};
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
/*软件定时器 1s触发一次*/
static void timeout1(void *parameter)
{
    fun_pwm_value = PwmCnt1;
    PwmCnt1 = 0;
    ele_pwm_value = PwmCnt2;
    PwmCnt2 = 0;
}

static void Init_SoftTimer(void)
{
    /*创建软件定时器1 周期定时器,1s超时一次*/
    timer1 = rt_timer_create("timer",
                             timeout1,
                             RT_NULL,
                             1000,
                             RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);
    /*启动定时器1*/
    if(timer1 != RT_NULL)rt_timer_start(timer1);
}
/*风扇PWM输入 上升沿中断回调*/
void FunInputCallBack(void *args)
{
    PwmCnt1++;
}
/*油泵PWM输入 上升沿中断回调*/
void BumpInputCallBack(void *args)
{
    PwmCnt2++;
}
static void Init_InputPin(void)
{
   /* 风扇输入捕获引脚为输入模式 */
   rt_pin_mode(FUN_INPUT_PIN, PIN_MODE_INPUT_PULLDOWN);
   /* 绑定中断，上升沿模式，回调函数名为InputCallBack */
   rt_pin_attach_irq(FUN_INPUT_PIN, PIN_IRQ_MODE_RISING, FunInputCallBack, RT_NULL);
   /* 使能中断 */
   rt_pin_irq_enable(FUN_INPUT_PIN, PIN_IRQ_ENABLE);

   /* 电子泵输入捕获引脚为输入模式 */
   rt_pin_mode(ELE_INPUT_PIN, PIN_MODE_INPUT_PULLDOWN);
   /* 绑定中断，上升沿模式，回调函数名为InputCallBack */
   rt_pin_attach_irq(ELE_INPUT_PIN, PIN_IRQ_MODE_RISING, BumpInputCallBack, RT_NULL);
   /* 使能中断 */
   rt_pin_irq_enable(ELE_INPUT_PIN, PIN_IRQ_ENABLE);
}
void Init_PwmInput(void)
{
    /*输入捕获引脚初始化*/
    Init_InputPin();
    /*软件定时器初始化*/
    Init_SoftTimer();
}

void ProcessIcValue(uint8_t* fun_speed_t, uint8_t* bump_speed_t)
{
	if((fun_pwm_value/2) > 255){
		*fun_speed_t = 255;
	}else{
		*fun_speed_t = (UINT8)(fun_pwm_value/2);
	}

	if((ele_pwm_value/4) > 255){
		*bump_speed_t = 255;
	}else{
		*bump_speed_t = (UINT8)(ele_pwm_value/4);
	}
}

