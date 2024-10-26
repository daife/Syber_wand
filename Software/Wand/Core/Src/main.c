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
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "W25Q16.h"
#include "weights.h"
#include "nnom.h"
#include "key.h"
#include "ir_decode.h"
#include "RGB.h"
#include "mpu6050.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern DMA_HandleTypeDef hdma_usart1_tx;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef NNOM_USING_STATIC_MEMORY
	uint8_t static_buf[1024 * 8];
#endif //NNOM_USING_STATIC_MEMORY

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
nnom_model_t* model;
short gx=0,gy=0,gz=0;
uint8_t data_feed_On=0;
uint8_t count=0;//feed count
short gyro[150][3]={0};
uint8_t received=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim4)
  {
    KEY_Scan();
  }
}
/*
 * @brief Quantirize IMU data and feed to CNN model
 * @param None
 * @return None
 */
void model_feed_data(void)
{
	const double scale = QUANTIFICATION_SCALE;
	uint16_t i = 0;
	for(i = 0; i < IMU_SEQUENCE_LENGTH_MAX;i++){
		nnom_input_data[i*3] = (int8_t)round((float)gyro[i][Roll]/IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
		nnom_input_data[i*3+1] = (int8_t)round((float)gyro[i][Pitch]/IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
		nnom_input_data[i*3+2] = (int8_t)round((float)gyro[i][Yaw]/IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
	}
}

int8_t model_get_output(void)
{
	volatile uint8_t i = 0;
	volatile int8_t max_output = -128;
	int8_t ret = 0;
	model_feed_data();
	model_run(model);
	for(i = 0; i < 13;i++){
		
		#ifdef SERIAL_DEBUG
		printf("Output[%d] = %.2f %%\n",i,(nnom_output_data[i] / 127.0)*100);
		#endif //SERIAL_DEBUG
		
		if(nnom_output_data[i] >= max_output){
			max_output = nnom_output_data[i] ;
			ret = i;
		}
	}
	if(max_output < OUPUT_THRESHOLD || ret == NoMotion){
		ret = Unrecognized;
	}
	//put into vitual fifo
	switch(ret){
		case Unrecognized:
			printf("Unrecognized");
			break;
		case RightAngle:
			printf("RightAngle");
		KEY_FIFO_Put(10+RightAngle);//+10 to match fifo enum
		
			break;
		case SharpAngle:
			printf("SharpAngle");
				KEY_FIFO_Put(10+SharpAngle);
			break;
		case Lightning:
			printf("Lightning");
				KEY_FIFO_Put(10+Lightning);
			break;
		case Triangle:
			printf("Triangle");
		KEY_FIFO_Put(10+Triangle);
			break;
		case Letter_h:
			printf("Letter_h");
				KEY_FIFO_Put(10+Letter_h);
			break;
		case letter_R:
			printf("Letter_R");
				KEY_FIFO_Put(10+letter_R);
			break;
		case letter_W:
			printf("Letter_W");
				KEY_FIFO_Put(10+letter_W);
			break;
		case letter_phi:
			printf("Letter_phi");
				KEY_FIFO_Put(10+letter_phi);
			break;
		case Circle:
			printf("Circle");
				KEY_FIFO_Put(10+Circle);
			break;
		case UpAndDown:
			printf("UpAndDown");
				KEY_FIFO_Put(10+UpAndDown);
			break;
		case Horn:
			printf("Horn");
				KEY_FIFO_Put(10+Horn);
			break;
		case Wave:
			printf("Wave");
				KEY_FIFO_Put(10+Wave);
			break;
		case NoMotion:
			printf("Unrecognized");
			break;
	}
	printf("\n");
	
	return ret;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)//10msä¸?æ¬?
		{
			//é™?èžºä»ª
			MPU_Get_Gyroscope(&gx,&gy,&gz);
			if(data_feed_On){
			gyro[count][Roll]=gx;
			gyro[count][Pitch]=gy;
				gyro[count][Yaw]=gz;
				if(++count==150){
					model_feed_data();
					data_feed_On=0;
					count=0;
					model_get_output();//recognize gesture and work
				}
				
			}
			
		}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart==&huart3){//from asrpro
		switch();
		received=0;
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

	//crate CNN model
	#ifdef NNOM_USING_STATIC_MEMORY
		nnom_set_static_buf(static_buf, sizeof(static_buf)); 
	#endif //NNOM_USING_STATIC_MEMORY
	
	MPU_Init();
	model = nnom_model_create();
	
KEY_Init();
HAL_TIM_Base_Start_IT(&htim3);

HAL_UART_Receive_IT(&huart2,&received,1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
