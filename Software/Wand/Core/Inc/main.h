/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern short gx,gy,gz;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EXIT0_6050_Pin GPIO_PIN_0
#define EXIT0_6050_GPIO_Port GPIOA
#define PA1_LED_Pin GPIO_PIN_1
#define PA1_LED_GPIO_Port GPIOA
#define USER_Button_Pin GPIO_PIN_0
#define USER_Button_GPIO_Port GPIOB
#define USER_Button2_Pin GPIO_PIN_1
#define USER_Button2_GPIO_Port GPIOB
#define W25Q16_CHIP_SELECT_Pin GPIO_PIN_12
#define W25Q16_CHIP_SELECT_GPIO_Port GPIOB
#define DIN_2812B_Pin GPIO_PIN_8
#define DIN_2812B_GPIO_Port GPIOA
#define IR_Pin GPIO_PIN_4
#define IR_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define QUANTIFICATION_SCALE (pow(2,INPUT_1_OUTPUT_DEC))
#define OUPUT_THRESHOLD 63 //The out put of model must bigger than this value unless the out put would be unrecognized.
#define IMU_SEQUENCE_LENGTH_MAX (150)

//+-500 to radian is divided by (73.537*180/PI) = 4213.359738
#define IMU_GYRO_TRANS_RADIAN_CONSTANT (4213.359738) 
typedef enum IMU_GYRO_Index{
	Roll = 0,
	Pitch = 1,
	Yaw = 2

}IMU_GYRO_Index;

typedef enum eModel_Output{
	Unrecognized = -1,
	RightAngle = 0,
	SharpAngle = 1,
	Lightning = 2,
	Triangle = 3,
	Letter_h = 4,
	letter_R = 5,
	letter_W = 6,
	letter_phi = 7,
	Circle = 8,
	UpAndDown = 9,
	Horn = 10,
	Wave = 11,
	NoMotion = 12
}eModel_Output;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
