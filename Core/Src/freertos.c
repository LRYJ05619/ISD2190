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
#include <string.h>
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
/* USER CODE BEGIN Variables */
extern QueueHandle_t usart2Queue;
extern QueueHandle_t usart3Queue;
extern QueueHandle_t usart5Queue;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void VM1_Receive_Task(const void *pvParameters);
void VM2_Receive_Task(const void *pvParameters);
void BLE_Receive_Task(const void *pvParameters);
void VM_Init_Task(const void *argument);
void Data_Collect_Task(const void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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


    usart2Queue = xQueueCreate(128, sizeof(uint8_t));
    usart3Queue = xQueueCreate(128, sizeof(uint8_t));
    usart5Queue = xQueueCreate(128, sizeof(uint8_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    osThreadDef(VM1_Receive_Task,VM1_Receive_Task,osPriorityHigh,0,128);
    osThreadDef(VM2_Receive_Task,VM2_Receive_Task,osPriorityHigh,0,128);
    osThreadDef(BLE_Receive_Task,BLE_Receive_Task,osPriorityHigh,0,128);

    osThreadDef(VM_Init_Task,VM_Init_Task,osPriorityHigh,0,128);
    osThreadDef(Data_Collect_Task,Data_Collect_Task,osPriorityRealtime,0,128);

    osThreadCreate(osThread(VM1_Receive_Task), NULL);
    osThreadCreate(osThread(VM2_Receive_Task), NULL);
    osThreadCreate(osThread(BLE_Receive_Task), NULL);

    osThreadCreate(osThread(VM_Init_Task), NULL);
    osThreadCreate(osThread(Data_Collect_Task), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
extern volatile u8 VM1_Init;
extern volatile u8 VM2_Init;
extern volatile u8 VM1_Busy;
extern volatile u8 VM2_Busy;
extern volatile u8 VM1_OK;
extern volatile u8 VM2_OK;
extern volatile u8 VM_ERR;

extern volatile u8 ble_len;
extern volatile u8 BleBuf[MAX_DATA_LENGTH];
extern volatile u8 ble_flag;

extern SensorInfo Sensor[16];

extern volatile u8 Scan_Start;
extern volatile u8 Cmd;

//VM接收
void VM1_Receive_Task(const void *pvParameters) {
    u8 rxdata;
    u8 lastbuf;
    u8 rx_index;
    u8 receiving;
    u8 rx_check;
    u8 rx_buffer[MAX_DATA_LENGTH];

    while (1) {
        if (xQueueReceive(usart2Queue, &rxdata, portMAX_DELAY)) {
            // 处理接收到的数据
            if ((0xBB == rxdata && 0xAA == lastbuf) || (0xAA == rxdata && 0xAA == lastbuf)) {
                receiving = 1;
                rx_check += 0xAA;
                rx_buffer[rx_index++] = 0xAA;
            }
            if (receiving) {
                rx_buffer[rx_index++] = rxdata;
                if (rx_check == rxdata) {
                    if (0xAA != rxdata) {
                        if (0x05 == rx_buffer[3]) {
                            VM1_Init = 1;
                        }
                        if (0x73 == rx_buffer[3]) {
                            u8 sensor_indices[] = {3, 7, 4, 8, 2, 6, 5, 1}; // 传感器索引数组
                            u8 index = 0; // 初始化索引

                            for (u8 i = 4; i < 20; i += 2) {
                                Sensor[sensor_indices[index]].freq[0] = ((uint16_t)rx_buffer[i] << 8) | rx_buffer[i + 1];
                                index++;
                            }
                            VM1_OK = 1;
                        }
                        rx_index = 0;
                        receiving = 0;
                        rx_check = 0;
                        VM1_Busy = 0;
                        memset(rx_buffer, 0, MAX_DATA_LENGTH);
                        HAL_UART_Receive_IT(&huart2, &rxdata, 1);
                        return;
                    }
                }
                rx_check += rxdata;
            }
            lastbuf = rxdata;
            HAL_UART_Receive_IT(&huart2, &rxdata, 1);
        }
        osDelay(50);
    }
}
void VM2_Receive_Task(const void *pvParameters) {
    u8 rxdata;
    u8 lastbuf;
    u8 rx_index;
    u8 receiving;
    u8 rx_check;
    u8 rx_buffer[MAX_DATA_LENGTH];

    while (1) {
        if (xQueueReceive(usart3Queue, &rxdata, portMAX_DELAY)) {
            // 处理接收到的数据
            if ((0xBB == rxdata && 0xAA == lastbuf) || (0xAA == rxdata && 0xAA == lastbuf)) {
                receiving = 1;
                rx_check += 0xAA;
                rx_buffer[rx_index++] = 0xAA;
            }
            if (receiving) {
                rx_buffer[rx_index++] = rxdata;
                if (rx_check == rxdata) {
                    if (0xAA != rxdata) {
                        if (0x05 == rx_buffer[3]) {
                            VM2_Init = 1;
                            memset(rx_buffer, 0, MAX_DATA_LENGTH);
                        }
                        if (0x73 == rx_buffer[3]) {
                            u8 sensor_indices[] = {13, 9, 10, 14, 12, 16, 11, 15}; // 传感器索引数组
                            u8 index = 0; // 初始化索引

                            for (u8 i = 4; i < 20; i += 2) {
                                Sensor[sensor_indices[index]].freq[0] = ((uint16_t)rx_buffer[i] << 8) | rx_buffer[i + 1];
                                index++;
                            }
                            VM2_OK = 1;
                        }
                        rx_index = 0;
                        receiving = 0;
                        rx_check = 0;
                        VM2_Busy = 0;
                        memset(rx_buffer, 0, MAX_DATA_LENGTH);
                        HAL_UART_Receive_IT(&huart3, &rxdata, 1);
                        return;
                    }
                }
                rx_check += rxdata;
            }
            lastbuf = rxdata;
            HAL_UART_Receive_IT(&huart3, &rxdata, 1);
        }
        osDelay(50);
    }
}
//蓝牙接收
void BLE_Receive_Task(const void *pvParameters) {
    uint8_t rxdata;
    while (1) {
        if (xQueueReceive(usart5Queue, &rxdata, portMAX_DELAY)) {
            // 处理接收到的数据
            BleBuf[ble_len++] = rxdata;
            HAL_TIM_Base_Stop_IT(&htim3); // 计数清零 重启定时器
            __HAL_TIM_SET_COUNTER(&htim3, 0);
            HAL_TIM_Base_Start_IT(&htim3);
            HAL_UART_Receive_IT(&huart5, &rxdata, 1);
        }

        osDelay(50);
    }
}
//VM初始化
void VM_Init_Task(const void *argument){
    while (1){
        if (!VM1_Init) {
            if (VM1_Busy) {
                HAL_TIM_Base_Start_IT(&htim2);
                continue;
            }
            VM1_Busy = 1;
            HAL_TIM_Base_Start_IT(&htim2);
            Init_VM(huart3);
        }
        if (!VM2_Init) {
            if (VM2_Busy) {
                HAL_TIM_Base_Start_IT(&htim3);
                continue;
            }
            VM2_Busy = 1;
            HAL_TIM_Base_Start_IT(&htim3);
            Init_VM(huart3);
        }
        if (VM_ERR && VM1_Init && VM2_Init) {
            VM_ERR = 0;
            StatuCallback(Cmd, 0x13);
        }

        osDelay(50);
    }
}
//数据采集与指令处理
void Data_Collect_Task(const void *argument){
    while (1){
        if (ble_flag) {
            ble_flag = 0;
            BleProcess();
        }

        if (VM1_Init && VM2_Init && !VM1_Busy && !VM2_Busy && Scan_Start) {
            Data_Collect();

            if(VM_ERR){
                continue;
            }

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
<<<<<<< Updated upstream
=======
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

                u8 ack_received = 0;
                u8 time = 0;

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
                        Verify_VM(huart2);
                        time++;
                    }
                    if(0x03 == rx_buffer[1] && 0x02 == rx_buffer[2]) {
                        time++;
                        // 等待设备状态寄存器的确认
                        if (rx_buffer[4] & (1 << 4)) {
                            // bit4为1，说明测量完成
                            ack_received = 1;
                            time = 0;
                            Read_VM(huart2);
                        }
                        // bit15为1，说明无设备
                        if ((rx_buffer[3] & (1 << 7)) && time > 17) {

                            for (u8 j = 0; j < 8; j++) {
                                Sensor[j].freq[0] = 0;
                                Sensor[j].freq_status = 1;
                            }
                            Clear_VM(huart2);
                            VM1_OK = 1;
                            VM1_Busy = 0;
                            time = 0;
                            ack_received = 1;
                        }
                        if (!ack_received) {
                            // 每隔3秒发送一次命令以获取设备状态
                            osDelay(pdMS_TO_TICKS(3000));
                            Verify_VM(huart2);
                        }
                    }
                    if (0x03 == rx_buffer[1] && 0x10 == rx_buffer[2]) {
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
                    osDelay(1);
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

                u8 ack_received = 0;
                u8 time = 0;

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
                        Verify_VM(huart3);
                        time++;
                    }
                    if(0x03 == rx_buffer[1] && 0x02 == rx_buffer[2]) {
                        time++;
                        // 等待设备状态寄存器的确认
                        if (rx_buffer[4] & (1 << 4)) {
                            // bit4为1，说明测量完成
                            ack_received = 1;
                            time = 0;
                            Read_VM(huart3);
                        }
                        // bit15为1，说明无设备
                        if ((rx_buffer[3] & (1 << 7)) && time > 17) {
                            for (u8 j = 0; j < 8; j++) {
                                Sensor[j + 8].freq[0] = 0;
                                Sensor[j].freq_status = 1;
                            }
                            Clear_VM(huart3);
                            VM2_OK = 1;
                            VM2_Busy = 0;
                            time = 0;
                            ack_received = 1;
                        }
                        if (!ack_received) {
                            // 每隔3秒发送一次命令以获取设备状态
                            osDelay(pdMS_TO_TICKS(3000));
                            Verify_VM(huart3);
                        }
                    }
                    if (0x03 == rx_buffer[1] && 0x10 == rx_buffer[2]) {
                        u8 sensor_indices[] = {12, 8, 9, 13, 11, 15, 10, 14}; // 传感器索引数组
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
                        Clear_VM(huart3);
                        VM2_OK = 1;
                        VM2_Busy = 0;
                    }
                    osDelay(1);
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
>>>>>>> Stashed changes
}


/* USER CODE END Application */

