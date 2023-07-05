/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-22     LXH       the first version
 */
#ifndef APPLICATIONS_USERAPP_TASK_TIU_H_
#define APPLICATIONS_USERAPP_TASK_TIU_H_
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "rtdevice.h"
#include "fcm_common.h"
#include "io_operate.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
#define MAX_GUN_NUM    1

//协议版本
#define FCM_COMMU_PROTOCOL_VERSION      0x0100

#define CAN_MSG_SINGLE_PACKAGE_LEN 8
#define CAN_MSG_MAX_LEN  256
//单次向TIU连续发送的CAN消息条数限值
#define MAX_SEND_TIU_CAN_NUM_ONCE 3

//液冷控制器源地址和目的地址
#define CAN_FCM2TIU_FCM_BASE_ADDR       0xD0
#define CAN_FCM2TIU_TIU_BASE_ADDR       0x60

#define PERIOD_SEND                     0x00
#define STOP_SEND                       0x01

#define SEEK_INDEX_BY_PGN_RECV_TIU(pgn) seek_index_by_pgn(pgn, gtCanInfoTiu2Fcm, (sizeof(gtCanInfoTiu2Fcm)/sizeof(T_CAN_PGN_INFO)))
#define SEEK_INDEX_BY_PGN_SEND_TIU(pgn) seek_index_by_pgn(pgn, gtCanInfoFcm2Tiu, (sizeof(gtCanInfoFcm2Tiu)/sizeof(T_CAN_PGN_INFO)))
#define STOP_SEND2TIU(Pgn) stop_send_pgn(Pgn,gtCanInfoFcm2Tiu,gtCanStatusFcm2Tiu,(sizeof(gtCanInfoFcm2Tiu)/sizeof(T_CAN_PGN_INFO)))
#define SATRT_SEND2TIU(Pgn) start_send_pgn(Pgn,gtCanInfoFcm2Tiu,gtCanStatusFcm2Tiu,(sizeof(gtCanInfoFcm2Tiu)/sizeof(T_CAN_PGN_INFO)), (ENCODE_FUNCTION)encode_and_send_tiu_can_message, (CAN_FCM2TIU_TIU_BASE_ADDR))//32位数b的最高两位为设备序号，最低两位为目标地址(区分单播及组播用)
#define PERIOD_CHECK_SEND2TIU(i) period_send_check(gtCanInfoFcm2Tiu,gtCanStatusFcm2Tiu,(sizeof(gtCanInfoFcm2Tiu)/sizeof(T_CAN_PGN_INFO)), (ENCODE_FUNCTION)encode_and_send_tiu_can_message, MAX_SEND_TIU_CAN_NUM_ONCE)

/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/
//温度档位
typedef enum
{
    temp_gear_1 = 0,
    temp_gear_2,
    temp_gear_3,
    temp_gear_4
}TIU_TEMP_GEAR;

typedef enum
{
    TIU_STATE_IDLE = 0,             //空闲
    TIU_STATE_CONNECT,              //连接
    TIU_STATE_HANDSHAKE,            //握手
    TIU_STATE_INSU_DET,             //绝缘
    TIU_STATE_PRE_CHARGE,           //预充
    TIU_STATE_CHARGING,             //充电
    TIU_STATE_STAOPING,             //停止
    TIU_STATE_COMPLETE,             //完成
    TIU_STATE_FAULT,                //故障
}TIU_CHARGE_STATE;
/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
//定义发送的报文数据结构

/*温度参数设置结果帧*/
typedef struct{
    UINT8 temp_index;               /*温度档位设置序号*/
    UINT8 success_flag;             /*成功标志*/
}FCM_TEMP_GEAR_PARA_SET_RESULT;
/*FCM遥测帧*/
typedef struct{
    UINT8 input_temp;               /*进油口温度*/
    UINT8 output_temp;              /*出油口温度*/
    UINT8 press_val;                /*液体压力值*/
    UINT8 fun_pwm_speed;            /*风扇PWM速度*/
    UINT8 bump_pwm_speed;           /*油泵PWM速度*/
    UINT8 fluid_level;              /*液位信息*/
    UINT8 fun_speed;                /*风扇速度反馈值*/
    UINT8 bump_speed;               /*油泵速度反馈值*/
}FCM_REMOTE_MEASURE_INFO;
/*FCM故障遥信帧*/
typedef struct{
    UINT8 bump_fault;               /*油泵故障*/
    UINT8 fun_fault;                /*风扇故障*/
    UINT8 press_alarm;              /*液压告警*/
    UINT8 fluid_divulge;            /*冷却液泄漏故障*/
    UINT8 sys_overtemp;             /*液冷系统过温，0x01告警，0x10故障*/
    UINT8 reserve[3];               /*预留*/
}FCM_REMOTE_SIGNAL_FAULT_INFO;
/*FCM心跳帧*/
typedef struct{
    UINT8 count;
    UINT8 version[2];
}FCM_HEARTBEAT_MSG;
//定义收到的报文数据结构
/*心跳帧*/
typedef struct
{
    UINT8 count;
}TIU_HEARTBEAT_MSG;
/*TIU遥信状态帧*/
typedef struct
{
    UINT8 gun_status;               /*枪状态*/
    UINT8 gun_temp;                 /*枪头温度*/
}TIU_SINGNAL_STATUS;
/*TIU温度档位参数设置*/
typedef struct{
    UINT8 temp_index;               /*温度设置档位序号*/
    UINT8 max_temp[4];              /*代表四个档位的最大温度，前一个的最大温度是后者的最小温度，第四档可不使用最大温度*/
    UINT8 fun_pwm[4];               /*代表四个档位的风扇转速*/
    UINT8 bump_pwm[4];              /*代表四个档位的油泵转速*/
}TIU_TEMP_GEAR_PARA_SET;

/* 时间格式 */
typedef struct
{
    UINT8 second;                   /*秒*/
    UINT8 minute;                   /*分*/
    UINT8 hour;                     /*时*/
    UINT8 day;                      /*日*/
    //uint8_t weekday;
    UINT8 month;                    /*月*/
    UINT16 year;                    /*年*/
}TIU_SYNC_TIME_TYPE;

typedef struct{
    /*发送信息*/
    FCM_TEMP_GEAR_PARA_SET_RESULT fcm_temp_para_set_result;

    FCM_REMOTE_MEASURE_INFO fcm_remote_measure;

    FCM_REMOTE_SIGNAL_FAULT_INFO fcm_remote_signal_fault;

    FCM_HEARTBEAT_MSG fcm_heartbeat;


    /*接收信息*/
    TIU_HEARTBEAT_MSG tiu_heartbeat;

    TIU_SINGNAL_STATUS tiu_signal_status;

    TIU_TEMP_GEAR_PARA_SET tiu_temp_para_set;

    TIU_SYNC_TIME_TYPE tiu_sync_time;

    struct rt_mutex uxMulitiPackSendLock;   //CAN多包发送锁，防止多包发送时多进程抢占，导致多包嵌套

    uint8_t src_addr;
    uint8_t dest_addr;
}TERM_COMM_DATA;
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/

extern void recv_tiu_task(void *parameter);
extern void send_tiu_task(void *parameter);
extern void init_tiu_task(uint8_t dip_addr);

extern TERM_COMM_DATA * task_tiu_para_list_get(void);
extern rt_uint8_t get_gear_temp(rt_uint8_t temp_id);
#ifdef __cplusplus
}
#endif
#endif /* APPLICATIONS_USERAPP_TASK_TIU_H_ */
