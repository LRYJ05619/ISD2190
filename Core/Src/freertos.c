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
void USART2_Receive_Task(const void *pvParameters);
void USART3_Receive_Task(const void *pvParameters);
void USART5_Receive_Task(const void *pvParameters);
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
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    osThreadDef(USART2_Receive_Task,USART2_Receive_Task,osPriorityHigh,0,128);
    osThreadDef(USART3_Receive_Task,USART3_Receive_Task,osPriorityHigh,0,128);
    osThreadDef(USART5_Receive_Task,USART5_Receive_Task,osPriorityHigh,0,128);
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


void USART2_Receive_Task(const void *pvParameters) {
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
                            VM_init = 1;
                            memset(rx_buffer, 0, MAX_DATA_LENGTH);
                        }
                        if (0x73 == rx_buffer[3]) {

                        }
                        rx_index = 0;
                        receiving = 0;
                        rx_check = 0;
                        VM_Busy = 0;
                        HAL_UART_Receive_IT(&huart3, &rxdata, 1);
                        HAL_UART_Receive_IT(&huart2, &rxdata, 1);
                        HAL_TIM_Base_Stop_IT(&htim2);
                        __HAL_TIM_SET_COUNTER(&htim2, 0);
                        return;
                    }
                }
                rx_check += rxdata;
        }
    }
}
void USART3_Receive_Task(const void *pvParameters) {
    uint8_t received_char;
    while (1) {
        if (xQueueReceive(usart3Queue, &received_char, portMAX_DELAY)) {
            // 处理接收到的数据
        }
    }
}
void USART5_Receive_Task(const void *pvParameters) {
    uint8_t received_char;
    while (1) {
        if (xQueueReceive(usart3Queue, &received_char, portMAX_DELAY)) {
            // 处理接收到的数据
        }
    }
}
/* USER CODE END Application */

