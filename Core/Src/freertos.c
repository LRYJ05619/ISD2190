/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "tim.h"
#include "VMxx.h"
#include "CollectData.h"
#include "bt4531.h"
#include "queue.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern QueueHandle_t usart2Queue;
extern QueueHandle_t usart3Queue;

extern volatile u8 VM1_Init;
extern volatile u8 VM2_Init;
extern volatile u8 VM1_Busy;
extern volatile u8 VM2_Busy;
extern volatile u8 VM1_OK;
extern volatile u8 VM2_OK;
extern volatile u8 VM_ERR;

extern volatile u8 ble_len;
extern volatile u8 BleBuf[RX_BUFFER_SIZE];
extern volatile u8 ble_flag;

extern volatile SensorInfo Sensor[16];

extern volatile u8 Scan_Start;
extern volatile u8 Cmd;

extern volatile u8 Uart2Buf[RX_BUFFER_SIZE];
extern volatile u8 Uart3Buf[RX_BUFFER_SIZE];
/* USER CODE END Variables */
/* Definitions for DataCollectTask */
osThreadId_t DataCollectTaskHandle;
uint32_t DataCollectTaskBuffer[ 512 ];
osStaticThreadDef_t DataCollectTaskControlBlock;
const osThreadAttr_t DataCollectTask_attributes = {
  .name = "DataCollectTask",
  .cb_mem = &DataCollectTaskControlBlock,
  .cb_size = sizeof(DataCollectTaskControlBlock),
  .stack_mem = &DataCollectTaskBuffer[0],
  .stack_size = sizeof(DataCollectTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for VM1ReceiveTask */
osThreadId_t VM1ReceiveTaskHandle;
uint32_t VM1ReceiveTaskBuffer[ 128 ];
osStaticThreadDef_t VM1ReceiveTaskControlBlock;
const osThreadAttr_t VM1ReceiveTask_attributes = {
  .name = "VM1ReceiveTask",
  .cb_mem = &VM1ReceiveTaskControlBlock,
  .cb_size = sizeof(VM1ReceiveTaskControlBlock),
  .stack_mem = &VM1ReceiveTaskBuffer[0],
  .stack_size = sizeof(VM1ReceiveTaskBuffer),
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for VM2ReceiveTask */
osThreadId_t VM2ReceiveTaskHandle;
uint32_t VM2ReceiveTaskBuffer[ 128 ];
osStaticThreadDef_t VM2ReceiveTaskControlBlock;
const osThreadAttr_t VM2ReceiveTask_attributes = {
  .name = "VM2ReceiveTask",
  .cb_mem = &VM2ReceiveTaskControlBlock,
  .cb_size = sizeof(VM2ReceiveTaskControlBlock),
  .stack_mem = &VM2ReceiveTaskBuffer[0],
  .stack_size = sizeof(VM2ReceiveTaskBuffer),
  .priority = (osPriority_t) osPriorityRealtime,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Data_Collect_Task(void *argument);
void VM1_Receive_Task(void *argument);
void VM2_Receive_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    usart2Queue = xQueueCreate(8, sizeof(uint8_t));
    usart3Queue = xQueueCreate(8, sizeof(uint8_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DataCollectTask */
  DataCollectTaskHandle = osThreadNew(Data_Collect_Task, NULL, &DataCollectTask_attributes);

  /* creation of VM1ReceiveTask */
  VM1ReceiveTaskHandle = osThreadNew(VM1_Receive_Task, NULL, &VM1ReceiveTask_attributes);

  /* creation of VM2ReceiveTask */
  VM2ReceiveTaskHandle = osThreadNew(VM2_Receive_Task, NULL, &VM2ReceiveTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Data_Collect_Task */
uint8_t calculate_checksum(uint8_t *data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}
/**
  * @brief  Function implementing the DataCollectTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Data_Collect_Task */
void Data_Collect_Task(void *argument)
{
  /* USER CODE BEGIN Data_Collect_Task */
    /* Infinite loop */
    for (;;) {
        if (!VM1_Init) {
            if (VM1_Busy) {
                HAL_TIM_Base_Start_IT(&htim2);
                continue;
            }
            VM1_OK = 0;
            VM1_Busy = 1;
            HAL_TIM_Base_Start_IT(&htim2);
            Init_VM(huart2);
        }
        if (!VM2_Init) {
            if (VM2_Busy) {
                HAL_TIM_Base_Start_IT(&htim2);
                continue;
            }
            VM2_OK = 0;
            VM2_Busy = 1;
            HAL_TIM_Base_Start_IT(&htim2);
            Init_VM(huart3);
        }

        if (VM_ERR && VM1_Init && VM2_Init) {
            VM_ERR = 0;
            StatuCallback(Cmd, 0x13);
        }
        
        if (ble_flag) {
            ble_flag = 0;
            BleProcess();
        }

        if (VM1_Init && VM2_Init && !VM1_Busy && !VM2_Busy && Scan_Start){
            Data_Collect();

            //蓝牙指令处理
            if (Cmd == 0x70)
                DataSend(BleBuf[4]);
            if (Cmd == 0x71)
                TotalDataSend();
            if (Cmd == 0x40)
                ConfigInit();

            Scan_Start = 0;
        }
        osDelay(50);
    }
  /* USER CODE END Data_Collect_Task */
}

/* USER CODE BEGIN Header_VM1_Receive_Task */
/**
* @brief Function implementing the VM1ReceiveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_VM1_Receive_Task */
void VM1_Receive_Task(void *argument)
{
  /* USER CODE BEGIN VM1_Receive_Task */
    u8 rx_buffer[RX_BUFFER_SIZE];
    u8 Size;
    u8 received_checksum;
    /* Infinite loop */
    for (;;) {
        if (xQueueReceive(usart2Queue, &Size, portMAX_DELAY)) {
            // 处理接收到的数据
            memcpy(rx_buffer, Uart2Buf, Size);

            if (Size >= 3)
            {
                received_checksum = rx_buffer[Size - 1];

                // 计算和校验
                u8 calculated_checksum = calculate_checksum(rx_buffer, Size - 1);

                if (calculated_checksum == received_checksum)
                {
                    // 数据包校验通过
                    if (0x05 == rx_buffer[3]) {
                        VM1_Init = 1;
                        memset(rx_buffer, 0, RX_BUFFER_SIZE);

                        __HAL_TIM_SET_COUNTER(&htim2, 0);
                        HAL_TIM_Base_Stop_IT(&htim2);
                    }
                    if (0x73 == rx_buffer[3]) {
                        u8 sensor_indices[] = {2, 6, 3, 7, 1, 5, 4, 0}; // 传感器索引数组
                        u8 index = 0; // 初始化索引

                        for (u8 i = 4; i < 20; i += 2) {
                            Sensor[sensor_indices[index]].freq[0] =
                                    ((uint16_t) rx_buffer[i] << 8) | rx_buffer[i + 1];
                            index++;
                        }
                    }
                    VM1_OK = 1;
                    VM1_Busy = 0;
                }
            }
        }
        osDelay(1);
    }
  /* USER CODE END VM1_Receive_Task */
}

/* USER CODE BEGIN Header_VM2_Receive_Task */
/**
* @brief Function implementing the VM2ReceiveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_VM2_Receive_Task */
void VM2_Receive_Task(void *argument)
{
  /* USER CODE BEGIN VM2_Receive_Task */
    u8 rx_buffer[RX_BUFFER_SIZE];
    u8 Size;
    u8 received_checksum;
    /* Infinite loop */
    for (;;) {
        if (xQueueReceive(usart3Queue, &Size, portMAX_DELAY)) {
            // 处理接收到的数据
            // 处理接收到的数据
            memcpy(rx_buffer, Uart3Buf, Size);

            if (Size >= 3)
            {
                received_checksum = rx_buffer[Size - 1];

                // 计算和校验
                u8 calculated_checksum = calculate_checksum(rx_buffer, Size - 1);

                if (calculated_checksum == received_checksum)
                {
                    // 数据包校验通过
                    if (0x05 == rx_buffer[3]) {
                        VM2_Init = 1;
                        memset(rx_buffer, 0, RX_BUFFER_SIZE);

                        __HAL_TIM_SET_COUNTER(&htim2, 0);
                        HAL_TIM_Base_Stop_IT(&htim2);
                    }
                    if (0x73 == rx_buffer[3]) {
                        u8 sensor_indices[] = {12, 8, 9, 13, 11, 15, 10, 14}; // 传感器索引数组
                        u8 index = 0; // 初始化索引

                        for (u8 i = 4; i < 20; i += 2) {
                            Sensor[sensor_indices[index]].freq[0] =
                                    ((uint16_t) rx_buffer[i] << 8) | rx_buffer[i + 1];
                            index++;
                        }
                    }
                    VM2_OK = 1;
                    VM2_Busy = 0;
                }
            }
        }
        osDelay(1);
    }
  /* USER CODE END VM2_Receive_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/* USER CODE END Application */

