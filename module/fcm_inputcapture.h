/******************************************************************************

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_inputcapture.h
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月14日
  最近修改   :
  功能描述   : fcm_inputcapture.c的头文件
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月14日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/

#ifndef APPLICATIONS_MODULE_FCM_INPUTCAPTURE_H_
#define APPLICATIONS_MODULE_FCM_INPUTCAPTURE_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "rtdevice.h"
#include "stm32f1xx_hal.h"
#include "board.h"
#include "fcm_pwm.h"
#include "fcm_common.h"
#include "fcm_can.h"
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
#define TIM_IC_Freq           1000000

#define TIM_ICOF_MAX          0x32

#define IC_TIM_PRESCALER      71
#define IC_TIM_PERIOD         0xFFFF


/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/
typedef enum
{
    EPUMP_INPUT_CAPTURE_NUM = 0,
    FUN_INPUT_CAPTURE_NUM,
    INPUT_CAPTURE_NUM,
}G_INPUTCAPTURE_TYPE;

typedef enum
{
    IC_RISING_EDGE_1 = 0, //第一次上升沿
    IC_RISING_EDGE_2,     //第二次上升沿
}G_IC_RISING_EDGE;
/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
typedef struct
{
    //rt_uint8_t overflow_cnt;      //溢出次数
    //rt_uint8_t input_cap_STA;     //输入捕获状态。虚拟寄存器：bit7 捕获完成标志  bit6 捕获高电平标志
                                  //bit5~bit0 捕获高电平后定时器溢出次数(概率极小) 此处不使用
    //rt_uint32_t cap_val_rising;   //上升沿中断时计数值
    //rt_uint32_t cap_val_down;     //下降沿中断时计数值

    float FbFreq;                   //频率反馈值
    float DevFbRotateSpd;            //转速反馈值

    rt_uint8_t ic_edage;            //第一次上升沿 0 ; 第二次上升沿 1

    rt_uint32_t cap_val1;           //第一次上升沿时cnt值
    rt_uint32_t cap_val2;           //第二次上升沿时cnt值
    rt_uint32_t cap_sum;            //周期时间

    rt_uint16_t IcofCntBuf;          //定时器溢出计数值


}T_TIM_INPUTCAP_INFO;
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern void Init_PwmInput(void);
//extern void Init_InputCap(void);
//extern void TIM_IcOverflowcntCallBack(TIM_HandleTypeDef *htim);
void ProcessIcValue(uint8_t* fun_speed_t, uint8_t* bump_speed_t);
/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /* APPLICATIONS_MODULE_FCM_INPUTCAPTURE_H_ */
