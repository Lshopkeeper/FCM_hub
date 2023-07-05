/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-14     LXH       the first version
 */
#ifndef APPLICATIONS_USERAPP_FCM_COMMON_H_
#define APPLICATIONS_USERAPP_FCM_COMMON_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <rtconfig.h>
#include <rtthread.h>
#include <ulog.h>
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

#ifndef FALSE
#define  FALSE   (0)
#endif

#ifndef TRUE
#define  TRUE    (1)
#endif

#ifndef INT8
typedef char INT8;
#endif

#ifndef INT16
typedef short INT16;
#endif

#ifndef INT32
typedef int INT32;
#endif

#ifndef INT64
typedef long long INT64;
#endif

#ifndef UINT8
typedef unsigned char UINT8;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef UINT64
typedef unsigned long long UINT64;
#endif

#define STRING_DTU(s)   #s

#define HH_BYTE(x)  (((x)>>24)&0xFF)
#define HL_BYTE(x)  (((x)>>16)&0xFF)
#define LH_BYTE(x)  (((x)>>8)&0xFF)
#define LL_BYTE(x)  ((x)&0xFF)

//4字节CAN ID获取对应数据
#define CANID_GET_PRI(x)    (((x)>>26) & 0xFF)  //优先权
#define CANID_GET_PGN(x)    (((x)>>8) & 0xFF00) //PGN
#define CANID_GET_DST(x)    (((x)>>8) & 0xFF)   //目的地址
#define CANID_GET_SRC(x)    (((x)) & 0xFF)      //源地址

#define SYSTIME             rt_uint32_t

#define SYSTEM_TIME_GET(x)  (x = rt_tick_get())
#define TIME_INTERVAL       get_diff_time
#define TIME_DELAY          rt_thread_mdelay

//j1939 应答与广播
#define PGN_J1939_ACK               0xE800
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
typedef void (*ENCODE_FUNCTION)(UINT8 Index,UINT32 para);

typedef struct
{
    struct rt_thread *thread;           //线程控制块地址
    const char name[RT_NAME_MAX + 1];   //线程名称（长度由宏RT_NAME_MAX定义）
    void (*entry)(void *parameter);     //线程入口函数
    void             *parameter;        //线程入口函数参数
    void             *stack_start;      //线程栈起始地址
    UINT32       stack_size;            //线程栈大小
    UINT8        priority;              //线程优先级（数字越小，优先级越高）
    UINT32       tick;                  //线程时间片
    UINT8 ucDynamicTh;                  //动态线程标识,  TRUE-动态线程；FALSE-静态线程
    UINT8 ucStartupCreate;              //app启动时创建,True-启动时创建
}T_TASK_LIST;

/*can数据结构 报文基本信息定义*/
typedef struct{
    UINT16 wLen;          /* 数据长度 */
    UINT16 wPriority;     /* 优先级 */
    UINT16 wPeriod;       /* 发送周期 - 0为只发送一帧 */
    UINT32 dwTimeout;     /* 超时时间。全F为无超时时间 */
    UINT16 wPgn;          /* PGN */
}T_CAN_PGN_INFO;

/*can数据结构 报文状态信息定义*/
typedef struct{
    union
    {
        UINT8 bSendPeriod;
        UINT8 valid;
    };  /* 周期性发送标识 或 有效性标识 */
    UINT32 dwCount;     /* 发送次数 */
    UINT32 time;        /* 发送时间 */
    UINT32 dwExt;       /* 外部参数 */
}CAN_PGN_STATUS;

#pragma pack(1)

typedef struct
{
    UINT32 id;
    UINT8 buf[8];
}T_CAN_MSG;

#pragma pack()
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern uint32_t get_diff_time(uint32_t last_time, uint32_t cur_time);
extern UINT8 seek_index_by_pgn( UINT16 pgn, const T_CAN_PGN_INFO *pgn_para_list, UINT8 max_item_num);
extern void stop_send_pgn(UINT16 pgn,const T_CAN_PGN_INFO* Info,CAN_PGN_STATUS* status,UINT8 max_num);
extern void start_send_pgn(UINT16 pgn, const T_CAN_PGN_INFO* Info, CAN_PGN_STATUS* status, UINT8 max_num, ENCODE_FUNCTION EnCodeFun, UINT32 para);
extern void period_send_check(const T_CAN_PGN_INFO* Info,CAN_PGN_STATUS* status,UINT8 max_num,ENCODE_FUNCTION EnCodeFun,UINT8 max_send_num);

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
extern rt_uint8_t stop_feed_dog_flag;
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* APPLICATIONS_USERAPP_FCM_COMMON_H_ */
