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
extern volatile u8 BleBuf[VM_BLE_RX_BUFFER_SIZE];
extern volatile u8 ble_flag;

extern volatile SensorInfo Sensor[16];

extern volatile u8 Scan_Start;
extern volatile u8 Cmd;

extern u8 Uart2Buf[VM_BLE_RX_BUFFER_SIZE];
extern u8 Uart3Buf[VM_BLE_RX_BUFFER_SIZE];
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
unsigned int crc16(unsigned char *dat, unsigned int len);

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

        if (VM1_Init && VM2_Init && !VM1_Busy && !VM2_Busy && Scan_Start) {
            Data_Collect();

            //蓝牙指令处理
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
    u8 rx_buffer[VM_BLE_RX_BUFFER_SIZE];
    u8 Size;
    /* Infinite loop */
    for (;;) {
        if (xQueueReceive(usart2Queue, &Size, portMAX_DELAY)) {
            // 处理接收到的数据
            memcpy(rx_buffer, Uart2Buf, Size);

            if (Size >= 3) {
                //crc16校验
                u16 received_check = crc16(rx_buffer, Size - 2);
                u16 crc_check = (rx_buffer[Size - 1] << 8) + rx_buffer[Size - 2];

                if (received_check == crc_check) {
                    // 数据包校验通过
                    if (0x05 == rx_buffer[3] && 0x06 == rx_buffer[1]) {
                        VM1_Init = 1;
                        memset(rx_buffer, 0, VM_BLE_RX_BUFFER_SIZE);

                        __HAL_TIM_SET_COUNTER(&htim2, 0);
                        HAL_TIM_Base_Stop_IT(&htim2);

                        VM1_OK = 1;
                        VM1_Busy = 0;
                    }
                    if (0x03 == rx_buffer[3] && 0x06 == rx_buffer[1]) {
                        u8 ack_received = 0;
                        u8 rx[VM_BLE_RX_BUFFER_SIZE];
                        u8 time = 0;
                        // 每隔3秒发送一次命令以获取设备状态
                        while (!ack_received) {
                            // 发送命令后等待3秒再重新发送
                            osDelay(pdMS_TO_TICKS(5000));
                            Verify_VM(huart2);
                            time++;
                            // 等待设备状态寄存器的确认
                            if (xQueueReceive(usart2Queue, &Size, pdMS_TO_TICKS(500))) {
                                memcpy(rx, Uart2Buf, Size);
                                if (rx[4] & (1 << 4)) {
                                    // bit4为1，说明测量完成
                                    ack_received = 1;
                                    Read_VM(huart2);
                                }
                                // bit15为1，说明无设备
                                if ((rx[3] & (1 << 7)) && time > 5) {
                                    u8 index = 0; // 初始化索引
                                    for (u8 j = 0; j < 8; j ++) {
                                        Sensor[j].freq[0] = 0;
                                        Sensor[j].freq_status = 1;
                                        index++;
                                    }
                                    Clear_VM(huart2);
                                    VM1_OK = 1;
                                    VM1_Busy = 0;
                                    ack_received = 1;
                                }
                            }
                        }
                    }
                    if (0x03 == rx_buffer[1]) {
                        u8 sensor_indices[] = {2, 6, 3, 7, 1, 5, 4, 0}; // 传感器索引数组
                        u8 index = 0; // 初始化索引

                        for (u8 i = 3; i < 19; i += 2) {
                            Sensor[sensor_indices[index]].freq[0] =
                                    ((uint16_t) rx_buffer[i] << 8) | rx_buffer[i + 1];
                            if(!Sensor[sensor_indices[index]].freq[0]){
                                Sensor[sensor_indices[index]].freq_status = 1;
                            } else{
                                Sensor[sensor_indices[index]].freq_status = 0;
                            }
                            index++;
                        }
                        Clear_VM(huart2);
                        VM1_OK = 1;
                        VM1_Busy = 0;
                    }
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
    u8 rx_buffer[VM_BLE_RX_BUFFER_SIZE];
    u8 Size;
    /* Infinite loop */
    for (;;) {
        if (xQueueReceive(usart3Queue, &Size, portMAX_DELAY)) {
            // 处理接收到的数据
            memcpy(rx_buffer, Uart3Buf, Size);

            if (Size >= 3) {
                //crc16校验
                u16 received_check = crc16(rx_buffer, Size - 2);
                u16 crc_check = (rx_buffer[Size - 1] << 8) + rx_buffer[Size - 2];

                if (crc_check == received_check) {
                    // 数据包校验通过
                    if (0x05 == rx_buffer[3] && 0x06 == rx_buffer[1]) {
                        VM2_Init = 1;
                        memset(rx_buffer, 0, VM_BLE_RX_BUFFER_SIZE);

                        __HAL_TIM_SET_COUNTER(&htim2, 0);
                        HAL_TIM_Base_Stop_IT(&htim2);

                        VM2_OK = 1;
                        VM2_Busy = 0;
                    }
                    if (0x03 == rx_buffer[3] && 0x06 == rx_buffer[1]) {
                        u8 ack_received = 0;
                        u8 rx[VM_BLE_RX_BUFFER_SIZE];
                        u8 time = 0;
                        // 每隔5秒发送一次命令以获取设备状态
                        while (!ack_received) {
                            // 发送命令后等待5秒再重新发送
                            osDelay(pdMS_TO_TICKS(5000));
                            Verify_VM(huart3);
                            time++;
                            // 等待设备状态寄存器的确认
                            if (xQueueReceive(usart3Queue, &Size, pdMS_TO_TICKS(500))) {
                                memcpy(rx, Uart3Buf, Size);
                                if (rx[4] & (1 << 4)) {
                                    // bit4为1，说明测量完成
                                    ack_received = 1;
                                    Read_VM(huart3);
                                }
                                // bit15为1，说明无设备
                                if ((rx[3] & (1 << 7)) && time > 5) {
                                    u8 index = 0; // 初始化索引

                                    for (u8 j = 0; j < 8; j ++) {
                                        Sensor[j + 8].freq[0] = 0;
                                        Sensor[j].freq_status = 1;
                                        index++;
                                    }
                                    Clear_VM(huart3);
                                    VM2_OK = 1;
                                    VM2_Busy = 0;
                                    ack_received = 1;
                                }
                            }
                        }
                    }
                    if (0x03 == rx_buffer[1]) {
                        u8 sensor_indices[] = {12, 8, 9, 13, 11, 15, 10, 14}; // 传感器索引数组
                        u8 index = 0; // 初始化索引

                        for (u8 i = 3; i < 19; i += 1) {
                            Sensor[sensor_indices[index]].freq[0] =
                                    ((uint16_t) rx_buffer[i] << 8) | rx_buffer[i + 1];
                            if(!Sensor[sensor_indices[index]].freq[0]){
                                Sensor[sensor_indices[index]].freq_status = 1;
                            } else{
                                Sensor[sensor_indices[index]].freq_status = 0;
                            }
                            index++;
                        }
                        Clear_VM(huart3);
                        VM2_OK = 1;
                        VM2_Busy = 0;
                    }
                }
            }
        }
        osDelay(1);
    }
  /* USER CODE END VM2_Receive_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
unsigned int crc16(unsigned char *dat, unsigned int len) {
    unsigned int crc = 0xffff;
    unsigned char i;
    while (len != 0) {
        crc ^= *dat;
        for (i = 0; i < 8; i++) {
            if ((crc & 0x0001) == 0)
                crc = crc >> 1;
            else {
                crc = crc >> 1;
                crc ^= 0xa001;
            }
        }
        len -= 1;
        dat++;
    }
    return crc;
}


/* USER CODE END Application */

