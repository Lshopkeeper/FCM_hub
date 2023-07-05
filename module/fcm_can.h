/******************************************************************************

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_can.h
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月11日
  最近修改   :
  功能描述   : fcm_can.c的头文件
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月11日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/
#ifndef APPLICATIONS_FCM_CAN_H_
#define APPLICATIONS_FCM_CAN_H_
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
//FCM CAN端口数量
#define FCM_CAN_NUM 1



//端口波特率设定
#define CAN_TIU_BAUD    CAN250kBaud

//接收消息池缓存消息数量
#define FCM_CAN_MSG_POOL_NUM   10

#define __FCM_CAN_DEV_NAME(i)       #i
#define _FCM_CAN_DEV_NAME(i)        __FCM_CAN_DEV_NAME(can##i)
#define FCM_CAN_DEV_NAME(i)         _FCM_CAN_DEV_NAME(i)

//CAN过滤器设置
#define RT_CAN_USING_HDR


/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
#pragma pack(1)
typedef struct
{
    uint32_t    identifier;
    uint8_t    data[8];
    uint8_t    byte_count;
}CAN_PACKET_T;

#pragma pack()
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern void can_init(uint8_t dip_addr);
extern void rev_tiu_can_task(void *parameter);
extern void send_tiu_can_task(void *parameter);
extern void send_msg_by_can(rt_uint32_t pgn, rt_uint8_t len, rt_uint8_t * buf);
/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern struct rt_messagequeue recv_mq;   // CAN消息接收队列
extern struct rt_messagequeue send_mq;   // CAN消息发送队列

extern rt_device_t can_dev;                // CAN设备句柄

#ifdef __cplusplus
}
#endif
#endif /* APPLICATIONS_FCM_CAN_H_ */
