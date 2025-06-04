/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "flash.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile u8 Scan_Start; //置1开始扫描

volatile u8 ble_flag;    //蓝牙指令接收
volatile u8 ble_len;
u8 BleBuf[BLE_CONFIG_BUFFER_SIZE];
volatile u8 Cmd;              //蓝牙指令

u8 Uart2Buf[VM_BLE_RX_BUFFER_SIZE];
u8 Uart3Buf[VM_BLE_RX_BUFFER_SIZE];

volatile u8 VM1_Init;    //VM1初始化
volatile u8 VM2_Init;
volatile u8 VM1_Busy;    //VM1工作中
volatile u8 VM2_Busy;
volatile u8 VM1_OK;      //VM1读取成功
volatile u8 VM2_OK;
volatile u8 VM_ERR;      //模块异常，尝试初始化


u16 ADC_Value[ADC_CHANCEL_NUM]; //adc采集值
int16_t Temp_Value[ADC_CHANCEL_NUM]; //实际温度

SensorInfo Sensor[16];   //数据结构体

QueueHandle_t usart2Queue;
QueueHandle_t usart3Queue;
//数采通道 -> 振弦通道 -> adc通道 对照
// 1  -> 8  -> 10 ;   2  -> 5  -> 12 ;   3  -> 1  -> 0  ;  4  -> 3  -> 4  ;
// 5  -> 7  -> 11 ;   6  -> 6  -> 13 ;   7  -> 2  -> 1  ;  8  -> 4  -> 5  ;
// 9  -> 10 -> 6  ;   10 -> 11 -> 8  ;   11 -> 15 -> 18 ;  12 -> 13 -> 20 ;
// 13 -> 9  -> 7  ;   14 -> 12 -> 14 ;   15 -> 16 -> 19 ;  16 -> 14 -> 21 ;

ConfigInfo Config;   //配置结构体，包括ip和id

/* USER CODE END Private defines */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_UART5_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, Uart2Buf, VM_BLE_RX_BUFFER_SIZE);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, Uart3Buf, VM_BLE_RX_BUFFER_SIZE);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, BleBuf, BLE_CONFIG_BUFFER_SIZE);
//    Flash_Erase();
    Flash_Read();

//    Config_Erase();
    Config_Read();

    //更改蓝牙名称
    printf("TTM:REN-IAD101\r\n\0");

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
    if (htim->Instance == TIM2) {
        HAL_TIM_Base_Stop_IT(&htim2);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        VM_ERR = 1;
        VM1_Busy = 0;
        VM2_Busy = 0;
        Scan_Start = 0;

        if(!VM1_OK)
            VM1_Init = 0;

        if(!VM2_OK)
            VM2_Init = 0;
    }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
