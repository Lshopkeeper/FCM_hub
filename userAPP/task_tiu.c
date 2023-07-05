/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-22     LXH       the first version
 */
#define DBG_TAG "tiu"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fcm_can.h"
#include <task_tiu.h>
#include "fcm_pwm.h"

/*****************************************************************************/
/*命令帧*/
#define TIU_HANDSHAKE                           0x4000    //  TIU握手帧
#define FCM_HANDSHAKE                           0x4100    //  液冷控制器握手帧
#define TIU_TEMP_PARA_SET                       0x5000    //  温度档位参数设置帧
#define TIU_TEMP_PARA_SET_ACK                   0x5100    //  温度档位参数设置
#define TIU_SYNC_TIME                           0x5200    //  时间同步帧

/*数据帧*/
#define TIU_REMOTE_SIGNAL_STATUS                0x6000    //  TIU状态遥信帧
#define FCM_REMOTE_MEARSURE_STATUS              0x6100    //  液冷控制器遥测帧
#define FCM_REMOTE_SIGNAL_FAULT                 0x6200    //  液冷控制器故障遥信

/*心跳帧*/
#define TIU_HEART_BEAT                          0x9000    //  主控心跳帧
#define FCM_HEART_BEAT                          0x9100    //  终端心跳帧
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
/* FCM<->TIU 交互发送pgn基本信息定义 */
static const T_CAN_PGN_INFO gtCanInfoFcm2Tiu[]=
{
    /*数据长度，优先级      周期，        超时时间，                     PGN*/

    /*周期帧*/
    {8,       6,       500,       0xFFFFFFFF,       FCM_HEART_BEAT                  },/*FCM心跳*/
    {8,       6,       500,       0xFFFFFFFF,       FCM_REMOTE_MEARSURE_STATUS      },/*FCM数据遥测*/
    {8,       6,       500,       0xFFFFFFFF,       FCM_REMOTE_SIGNAL_FAULT         },/*FCM故障遥信*/

    /*命令帧*/
    {8,       6,       2000,      0xFFFFFFFF,       TIU_TEMP_PARA_SET_ACK           }/*TIU档位温度参数设置结果*/

};
static const T_CAN_PGN_INFO gtCanInfoTiu2Fcm[]=
{
    /*数据长度，优先级      周期，        超时时间，                     PGN*/
    /*周期帧*/
    {8,       6,       500,       5000,             TIU_HEART_BEAT                  },/*TIU心跳*/
    {8,       6,       500,       5000,             TIU_REMOTE_SIGNAL_STATUS        },/*TIU状态遥信*/

    /*命令帧*/
    {8,       6,       1000,      5000,             TIU_TEMP_PARA_SET               },/*TIU档位温度参数设置*/
    {8,       6,       2000,      0xFFFFFFFF,       TIU_SYNC_TIME                   },/*时间同步帧*/
    {8,       6,       5000,      0xFFFFFFFF,       PGN_J1939_ACK                   }   /*ack*/
};

/* DTU<->TIU 帧收发状态定义 */
static CAN_PGN_STATUS gtCanStatusFcm2Tiu[sizeof(gtCanInfoFcm2Tiu)/sizeof(T_CAN_PGN_INFO)] = {0};
static CAN_PGN_STATUS gtCanStatusTiu2Fcm[sizeof(gtCanInfoTiu2Fcm)/sizeof(T_CAN_PGN_INFO)] = {0};

TERM_COMM_DATA  g_tiu_data = {0};
static void encode_and_send_tiu_can_message(rt_uint8_t uxCanIndex, rt_uint32_t uxPara);
void send_can_msg2tiu(rt_uint32_t id, rt_uint16_t len, rt_uint8_t* buf);

static UINT8 recv_flag = 1;
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
void init_tiu_task(uint8_t dip_addr)
{
    rt_memset(&g_tiu_data, 0, sizeof(TERM_COMM_DATA));
    g_tiu_data.fcm_heartbeat.version[0] = (FCM_COMMU_PROTOCOL_VERSION>>8)&0xff;
    g_tiu_data.fcm_heartbeat.version[1] = FCM_COMMU_PROTOCOL_VERSION&0xff;

    //液冷系统温度设定值与风扇油泵转速定值初始化
    g_tiu_data.tiu_temp_para_set.max_temp[0] = 10;
    g_tiu_data.tiu_temp_para_set.fun_pwm[0] = PWM_DUTY_CYCLE_30;
    g_tiu_data.tiu_temp_para_set.bump_pwm[0] = PWM_DUTY_CYCLE_30;

    g_tiu_data.tiu_temp_para_set.max_temp[1] = 30;
    g_tiu_data.tiu_temp_para_set.fun_pwm[1] = PWM_DUTY_CYCLE_70;
    g_tiu_data.tiu_temp_para_set.bump_pwm[1] = PWM_DUTY_CYCLE_60;

    g_tiu_data.tiu_temp_para_set.max_temp[2] = 50;
    g_tiu_data.tiu_temp_para_set.fun_pwm[2] = PWM_DUTY_CYCLE_90;
    g_tiu_data.tiu_temp_para_set.bump_pwm[2] = PWM_DUTY_CYCLE_70;

    g_tiu_data.tiu_temp_para_set.max_temp[3] = 120;
    g_tiu_data.tiu_temp_para_set.fun_pwm[3] = PWM_DUTY_CYCLE_100;
    g_tiu_data.tiu_temp_para_set.bump_pwm[3] = PWM_DUTY_CYCLE_80;


    rt_mutex_init(&g_tiu_data.uxMulitiPackSendLock, "TiuMPSL", RT_IPC_FLAG_FIFO);

//    uint8_t tmp=0;
//    
//    tmp=get_io_state(DIGITAL_KIN4);//KIN4为高位
//    tmp<<=1;
//    tmp|=get_io_state(DIGITAL_KIN3);//kin3为低位
    
    g_tiu_data.src_addr=dip_addr+CAN_FCM2TIU_FCM_BASE_ADDR;//通过KIN4和KIN3模拟拨码地址
    g_tiu_data.dest_addr=dip_addr+CAN_FCM2TIU_TIU_BASE_ADDR;


}

/*判断是否是多包数据*/
static rt_uint8_t Is_tiu_multi_package(T_CAN_MSG *rxmsg)
{
    rt_uint8_t result = 0;
    rt_uint16_t pgn = CANID_GET_PGN(rxmsg->id);

    switch(pgn)
    {
        case 0:
            result = 1;
        default:
            break;
    }
    return result;
}
/*多包数据是否接收完整*/
static rt_uint8_t is_complete_multi_package(T_CAN_MSG *rxmsg, rt_uint16_t* len, rt_uint8_t* buf)
{
    rt_uint8_t result = 0;
    static rt_uint8_t data_buf[CAN_MSG_MAX_LEN] = {0};
    rt_uint8_t pack_index = 0;  //包序号
    static rt_uint8_t max_pack_index = 0;//包数量或最大包序号
    static rt_uint16_t sum_vaild = 0;   //校验码
    static rt_uint16_t total_len = 0;   //数据总长度
    static rt_uint16_t cur_len = 0;     //当前数据长度

    if(!Is_tiu_multi_package(rxmsg))
    {
        return result;
    }
    pack_index = rxmsg->buf[0];
    switch(pack_index)
    {
        case 0:
            break;
        case 1:
            total_len = rxmsg->buf[2]|((rt_uint16_t)rxmsg->buf[3]<<8);
            max_pack_index = rxmsg->buf[1];
            rt_memset(data_buf, 0, CAN_MSG_MAX_LEN);
            rt_memcpy(&data_buf, &rxmsg->buf[4], 4);
            /*首包只有四个数据*/
            cur_len = 4;
        default:
            if(cur_len >= CAN_MSG_MAX_LEN)
            {
                result = 0;
                break;
            }

            if(cur_len + 7 >= total_len)//判断是否是最后一包
            {   //尾包有效数据长度
                rt_uint8_t num = total_len - cur_len;
                //有效数据是从1-7
                rt_memcpy(&data_buf[cur_len], &rxmsg->buf[1], num);
                cur_len += num;
                //特殊情况，校验码跨包
                if(num == 6)
                {
                    sum_vaild = rxmsg->buf[7];
                }
            }else{  //不是最后一包
                //每一包都有7个数据
                rt_memcpy(&data_buf[cur_len], &rxmsg->buf[1], 7);
                cur_len += 7;
            }
            //尾包数据处理
            if(max_pack_index == pack_index)
            {
                rt_uint8_t num = (total_len - 4) % 7;
                if(num == 6)
                {
                    sum_vaild |= ((rt_uint16_t)rxmsg->buf[1] << 8);
                }else {
                    sum_vaild = rxmsg->buf[num+1] | ((rt_uint16_t)rxmsg->buf[num+2]<<8);
                }
                rt_uint16_t check_num = 0,i;
                for(i=0;i<total_len;i++)
                {
                    check_num += data_buf[i];  //计算所有数据和
                }
                check_num += (max_pack_index + (total_len&0xFF) + ((total_len >> 8)&0xFF));
                if(check_num == sum_vaild){
                    rt_memcpy(buf, data_buf, cur_len);
                    result = 1;
                }
            }
            break;
    }
    *len = cur_len;
    return result;
}
#if(0)
/*发送ack*/
static void send_j1939_ack2tiu(uint16_t pgn, uint8_t ack )
{
  uint8_t data[8] = {0};
  data[0] = ack;
  data[1] = 0x00;
  data[2] = 0xFF;
  data[3] = 0xFF;
  data[4] = 0xFF;
  data[5] = (uint8_t)(pgn & 0xFF);
  data[6] = (uint8_t)((pgn>>8) & 0xFF);
  data[7] = 0x00;

    uint8_t src_addr = CAN_FCM2TIU_FCM_BASE_ADDR;
    uint8_t dest_addr = CAN_FCM2TIU_TIU_BASE_ADDR;
    uint8_t priority = 6;
    uint8_t len = 8;
    uint32_t id = 0;

    uint16_t pgn_ack = PGN_J1939_ACK;//j1939中规定，ack pgn
    id |= (uint32_t)pgn_ack << 8;
    id |= (uint32_t)priority << 26;
    id |= (uint32_t)dest_addr << 8;
    id |= src_addr;

    send_can_msg2tiu(id, len, data);
}
#endif
static void process_j1939_ack_msg(uint16_t pgn_ack, uint8_t* buf)
{
    //j1939中规定，ack pgn 0xE800
    if(pgn_ack != PGN_J1939_ACK)
    {
        return;
    }

    uint16_t pgn = 0;
    pgn=buf[6];pgn<<=8;

    switch(pgn)
    {
        case FCM_HEART_BEAT:
        case FCM_REMOTE_MEARSURE_STATUS:
        case FCM_REMOTE_SIGNAL_FAULT:
        case FCM_HANDSHAKE:
            break;
        default:
            STOP_SEND2TIU(pgn);
            break;
    }
}

/*解析TIU状态遥信帧*/
static void _decode_tiu_remote_signal_status_msg(UINT8 *buf)
{
    rt_memcpy(&g_tiu_data.tiu_signal_status,buf,2);
}
/*解析TIU参数设置帧*/
static void _decode_tiu_temp_gear_para_set(UINT8 *buf)
{
    UINT8 Index = 0,i = 0;
    g_tiu_data.tiu_temp_para_set.temp_index = buf[i++];
    g_tiu_data.fcm_temp_para_set_result.temp_index = g_tiu_data.tiu_temp_para_set.temp_index;
    Index = g_tiu_data.tiu_temp_para_set.temp_index-1;
    if(buf[1]==0)
    {
        g_tiu_data.fcm_temp_para_set_result.success_flag = 1;
        return;
    }
    switch(Index)
    {
      case temp_gear_1:

          g_tiu_data.tiu_temp_para_set.max_temp[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.fun_pwm[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.bump_pwm[Index] = buf[i++];
          break;
      case temp_gear_2:

          g_tiu_data.tiu_temp_para_set.max_temp[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.fun_pwm[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.bump_pwm[Index] = buf[i++];
          break;
      case temp_gear_3:

          g_tiu_data.tiu_temp_para_set.max_temp[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.fun_pwm[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.bump_pwm[Index] = buf[i++];
          break;

      case temp_gear_4:

          g_tiu_data.tiu_temp_para_set.max_temp[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.fun_pwm[Index] = buf[i++];
          g_tiu_data.tiu_temp_para_set.bump_pwm[Index] = buf[i++];

          recv_flag = 1;
          break;
      default:

          break;
    }
    g_tiu_data.fcm_temp_para_set_result.success_flag = 0;
}
/*解析TIU时间同步帧*/
static void _decode_tiu_sync_time(UINT8 *buf)
{
    rt_memcpy(&g_tiu_data.tiu_sync_time,buf,8);
}

//解码收到的TIU CAN报文
static void decode_Tiu_can_msg(rt_uint16_t pgn_id, rt_uint16_t len, rt_uint8_t *buf)
{
   switch(gtCanInfoTiu2Fcm[pgn_id].wPgn)
   {
       case TIU_HEART_BEAT:

           break;
       case TIU_REMOTE_SIGNAL_STATUS:

           _decode_tiu_remote_signal_status_msg(buf);

           break;
       case TIU_TEMP_PARA_SET:

           _decode_tiu_temp_gear_para_set(buf);
           SATRT_SEND2TIU(TIU_TEMP_PARA_SET_ACK);
           break;
       case TIU_SYNC_TIME:

           _decode_tiu_sync_time(buf);

           break;
       case PGN_J1939_ACK:
           process_j1939_ack_msg(gtCanInfoTiu2Fcm[pgn_id].wPgn,buf);
           break;
       default:
           break;
   }
}

static void proc_tiu_can_msg(T_CAN_MSG *rcv_msg)
{
    rt_uint16_t pgn = 0,len = 0,pgn_id = 0;
    rt_uint8_t buff[CAN_MSG_MAX_LEN] = {0};

    if(Is_tiu_multi_package(rcv_msg))  //多包
    {
        if(is_complete_multi_package(rcv_msg,&len,buff))
        {
            /*多包收集完成后进行下一步*/
        }else{
            return;
        }
    }
	else  //单包
	{
        rt_memcpy(buff, rcv_msg->buf, CAN_MSG_SINGLE_PACKAGE_LEN);
    }
	
    pgn = CANID_GET_PGN(rcv_msg->id);
    pgn_id = SEEK_INDEX_BY_PGN_RECV_TIU(pgn);

    //升级
    if(pgn_id == 0xFF)
    {
        switch(pgn)
        {
            case 0:
                break;
            default:
                break;
        }
        return;
    }
    SYSTEM_TIME_GET(gtCanStatusTiu2Fcm[pgn_id].time);

    gtCanStatusTiu2Fcm[pgn_id].bSendPeriod = TRUE;

    decode_Tiu_can_msg(pgn_id,len,buff);
}





/*can发送，包含单包多包*/
void send_can_msg2tiu(rt_uint32_t id, rt_uint16_t len, rt_uint8_t* buf)
{
    CAN_PACKET_T txmsg = {0};

    txmsg.identifier = id;
    txmsg.byte_count = len;

    if(len <= CAN_MSG_SINGLE_PACKAGE_LEN)  //todo:单包
    {
        rt_memcpy(txmsg.data,buf,len);
        rt_mq_send(&send_mq, (void*)&txmsg,sizeof(CAN_PACKET_T));
    }else{
        // todo:多包
        rt_uint8_t offset = 0,package_num = (len + 6 + 5) / 7;
        rt_uint16_t i = 0, sum_check = 0;

        rt_uint8_t send_bBuf[CAN_MSG_MAX_LEN] = {0};

        send_bBuf[0] = package_num;
        send_bBuf[1] = len&0xFF;
        send_bBuf[2] = (len>>8)&0xFF;
        if(len >= CAN_MSG_MAX_LEN)
        {
            len = CAN_MSG_MAX_LEN - 1;
        }
        rt_memcpy(&send_bBuf[3], buf, len);
        for(i=0;i<(len+3);i++)
        {
            sum_check += send_bBuf[i];
        }
        send_bBuf[len+3] = sum_check&0xFF;
        send_bBuf[len+1+3] = (sum_check>>8)&0xFF;

        rt_mutex_take(&g_tiu_data.uxMulitiPackSendLock, RT_WAITING_FOREVER);
        for(i = 0;i < package_num; i++)
        {
            txmsg.data[0] = i+1;
            rt_memcpy(&txmsg.data[1],&send_bBuf[offset],7);
            rt_mq_send(&send_mq,(void*)&txmsg,sizeof(CAN_PACKET_T));
            offset += 7;
        }
        rt_mutex_release(&g_tiu_data.uxMulitiPackSendLock);
    }
}

static void _encode_tiu_heartbeat(UINT8 *buf)
{
    buf[0] = g_tiu_data.fcm_heartbeat.count++;
    buf[1] = (FCM_COMMU_PROTOCOL_VERSION>>8)&0xff;
    buf[2] = FCM_COMMU_PROTOCOL_VERSION&0xff;
}

static void _encode_tiu_temp_para_set_result(UINT8 *buf)
{
    buf[0] = g_tiu_data.fcm_temp_para_set_result.temp_index;
    buf[1] = g_tiu_data.fcm_temp_para_set_result.success_flag;
}
static void _encode_tiu_remote_measure(UINT8 *buf)
{
    UINT8 i = 0;

    buf[i++] = g_tiu_data.fcm_remote_measure.input_temp;
    buf[i++] = g_tiu_data.fcm_remote_measure.output_temp;
    buf[i++] = g_tiu_data.fcm_remote_measure.press_val;
    buf[i++] = g_tiu_data.fcm_remote_measure.fun_pwm_speed;
    buf[i++] = g_tiu_data.fcm_remote_measure.bump_pwm_speed;
    buf[i++] = g_tiu_data.fcm_remote_measure.fluid_level;
    buf[i++] = g_tiu_data.fcm_remote_measure.fun_speed;
    buf[i++] = g_tiu_data.fcm_remote_measure.bump_speed;
}
static void _encode_tiu_remote_signal_fault(UINT8 *buf)
{
    UINT8 i = 0;

    buf[i++] = g_tiu_data.fcm_remote_signal_fault.bump_fault;
	buf[i++] = g_tiu_data.fcm_remote_signal_fault.fun_fault;
    buf[i++] = g_tiu_data.fcm_remote_signal_fault.press_alarm;
    buf[i++] = g_tiu_data.fcm_remote_signal_fault.fluid_divulge;
    buf[i++] = g_tiu_data.fcm_remote_signal_fault.sys_overtemp;
}

//编码并发送CAN报文
static void encode_and_send_tiu_can_message(rt_uint8_t pgn_id, rt_uint32_t para)
{
    //rt_uint8_t uxIndex = HH_BYTE(uxPara);
    rt_uint8_t buf[CAN_MSG_MAX_LEN] = {0};

    //rt_uint8_t src_addr = CAN_FCM2TIU_FCM_BASE_ADDR;
    //rt_uint8_t dest_addr = LL_BYTE(para);
    
    rt_uint32_t id = 0;

    if(0xFF == pgn_id)
    {
        return;
    }
    switch(gtCanInfoFcm2Tiu[pgn_id].wPgn)
    {
        case FCM_HEART_BEAT:
            _encode_tiu_heartbeat(buf);
            break;
        case TIU_TEMP_PARA_SET_ACK:
            _encode_tiu_temp_para_set_result(buf);
            break;
        case FCM_REMOTE_MEARSURE_STATUS:
            _encode_tiu_remote_measure(buf);
            break;
        case FCM_REMOTE_SIGNAL_FAULT:
            _encode_tiu_remote_signal_fault(buf);
            break;
        default:
            break;
    }
    id |= (rt_uint32_t)gtCanInfoFcm2Tiu[pgn_id].wPgn<<8;
    id |= (rt_uint32_t)gtCanInfoFcm2Tiu[pgn_id].wPriority<<26;
    id |= (rt_uint32_t)g_tiu_data.dest_addr<<8;
    id |= g_tiu_data.src_addr;

    send_can_msg2tiu(id,gtCanInfoFcm2Tiu[pgn_id].wLen,buf);
}

//TIU接收 任务
void recv_tiu_task(void *parameter)
{
    while(1)
    {
        CAN_PACKET_T rxmsg = {0};
        T_CAN_MSG msg_ptr = {0}; /* 用于放置消息的局部变量 */

        /* 从消息队列中接收消息 */
        if (rt_mq_recv(&recv_mq, (void*)&rxmsg, sizeof(CAN_PACKET_T), 1) == RT_EOK)
        {
            msg_ptr.id = rxmsg.identifier;
            rt_memcpy(msg_ptr.buf, rxmsg.data, rxmsg.byte_count);

            /* 打印数据 ID 及内容 */
            /*LOG_D("tiu recv ID : 0x%x, DATA : %2x %2x %2x %2x %2x %2x %2x %2x\n", msg_ptr.id,
                msg_ptr.buf[0], msg_ptr.buf[1], msg_ptr.buf[2], msg_ptr.buf[3],
                msg_ptr.buf[4], msg_ptr.buf[5], msg_ptr.buf[6], msg_ptr.buf[7]);*/

            proc_tiu_can_msg(&msg_ptr);
        }
    }
}
//TIU发送任务
void send_tiu_task(void *parameter)
{
    //启动心跳发送
    SATRT_SEND2TIU(FCM_HEART_BEAT);
    //开始发送遥测帧
    SATRT_SEND2TIU(FCM_REMOTE_MEARSURE_STATUS);
    //开始发送遥信帧
    SATRT_SEND2TIU(FCM_REMOTE_SIGNAL_FAULT);

    while(1)
    {
        PERIOD_CHECK_SEND2TIU();
        rt_thread_mdelay(5);
    }
}

TERM_COMM_DATA * task_tiu_para_list_get(void)
{
    return &g_tiu_data;
}

rt_uint8_t get_gear_temp(rt_uint8_t temp_id)
{
    if(temp_id>=4)
    {
        return 0;
    }else{

        return g_tiu_data.tiu_temp_para_set.max_temp[temp_id];
    }
}
