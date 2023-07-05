/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-14     LXH       the first version
 */
#define DBG_TAG "fcm_common"
#define DBG_LVL LOG_LVL_DBG
/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <rtdbg.h>
#include <rthw.h>
#include "fcm_common.h"
/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
rt_uint8_t stop_feed_dog_flag = FALSE;
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
//传入值以rt_tick为参考，返回值单位ms
uint32_t get_diff_time(uint32_t last_time, uint32_t cur_time)
{
    uint32_t diff_time = 0;

    if(cur_time >= last_time)
    {
        diff_time = cur_time - last_time;
    }
    else
    {
        diff_time = 0xFFFFFFFF - last_time + cur_time;
    }

    return diff_time;
}

/* 根据pgn判断在数组中的序号 */
UINT8 seek_index_by_pgn( UINT16 pgn, const T_CAN_PGN_INFO *pgn_para_list, UINT8 max_item_num)
{
    for(UINT8 i = 0; i < max_item_num; i++ )
    {
        if( pgn == pgn_para_list[i].wPgn )
        {
            return i;
        }
    }

    LOG_W("seek pgn index failed! (0x%08x : %x) \n", (UINT32)pgn_para_list, pgn);

    return 0xFF;
}


/* 停止发送 */
void stop_send_pgn(UINT16 pgn,const T_CAN_PGN_INFO* Info,CAN_PGN_STATUS* status,UINT8 max_num)
{
    /* 序号查找 */
    UINT8 i = seek_index_by_pgn(pgn, Info, max_num);

    /* 取消定期发送标志 */
    if(i < max_num)
    {
        status[i].bSendPeriod = FALSE;
    }
    else
    {
        LOG_E("can't stop send index(%d) error : pgn 0x%d!\n", i,pgn);
    }
}

/* 开始周期性发送 */
void start_send_pgn(UINT16 pgn, const T_CAN_PGN_INFO* Info, CAN_PGN_STATUS* status, UINT8 max_num, ENCODE_FUNCTION EnCodeFun, UINT32 para)
{
    /* 序号查找 */
    UINT8 i = seek_index_by_pgn(pgn, Info, max_num);

    /* 帧信息编辑 */
    if (i < max_num)
    {
        /* 信息初始化 */
        SYSTEM_TIME_GET(status[i].time);/*获取当前时间*/
        status[i].dwCount = 0;
        status[i].dwExt = para;
        status[i].bSendPeriod = TRUE;
    }
    else
    {
        LOG_E("can start send index(%d) error : pgn 0x%d!\n", i,pgn);
    }
}

/* 周期检查是否达到发送周期 */
void period_send_check(const T_CAN_PGN_INFO* Info,CAN_PGN_STATUS* status,UINT8 max_num,ENCODE_FUNCTION EnCodeFun,UINT8 max_send_num)
{
    UINT8 i, k = 0;
    UINT32 diffTm;
    rt_tick_t curtTm;

    /* 获取当前时间 */
    SYSTEM_TIME_GET(curtTm);

    /* 遍历全部消息 */
    for(i = 0; i < max_num; i++)
    {
        /* 需要发送消息 */
        if(status[i].bSendPeriod == TRUE)
        {
            /* 第一次发送到现在的时间 */
            diffTm = TIME_INTERVAL(status[i].time, curtTm);

            /* 发送超时，需要停止发送 */
            if(diffTm > Info[i].dwTimeout)
            {
                /* 超时处理 */

                /* 停止消息发送检查*/
                status[i].bSendPeriod=FALSE;
            }

            /* 发送未超时 */
            else
            {
                /* 理论上应该已经发送的消息的次数 */
                diffTm /= Info[i].wPeriod;

                /* 大于实际发送的次数，需要发送 */
                if(diffTm > status[i].dwCount)
                {
                    /* 消息是永不超时消息 */
                    //if(Info[i].timeout==0xFFFFFFFF)
                    if(Info[i].dwTimeout & 0x80000000)    /*as same*/
                    {
                        /* 重新记录发送时间 清空发送次数 */
                        status[i].time = curtTm;
                        status[i].dwCount = 0;
                    }

                    /* 更新发送次数 */
                    else
                    {
                        status[i].dwCount = diffTm;
                    }

                    /* 消息继续发送 */
                    EnCodeFun(i, status[i].dwExt);

                    /* 检查是否需要退出本次检查 */
                    if(++k >= max_send_num)
                    {
                        break;
                    }
                }
            }
        }
    }
}
