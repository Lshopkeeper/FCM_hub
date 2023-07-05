/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-11     LXH       the first version
 */
#define DBG_TAG "watchdog"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>
#include "watchdog.h"

#define WDT_DEVICE_NAME    "wdt"    /* 看门狗设备名称 */

static rt_device_t wdg_dev;         /* 看门狗设备句柄 */

//static void idle_hook(void)
//{
//    /* 在空闲线程的回调函数里喂狗 */
//    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
//    //rt_kprintf("feed the dog!\n ");
//}

int wdt_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t timeout = 4;        /* 溢出时间，单位：秒 */
    char device_name[RT_NAME_MAX] = {0};

    rt_strncpy(device_name, WDT_DEVICE_NAME, RT_NAME_MAX);

    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdg_dev = rt_device_find(device_name);
    if (!wdg_dev)
    {
        LOG_E("find watchdog %s failed!\n", device_name);
        return RT_ERROR;
    }

    /* 设置看门狗溢出时间 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != RT_EOK)
    {
        LOG_E("set watchdog %s timeout failed!\n", device_name);
        return RT_ERROR;
    }

    /* 启动看门狗 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("start watchdog %s failed!\n", device_name);
        return -RT_ERROR;
    }
    /* 设置空闲线程回调函数 */
    //rt_thread_idle_sethook(idle_hook);

    return ret;
}

void feed_dog(void)
{
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}
void close_watchdog(void)
{
    rt_device_close(wdg_dev);
}
