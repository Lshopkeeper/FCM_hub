/******************************************************************************

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_can.c
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月11日
  最近修改   :
  功能描述   : FCM CAN消息收发任务
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月11日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/
#define DBG_TAG "fcm_can"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fcm_can.h"

#define CAN_MSG  CAN_PACKET_T
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/
struct rt_messagequeue recv_mq;   // CAN消息接收队列
struct rt_messagequeue send_mq;   // CAN消息发送队列

/* 消息队列中用到的放置消息的内存池 */
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t recv_msg_pool[FCM_CAN_MSG_POOL_NUM * sizeof(CAN_MSG)];//接收消息缓存池
/* 消息队列中用到的放置消息的内存池 */
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t send_msg_pool[FCM_CAN_MSG_POOL_NUM * sizeof(CAN_MSG)];//发送消息缓存池

rt_device_t can_dev;                // CAN设备句柄

static struct rt_semaphore sem_recv;       // 用于接收CAN消息的信号量
/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/

/*rt_device_t，rt_adc_device_t*/
/* FCM接收TIU数据回调函数 */
static rt_err_t fcm_can_rx_callback(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&sem_recv);

    return RT_EOK;
}

static rt_uint8_t fcm_can_ipc_init(void)
{
    rt_err_t result = 0;

    /******* TIU-FCM CAN 发送及接收任务初始化*******/
    /*初始化消息队列*/
    result = rt_mq_init(&recv_mq,"tiu_recv_mq",
                        (void*)&recv_msg_pool,  // 内存池指向 msg_pool
                        sizeof(CAN_MSG),                            // 每个消息的大小
                        sizeof(recv_msg_pool),  // 内存池的大小是 msg_pool 的大小
                        RT_IPC_FLAG_FIFO);                                    // 如果有多个线程等待，按照先来先得到的方法分配消息
    if(result != RT_EOK)
    {
        LOG_E("init TIU_Recv_mq message queue failed\n");
        return RT_ERROR;
    }
    result = rt_mq_init(&send_mq,"tiu_send_mq",
                       (void*)&send_msg_pool,    // 内存池指向 msg_pool
                       sizeof(CAN_MSG),                              // 每个消息的大小
                       sizeof(send_msg_pool),    // 内存池的大小是 msg_pool 的大小
                       RT_IPC_FLAG_FIFO);                                      // 如果有多个线程等待，按照先来先得到的方法分配消息
    if(result != RT_EOK)
    {
        LOG_E("init TIU_Send_mq message queue failed\n");
        return RT_ERROR;
    }
    /*初始化信号量*/
    result = rt_sem_init(&sem_recv, "tiu_recv_sem", 0, RT_IPC_FLAG_FIFO);
    if(result != RT_EOK)
    {
        LOG_E("init TIU_Resv_sem sem failed\n");
        return RT_ERROR;
    }

    return RT_EOK;
}

static rt_uint8_t fcm_can_dev_init(uint8_t dip_addr)
{
    int res = 0;

    can_dev = rt_device_find(FCM_CAN_DEV_NAME(1));
    if(NULL == can_dev)
    {
        LOG_E("Find device s% is failed!\n",FCM_CAN_DEV_NAME(CAN_DEV_INDEX_TIU));
        return RT_ERROR;
    }
    /* 以中断接收及发送方式打开 CAN 设备 */
    res |= rt_device_open(can_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX|RT_DEVICE_FLAG_INT_TX);
    /* 设置 CAN 通信的波特率为 250kbit/s*/
    res |= rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN_TIU_BAUD);
    /* 设置 CAN 的工作模式为正常工作模式 */
    res |= rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL);

    if(res != RT_EOK)
    {
        LOG_E("init TIU can failed\n");
        return RT_ERROR;
    }

    /*设置CAN接收回调函数*/
    rt_device_set_rx_indicate(can_dev, fcm_can_rx_callback);

#ifdef RT_CAN_USING_HDR

    uint16_t filter_addr=0xd060;
    filter_addr|=dip_addr;
    filter_addr|=(uint16_t)dip_addr<<8;

    struct rt_can_filter_item items[5] =
    {
        {filter_addr, 1, 0, 0, (0xffff << 3), 0,}       //ID ，扩展帧，数据帧，标识符列表模式，ID mask，0号滤波器组  //这样配置则接收标准ID为0x55的帧
    };
    struct rt_can_filter_config cfg = {1, 1, items}; /*一共有 5 个过滤表 */
    /* 设置硬件过滤表 */
    res = rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
    RT_ASSERT(res == RT_EOK);

#endif
    return RT_EOK;
}
// FCM CAN口初始化
void can_init(uint8_t dip_addr)
{
    if(fcm_can_ipc_init() || fcm_can_dev_init(dip_addr))
    {
        LOG_E("TIU CAN init failed!\n");
        return;
    }
}

static void send_bms_can_msg( uint32_t id, uint16_t len, uint8_t* buf)
{
    static struct rt_can_msg msg = {0};

    if(len > 8){
        return;
    }

    msg.id = id;
    msg.ide = RT_CAN_EXTID;     /* 扩展格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = len;

    rt_memcpy(msg.data, buf, len);
    rt_device_write(can_dev, 0, &msg, sizeof(msg));

}

void rev_tiu_can_task(void *parameter)
{
    struct rt_can_msg rx_msg = {0};
    CAN_MSG msg_ptr = {0};
    while(1)
    {   /*阻塞等待接收信号量*/
        /* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
        rx_msg.hdr = -1;

        rt_sem_take(&sem_recv, RT_WAITING_FOREVER);
        /*从CAN读取一帧数据*/
        rt_device_read(can_dev, 0, &rx_msg, sizeof(rx_msg));

        rt_uint8_t i = 0;
        for(i = 0;i < 8;i++)
        {
            msg_ptr.data[i] = rx_msg.data[i];
        }
        msg_ptr.identifier = rx_msg.id;
        msg_ptr.byte_count = rx_msg.len;
        if(recv_mq.msg_pool == NULL){
            return;
        }

        rt_mq_send(&recv_mq, (void *)&msg_ptr, sizeof(CAN_MSG));
    }
}

void send_tiu_can_task(void *parameter)
{
    CAN_MSG tx_msg = {0};
    while(1)
    {
        if(rt_mq_recv(&send_mq, (void*)&tx_msg, sizeof(CAN_MSG), 100) == RT_EOK)
        {
            send_bms_can_msg(tx_msg.identifier, tx_msg.byte_count, tx_msg.data);
            //rt_device_write(can_dev,0,&tx_msg,sizeof(tx_msg));
        }
    }
}

void send_msg_by_can(rt_uint32_t pgn, rt_uint8_t len, rt_uint8_t * buf)
{
    uint8_t src_addr = 0x55;
    uint8_t dest_addr = 0xAA;
    uint8_t priority = 6;
    uint32_t id = 0;
    //uint8_t buf[2] = {0x5A, 0xA5};

    id |= (uint32_t)pgn << 8;
    id |= (uint32_t)priority << 26;
    id |= (uint32_t)dest_addr << 8;
    id |= src_addr;

    struct rt_can_msg msg = {0};

    msg.id = id;
    msg.ide = RT_CAN_EXTID;     /* 扩展格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = 8;

    rt_memcpy(msg.data, buf, len);
    rt_device_write(can_dev, 0, &msg, sizeof(msg));
    rt_thread_mdelay(100);

    return ;

}

