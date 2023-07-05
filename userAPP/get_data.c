/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-12     LXH       the first version
 */
#define DBG_TAG "fcm_temp"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include <get_data.h>
#include "fcm_pwm.h"
#include "io_operate.h"
#include "fcm_inputcapture.h"
#include <task_tiu.h>
/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/
G_GET_TEMP_PRESS_PARA g_get_ad_para = {0};
/*----------------------------------------------*
 * 内部变量原型说明                             *
 *----------------------------------------------*/
 static double g_temp_value = 0;
 static double g_press_value =0;
 uint8_t fun_speed_t = 0;
 uint8_t bump_speed_t = 0;

//==========================================================================================
// 温度查找表 PT1000
//==========================================================================================
const rt_uint32_t m_pt1000_table[]=
{
    803063, 807033, 811003, 814970, 818937, 822902, 826865, 830828, 834789, 838748,       //-50 -- -41

    842707, 846664, 850619, 854573, 858526, 862478, 866428, 870377, 874325, 878272,      //-40 -- -31

    882217, 886161, 890103, 894044, 897985, 901923, 905861, 909798, 913733, 917666,      //-30 -- -21

    921599, 925531, 929460, 933390, 937317, 941244, 945170, 949094, 953016, 956938,      //-20 -- -11

    960859, 964779, 968697, 972614, 976529, 980444, 984358, 988270, 992181, 996091,      //-10 -- -1

    1000000, 1003908, 1007814, 1011720, 1015624, 1019527, 1023429, 1027330, 1031229, 1035128,  //0 -- 9

    1039025, 1042921, 1046816, 1050710, 1054603, 1058495, 1062385, 1066274, 1070162, 1074049,  //10 -- 19

    1077935, 1081820, 1085703, 1089585, 1093467, 1097347, 1101225, 1105103, 1108980, 1112855,   //20 -- 29

    1116729, 1120602, 1124474, 1128345, 1132215, 1136083, 1139950, 1143817, 1147681, 1151545,   //30 -- 39

    1155408, 1159270, 1163130, 1166989, 1170847, 1174704, 1178560, 1182414, 1186268, 1190120,   //40 -- 49

    1193971, 1197821, 1201670, 1205518, 1209364, 1213210, 1217054, 1220897, 1224739, 1228579,    //50 -- 59

    1232419, 1236257, 1240095, 1243931, 1247766, 1251600, 1255432, 1259264, 1263094, 1266923,    //60 -- 69

    1270751, 1274578, 1278404, 1282228, 1286052, 1289874, 1293695, 1297515, 1301334, 1305152,    //70 -- 79

    1308968, 1312783, 1316597, 1320411, 1324222, 1328033 , 1331843, 1335651, 1339458, 1343264,    //80 -- 89

    1347069, 1350873, 1354676, 1358477, 1362277, 1366077, 1369875, 1373671, 1377467, 1381262,    //90 -- 99

    1385055, 1388847, 1392638, 1396428, 1400217, 1404005, 1407791, 1411576, 1415360, 1419143,    //100 -- 109

    1422925, 1426706, 1430485, 1434264, 1438041, 1441817, 1445592, 1449366, 1453138, 1456910,    //110 -- 119

    1460680, 1464449, 1468217, 1471984, 1475750, 1479514, 1483277, 1487040, 1490801, 1494561,    //120 -- 129

    1498319, 1502076, 1505833, 1509589, 1513343, 1517096, 1520847, 1524598, 1528381, 1532139,    //130 -- 139

    1535843, 1539589, 1543334, 1547078, 1550820, 1554562, 1558302, 1562041, 1565779, 1569516,    //140 -- 149

    1573251, 1576986, 1580719, 1584451, 1588182, 1591912, 1595641, 1599368, 1603094, 1606820    //150 -- 159

};


/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
/*根据PT1000获取温度值*/
static void get_temp_by_PT1000(void)
{
    rt_uint8_t salve_temp[FCM_PT_TEMP_CHANNEL_NUM] = {0};

    rt_uint8_t channel;
    rt_uint16_t offsetpos;
    rt_uint32_t tem_data;
    float maxvol_offset = 0.05;
    for( channel=0;channel<FCM_PT_TEMP_CHANNEL_NUM;channel++ )
    {
        /*获取多通道ADC值*/
        g_temp_value = get_adc_value(channel, ADC_SAMPAL_COUNTS);

        g_temp_value = g_temp_value * MCU_VERF_VOLTAGE/4096;

        if(g_temp_value >= BOARD_FETCH_BASE_VOLTAGE)
        {
            g_temp_value = BOARD_FETCH_BASE_VOLTAGE - maxvol_offset;
        }
        //电阻值 这里的1000 是电阻值1K，
        g_temp_value = g_temp_value*BOARD_FETCH_R_VALUE/(BOARD_FETCH_BASE_VOLTAGE - g_temp_value);
        tem_data = g_temp_value*1000;

#if(0)
        //二分法计算
        e=(sizeof(m_rt_table)/sizeof(UINT32))-1;

        while((e-s)>1)
        {
            offsetpos=(s+e)/2;
            if(recvie_data<=m_rt_table[offsetpos])
            {
                s=offsetpos;
            }
            else
            {
                e=offsetpos;
            }
    }
#else
        // 查表法计算
      for ( offsetpos = 0; offsetpos < 210; offsetpos ++ )
      {
          if ( m_pt1000_table[offsetpos] > tem_data )
          {
               break;
          }
      }

#endif
      //避免越界,大于最大值按最大值处理.
      if(offsetpos > 210)
      {
          offsetpos = 210;
      }
      //offsetpos -= BOARD_FETCH_TEMP_OFFSET;
      salve_temp[channel] = (rt_int8_t)offsetpos;
      rt_thread_delay(10);
    }
    g_get_ad_para.temp_outiol = salve_temp[0];
    g_get_ad_para.temp_inoil = salve_temp[1];
}
/*根据压力传感器获得压力*/
static void get_press_by_sen(void)
{
    rt_uint32_t salve_press;
    /*获取压力传感器AD值*/
    g_press_value = get_adc_value(FCM_PRES_SENSOR_CHANNEL,ADC_SAMPAL_COUNTS);

    g_press_value = g_press_value * MCU_VERF_VOLTAGE/4096;

    g_press_value = g_press_value * 2;

    salve_press = g_press_value * 10 * 2;      //0~5V电压对应0~10bar压力范围,精度为10。所以乘10,乘2

    g_get_ad_para.oil_press = (UINT8)salve_press;
}
/*更新数据到TIU*/
static void update_data_to_tiu(void)
{
    TERM_COMM_DATA * tiu_para;

    tiu_para = task_tiu_para_list_get();

    tiu_para->fcm_remote_measure.input_temp = OIL_IN_TEMP;
    tiu_para->fcm_remote_measure.output_temp = OIL_OUT_TEMP;
    tiu_para->fcm_remote_measure.press_val = g_get_ad_para.oil_press;
	
    //tiu_para->fcm_remote_measure.fun_pwm_speed = g_pwm_para[PWM_PORT_FUN].cur_pwm_duty;
    //tiu_para->fcm_remote_measure.bump_pwm_speed = g_pwm_para[PWM_PORT_BUMP].cur_pwm_duty;
    tiu_para->fcm_remote_measure.fun_pwm_speed = fun_speed_t;
    tiu_para->fcm_remote_measure.bump_pwm_speed = bump_speed_t;
	
    tiu_para->fcm_remote_measure.fluid_level = g_get_ad_para.liquid_level;
	
    //tiu_para->fcm_remote_measure.fun_speed = fun_speed_t;
    //tiu_para->fcm_remote_measure.bump_speed = bump_speed_t;
    tiu_para->fcm_remote_measure.fun_speed = g_pwm_para[PWM_PORT_FUN].cur_pwm_duty;
    tiu_para->fcm_remote_measure.bump_speed = g_pwm_para[PWM_PORT_BUMP].cur_pwm_duty;
	
}
void get_status_task(void *parameter)
{
    while(1)
    {   /*温度采集*/
        get_temp_by_PT1000();
        /*压力采集*/
        get_press_by_sen();
        /*液位信息*/
        g_get_ad_para.liquid_level = get_io_state(DIGITAL_KIN1);
		/*风扇转速反馈*/
        ProcessIcValue(&fun_speed_t, &bump_speed_t);
        /*更新遥测信息*/
        update_data_to_tiu();
        rt_thread_mdelay(50);
    }
}
