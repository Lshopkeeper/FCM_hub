/******************************************************************************,

                  版权所有 (C), 2009-2020, 青岛海汇德电气有限公司

 ******************************************************************************
  文 件 名   : fcm_temp.c
  版 本 号   : 初稿
  作    者   :LLXH
  生成日期   : 2022年1月11日
  最近修改   :
  功能描述   : FCM adc采集任务
  函数列表   :

  修改历史   :
  1.日    期   : 2022年1月11日
    作    者   : LLXH
    修改内容   : 创建文件

******************************************************************************/
#define DBG_TAG "fcm_adc"
#define DBG_LVL LOG_LVL_DBG
#include <rtdbg.h>

/*----------------------------------------------*
 * 包含头文件                                        *
 *----------------------------------------------*/
#include "fcm_adc.h"
/*----------------------------------------------*
 * 枚举类型定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 结构体定义                                        *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部变量原型说明（可供外部使用）                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明（可供外部使用）                             *
 *----------------------------------------------*/
#if(1)
ADC_HandleTypeDef hadc1;

static rt_uint32_t ADC_Sample_Value[FCM_ADC_CHANNEL_NUM] = {0};
/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void adc_init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;          /*扫描转换模式*/
  hadc1.Init.ContinuousConvMode = DISABLE;             /*连续转换*/
  hadc1.Init.DiscontinuousConvMode = ENABLE;         /*直接转换模式*/
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;         /*转换右对齐*/
  hadc1.Init.NbrOfConversion = 3;                     /*通道个数*/
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/*ADC校准，不然读取数据不准确*/
void ADC_Calibration(void)
{
    HAL_ADCEx_Calibration_Start(&hadc1);
}


/*AD采集*/
rt_uint32_t get_adc_value(rt_uint8_t Channel,rt_uint8_t average)
{
    rt_uint8_t i;
    rt_uint32_t SumVal = 0;
    for(i=0;i<FCM_ADC_CHANNEL_NUM;i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 50);
        ADC_Sample_Value[i] = HAL_ADC_GetValue(&hadc1);
    }
    for(i=0;i<average;i++)
    {
        SumVal += ADC_Sample_Value[Channel];
    }
    return SumVal/average;
}
#endif


