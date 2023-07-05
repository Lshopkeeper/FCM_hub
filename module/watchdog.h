/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-11     LXH       the first version
 */
#ifndef APPLICATIONS_WATCHDOG_H_
#define APPLICATIONS_WATCHDOG_H_

int  wdt_init(void);
void feed_dog(void);
void close_watchdog(void);
#endif /* APPLICATIONS_WATCHDOG_H_ */
