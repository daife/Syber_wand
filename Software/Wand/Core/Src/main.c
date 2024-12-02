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
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern DMA_HandleTypeDef hdma_usart1_tx;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
RGB_Color_TypeDef RED = {255, 0, 0}; // 显示红色RGB数据
RGB_Color_TypeDef GREEN = {0, 255, 0};
RGB_Color_TypeDef BLUE = {0, 0, 255};
RGB_Color_TypeDef SKY = {0, 255, 255};
RGB_Color_TypeDef MAGENTA = {255, 0, 220};
RGB_Color_TypeDef YELLOW = {127, 216, 0};
RGB_Color_TypeDef OEANGE = {127, 106, 0};
RGB_Color_TypeDef BLACK = {0, 0, 0};
RGB_Color_TypeDef WHITE = {255, 255, 255};
#define QUANTIFICATION_SCALE (pow(2, INPUT_1_OUTPUT_DEC))
#define OUPUT_THRESHOLD 63 // The out put of model must bigger than this value unless the out put would be unrecognized.
#define IMU_SEQUENCE_LENGTH_MAX (150)
#define countReloadvalue 2000 //达到重载值重装并触发一次移动
#define flashbegin 10 //前面有坏块，使用后面的内存区域

#define xbias -250 //陀螺仪的零飘
#define zbias -80

#define mouseGain 6 //鼠标增益，类似dpi?

//+-500 to radian is divided by (73.537*180/PI) = 4213.359738
#define IMU_GYRO_TRANS_RADIAN_CONSTANT (4213.359738)
typedef enum IMU_GYRO_Index
{
	Roll = 0,
	Pitch = 1,
	Yaw = 2

} IMU_GYRO_Index;

typedef enum eModel_Output
{
	Unrecognized = -1,
	RightAngle,
	SharpAngle,
	Lightning,
	Triangle,
	Letter_h,
	letter_R,
	letter_W,
	letter_phi,
	Circle,
	UpAndDown,
	Horn,
	Wave,
	NoMotion
} eModel_Output;
#ifdef NNOM_USING_STATIC_MEMORY
uint8_t static_buf[1024 * 8];
#endif // NNOM_USING_STATIC_MEMORY

t_remote_ac_status ac_status;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

UINT8 aux1[] = {
	0x1D, 0x00, 0x00, 0x09, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0x18, 0x00, 0x1B, 0x00, 0xFF, 0xFF, 0x1C, 0x00, 0x24, 0x00, 0xFF, 0xFF, 0x40, 0x00, 0x5E, 0x00, 0xFF, 0xFF, 0x76, 0x00, 0xFF, 0xFF, 0x80, 0x00, 0xF8, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x38, 0x39, 0x37, 0x39, 0x2C, 0x34, 0x35, 0x32, 0x35, 0x35, 0x35, 0x30, 0x2C, 0x35, 0x35, 0x30, 0x35, 0x35, 0x30, 0x2C, 0x31, 0x36, 0x39, 0x33, 0x36, 0x35, 0x32, 0x31, 0x30, 0x30, 0x30, 0x32, 0x30, 0x39, 0x30, 0x30, 0x30, 0x44, 0x43, 0x33, 0x34, 0x30, 0x45, 0x30, 0x30, 0x30, 0x41, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x31, 0x43, 0x34, 0x30, 0x32, 0x30, 0x36, 0x32, 0x30, 0x30, 0x32, 0x30, 0x36, 0x38, 0x30, 0x30, 0x32, 0x30, 0x36, 0x30, 0x30, 0x30, 0x32, 0x30, 0x36, 0x43, 0x30, 0x30, 0x32, 0x30, 0x36, 0x34, 0x30, 0x30, 0x32, 0x30, 0x34, 0x41, 0x30, 0x30, 0x32, 0x30, 0x34, 0x36, 0x30, 0x30, 0x32, 0x30, 0x34, 0x34, 0x30, 0x30, 0x32, 0x30, 0x34, 0x32, 0x30, 0x30, 0x34, 0x30, 0x31, 0x30, 0x30, 0x30, 0x43, 0x30, 0x43, 0x30, 0x33, 0x30, 0x31, 0x30, 0x42, 0x30, 0x35, 0x30, 0x33, 0x30, 0x32, 0x30, 0x42, 0x30, 0x36, 0x30, 0x33, 0x30, 0x33, 0x30, 0x42, 0x30, 0x30, 0x30, 0x33, 0x30, 0x34, 0x30, 0x42, 0x30, 0x31, 0x30, 0x33, 0x30, 0x35, 0x30, 0x42, 0x30, 0x34, 0x30, 0x33, 0x30, 0x36, 0x30, 0x42, 0x30, 0x32, 0x30, 0x33, 0x30, 0x37, 0x30, 0x42, 0x30, 0x32, 0x30, 0x33, 0x30, 0x43, 0x30, 0x42, 0x31, 0x36, 0x30, 0x33, 0x30, 0x38, 0x30, 0x42, 0x30, 0x38, 0x30, 0x33, 0x31, 0x36, 0x30, 0x42, 0x30, 0x42, 0x30, 0x33, 0x38, 0x33, 0x30, 0x42, 0x31, 0x35, 0x30, 0x33, 0x31, 0x42, 0x30, 0x42, 0x31, 0x39, 0x30, 0x33, 0x31, 0x43, 0x30, 0x42, 0x31, 0x33, 0x30, 0x33, 0x30, 0x39, 0x30, 0x42, 0x30, 0x44, 0x30, 0x33, 0x30, 0x41, 0x30, 0x42, 0x30, 0x44, 0x30, 0x33, 0x30, 0x38, 0x30, 0x44, 0x30, 0x31, 0x30, 0x33, 0x30, 0x44, 0x31, 0x30, 0x30, 0x30, 0x30, 0x33, 0x30, 0x44, 0x31, 0x30, 0x30, 0x37};
UINT8 aux2[] = {
	0x1D, 0x00, 0x00, 0x09, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0x18, 0x00, 0x1B, 0x00, 0xFF, 0xFF, 0x1C, 0x00, 0x24, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x42, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x4C, 0x00, 0x54, 0x00, 0x8E, 0x00, 0xAE, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDE, 0x00, 0xDF, 0x00, 0xE4, 0x00, 0xED, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x33, 0x34, 0x34, 0x30, 0x2C, 0x31, 0x36, 0x37, 0x30, 0x35, 0x31, 0x30, 0x2C, 0x34, 0x32, 0x36, 0x35, 0x31, 0x30, 0x2C, 0x31, 0x33, 0x31, 0x30, 0x31, 0x33, 0x37, 0x31, 0x30, 0x30, 0x30, 0x32, 0x30, 0x35, 0x32, 0x30, 0x30, 0x45, 0x32, 0x33, 0x43, 0x42, 0x32, 0x36, 0x30, 0x31, 0x30, 0x30, 0x32, 0x34, 0x30, 0x33, 0x30, 0x46, 0x33, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x33, 0x30, 0x34, 0x30, 0x31, 0x30, 0x30, 0x30, 0x44, 0x30, 0x44, 0x30, 0x33, 0x33, 0x43, 0x34, 0x30, 0x30, 0x46, 0x30, 0x33, 0x33, 0x34, 0x33, 0x38, 0x30, 0x33, 0x30, 0x33, 0x33, 0x34, 0x33, 0x38, 0x30, 0x31, 0x30, 0x36, 0x33, 0x34, 0x33, 0x38, 0x30, 0x38, 0x33, 0x43, 0x33, 0x45, 0x30, 0x37, 0x30, 0x36, 0x33, 0x34, 0x33, 0x38, 0x30, 0x37, 0x33, 0x43, 0x33, 0x45, 0x30, 0x37, 0x30, 0x36, 0x33, 0x34, 0x33, 0x38, 0x30, 0x32, 0x33, 0x43, 0x33, 0x45, 0x30, 0x37, 0x30, 0x33, 0x34, 0x35, 0x34, 0x38, 0x30, 0x30, 0x30, 0x33, 0x34, 0x35, 0x34, 0x38, 0x30, 0x32, 0x30, 0x33, 0x34, 0x35, 0x34, 0x38, 0x30, 0x33, 0x30, 0x33, 0x34, 0x35, 0x34, 0x38, 0x30, 0x35, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x37, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x30, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x31, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x32, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x33, 0x30, 0x33, 0x34, 0x32, 0x34, 0x35, 0x30, 0x34, 0x54, 0x54, 0x7C, 0x53, 0x26, 0x30, 0x54, 0x7C, 0x53, 0x26, 0x30, 0x2C, 0x32, 0x2C, 0x33, 0x30, 0x2C, 0x31, 0x2C, 0x32, 0x2C, 0x33, 0x2C, 0x34, 0x2C, 0x35, 0x2C, 0x36};
UINT8 aux3[] = {
	0x1D, 0x00, 0x00, 0x09, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0x18, 0x00, 0x1B, 0x00, 0xFF, 0xFF, 0x1C, 0x00, 0x24, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x00, 0xFF, 0xFF, 0x58, 0x00, 0xFF, 0xFF, 0x62, 0x00, 0xE2, 0x00, 0xEA, 0x00, 0xFF, 0xFF, 0x12, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x39, 0x30, 0x30, 0x30, 0x2C, 0x34, 0x35, 0x30, 0x30, 0x35, 0x36, 0x37, 0x2C, 0x35, 0x37, 0x37, 0x35, 0x36, 0x37, 0x2C, 0x31, 0x36, 0x37, 0x30, 0x36, 0x35, 0x32, 0x31, 0x30, 0x30, 0x30, 0x32, 0x30, 0x39, 0x30, 0x30, 0x30, 0x44, 0x43, 0x33, 0x34, 0x30, 0x30, 0x30, 0x30, 0x30, 0x41, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x35, 0x30, 0x30, 0x30, 0x32, 0x30, 0x34, 0x41, 0x30, 0x30, 0x32, 0x30, 0x34, 0x36, 0x30, 0x30, 0x32, 0x30, 0x34, 0x34, 0x30, 0x30, 0x32, 0x30, 0x34, 0x32, 0x30, 0x30, 0x34, 0x30, 0x31, 0x30, 0x30, 0x30, 0x43, 0x30, 0x43, 0x30, 0x33, 0x30, 0x31, 0x30, 0x42, 0x30, 0x35, 0x30, 0x33, 0x30, 0x32, 0x30, 0x42, 0x30, 0x36, 0x30, 0x33, 0x30, 0x33, 0x30, 0x42, 0x30, 0x30, 0x30, 0x33, 0x30, 0x34, 0x30, 0x42, 0x30, 0x31, 0x30, 0x33, 0x30, 0x35, 0x30, 0x42, 0x30, 0x34, 0x30, 0x33, 0x30, 0x36, 0x30, 0x42, 0x30, 0x32, 0x30, 0x33, 0x30, 0x37, 0x30, 0x42, 0x30, 0x32, 0x30, 0x33, 0x30, 0x38, 0x30, 0x42, 0x30, 0x38, 0x30, 0x33, 0x30, 0x39, 0x30, 0x42, 0x30, 0x44, 0x30, 0x33, 0x30, 0x41, 0x30, 0x42, 0x30, 0x44, 0x30, 0x33, 0x30, 0x44, 0x30, 0x42, 0x30, 0x37, 0x30, 0x33, 0x31, 0x36, 0x30, 0x42, 0x30, 0x42, 0x30, 0x33, 0x31, 0x38, 0x30, 0x42, 0x30, 0x35, 0x30, 0x33, 0x31, 0x39, 0x30, 0x42, 0x30, 0x39, 0x30, 0x33, 0x31, 0x42, 0x30, 0x42, 0x31, 0x39, 0x30, 0x33, 0x38, 0x33, 0x30, 0x42, 0x31, 0x35, 0x30, 0x33, 0x30, 0x38, 0x30, 0x44, 0x30, 0x31, 0x30, 0x33, 0x33, 0x30, 0x33, 0x33, 0x30, 0x31, 0x30, 0x33, 0x33, 0x30, 0x33, 0x33, 0x30, 0x34, 0x30, 0x33, 0x33, 0x30, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x33, 0x30, 0x33, 0x33, 0x30, 0x36, 0x30, 0x33, 0x33, 0x30, 0x33, 0x33, 0x30, 0x32, 0x30, 0x33, 0x30, 0x44, 0x31, 0x30, 0x30, 0x30, 0x30, 0x33, 0x30, 0x44, 0x31, 0x30, 0x30, 0x37};
UINT8 aux4[] = {
	0x1D, 0x00, 0x00, 0x09, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0x18, 0x00, 0x1B, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x1C, 0x00, 0x36, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x90, 0x00, 0xB8, 0x00, 0xD8, 0x00, 0xE8, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x24, 0x01, 0x25, 0x01, 0x28, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x38, 0x39, 0x30, 0x30, 0x2C, 0x34, 0x35, 0x30, 0x30, 0x36, 0x34, 0x31, 0x2C, 0x36, 0x32, 0x34, 0x36, 0x34, 0x32, 0x2C, 0x31, 0x35, 0x34, 0x30, 0x34, 0x35, 0x32, 0x31, 0x30, 0x43, 0x31, 0x30, 0x31, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x38, 0x34, 0x30, 0x30, 0x30, 0x31, 0x39, 0x30, 0x32, 0x30, 0x38, 0x31, 0x36, 0x30, 0x32, 0x30, 0x38, 0x31, 0x37, 0x30, 0x32, 0x30, 0x38, 0x31, 0x38, 0x30, 0x32, 0x30, 0x38, 0x31, 0x39, 0x30, 0x32, 0x30, 0x38, 0x32, 0x30, 0x30, 0x32, 0x30, 0x38, 0x32, 0x31, 0x30, 0x32, 0x30, 0x38, 0x32, 0x32, 0x30, 0x32, 0x30, 0x38, 0x32, 0x33, 0x30, 0x32, 0x30, 0x38, 0x32, 0x34, 0x30, 0x32, 0x30, 0x38, 0x32, 0x35, 0x30, 0x32, 0x30, 0x38, 0x32, 0x36, 0x30, 0x32, 0x30, 0x38, 0x32, 0x37, 0x30, 0x32, 0x30, 0x38, 0x32, 0x38, 0x30, 0x32, 0x30, 0x38, 0x32, 0x39, 0x30, 0x32, 0x30, 0x38, 0x33, 0x30, 0x30, 0x33, 0x30, 0x30, 0x30, 0x34, 0x30, 0x31, 0x30, 0x33, 0x30, 0x30, 0x30, 0x34, 0x30, 0x38, 0x30, 0x33, 0x30, 0x30, 0x30, 0x34, 0x30, 0x30, 0x30, 0x33, 0x30, 0x30, 0x30, 0x34, 0x30, 0x34, 0x30, 0x33, 0x30, 0x30, 0x30, 0x34, 0x30, 0x32, 0x30, 0x33, 0x34, 0x43, 0x35, 0x30, 0x30, 0x30, 0x30, 0x33, 0x34, 0x43, 0x35, 0x30, 0x30, 0x31, 0x30, 0x33, 0x34, 0x43, 0x35, 0x30, 0x30, 0x32, 0x30, 0x33, 0x34, 0x43, 0x35, 0x30, 0x30, 0x33, 0x30, 0x33, 0x34, 0x38, 0x34, 0x43, 0x30, 0x34, 0x30, 0x33, 0x34, 0x38, 0x34, 0x43, 0x30, 0x30, 0x30, 0x34, 0x30, 0x32, 0x30, 0x34, 0x30, 0x38, 0x30, 0x31, 0x30, 0x34, 0x30, 0x33, 0x30, 0x34, 0x30, 0x38, 0x30, 0x33, 0x30, 0x34, 0x30, 0x34, 0x30, 0x34, 0x30, 0x38, 0x30, 0x34, 0x30, 0x34, 0x30, 0x35, 0x30, 0x34, 0x30, 0x38, 0x30, 0x35, 0x30, 0x34, 0x30, 0x36, 0x30, 0x34, 0x30, 0x38, 0x30, 0x43, 0x30, 0x34, 0x30, 0x37, 0x30, 0x34, 0x30, 0x38, 0x30, 0x44, 0x54, 0x53, 0x26, 0x30, 0x54, 0x7C, 0x53, 0x26, 0x30, 0x2C, 0x32, 0x2C, 0x33};
uint16_t decoded[1024] = {8979,4525,550,1693,550,1693,550,550,550,550,550,550,550,550,550,1693,550,1693,550,550,550,550,550,550,550,550,550,1693,550,550,550,550,550,1693,550,550,550,550,550,550,550,550,550,550,550,1693,550,1693,550,1693,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,1693,550,1693,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,1693,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,1693,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,1693,550,550,550,1693,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,550,1693,550,1693,550,550,550,1693,550,1693,550};
	uint16_t decode_len=211;//缺省值
uint8_t decoded_index=0;
uint8_t ifhigh=1;
	uint8_t ifcollectmpu=0;//测试用，打印陀螺仪零飘
	long int ggx=0;//
	long int ggz=0;//
nnom_model_t *model;
short gx = 0, gy = 0, gz = 0;
uint8_t data_feed_On = 0;
uint8_t count = 0; // feed count
short gyro[150][3] = {0};
long int x_sum=0;//键鼠计数
long int y_sum=0;
uint8_t ifclicking=0;//鼠标是否按下
uint8_t received = 255;
uint8_t KeyCode = 0;
RGB_Color_TypeDef rgb = {0, 0, 0};

// Enabletype
uint8_t ENwork = 0;
// uint8_t workState=0;//0 for IR&1 for BLE...use gpio read

uint8_t ACagreement = 101;//协议缺省

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ShowHex(uint8_t *buf, uint8_t len)
{
	uint8_t i;
	printf("hex = ");
	for (i = 0; i < len; i++)
	{
		printf(" %02X", buf[i]); // 使用前导0补齐
								 // printf(" %2X",buf[i]);  //使用前导空格补齐
								 // printf(" %X",buf[i]);  //输出16进制格式
	}
	printf("\r\n");
}

void AC_init()
{ // read from flash
	uint16_t device_id = W25QXX_ReadID();
	printf("W25Q64 Device ID is 0x%04x\r\n", device_id);
	uint8_t read_buf[6] = {0};
	printf("read ac data");
	W25QXX_Read(read_buf, flashbegin, 6);
	printf("读取空调历史信息如下:");
	ShowHex(read_buf, 6);
	// first download
//	ac_status.ac_mode = AC_MODE_COOL;
//	ac_status.ac_power = AC_POWER_OFF;
//	ac_status.ac_temp = AC_TEMP_26;
//	ac_status.ac_wind_dir = AC_SWING_ON;
//	ac_status.ac_wind_speed = AC_WS_LOW;

	//not first
	 ACagreement=read_buf[0];
		    ac_status.ac_mode=(t_ac_mode)read_buf[1];
	    ac_status.ac_power=(t_ac_power)read_buf[2];
	    ac_status.ac_temp=(t_ac_temperature)read_buf[3];
	    ac_status.ac_wind_dir=(t_ac_swing)read_buf[4];
	    ac_status.ac_wind_speed=(t_ac_wind_speed)read_buf[5];
}

void AC_save()
{
	uint8_t write_buf[6] = {0};
	write_buf[0] = ACagreement;
	write_buf[1] = ac_status.ac_mode;
	write_buf[2] = ac_status.ac_power;
	write_buf[3] = ac_status.ac_temp;
	write_buf[4] = ac_status.ac_wind_dir;
	write_buf[5] = ac_status.ac_wind_speed;
	printf("已保存空调信息到内存:");
	W25QXX_Page_Program(write_buf, flashbegin, 6); // 写数据
	ShowHex(write_buf, 6);
}
void AC_update(uint8_t mode)
{
	/*0 off;1 on;2 cool;3 hot;4 temp++;5 temp--;6swingON;7swingOFF;8wind auto ;9wind low;10wind middle;11wind high
	101,102,103,104 aux;201,202...gree...
	*/
	switch (mode)
	{
	case 0:
		ac_status.ac_power = AC_POWER_OFF;
		break;
	case 1:
		ac_status.ac_power = AC_POWER_ON;
		break;
	case 2:
		ac_status.ac_mode = AC_MODE_COOL;
		break;
	case 3:
		ac_status.ac_mode = AC_MODE_HEAT;
		break;
	case 4:
		ac_status.ac_temp = ac_status.ac_temp + 1 < AC_TEMP_MAX ? ac_status.ac_temp + 1 : AC_TEMP_30;
		break;
	case 5:
		ac_status.ac_temp = ac_status.ac_temp == AC_TEMP_16 ? AC_TEMP_16 : ac_status.ac_temp - 1;

		break;
	case 6:
		ac_status.ac_wind_dir = AC_SWING_ON;
		break;
	case 7:
		ac_status.ac_wind_dir = AC_SWING_OFF;
		break;
	case 8:
		ac_status.ac_wind_speed = AC_WS_AUTO;
		break;
	case 9:
		ac_status.ac_wind_speed = AC_WS_LOW;

		break;
	case 10:
		ac_status.ac_wind_speed = AC_WS_MEDIUM;

		break;
	case 11:
		ac_status.ac_wind_speed = AC_WS_HIGH;

		break;
	default:
		ACagreement = mode;
		break;
	}
	AC_save();
}

void AC_getsrcArray(UINT8 **p, UINT16 *srcArraylens)
{
	switch (ACagreement)
	{
	case 101:
		*p = aux1;
		*srcArraylens = sizeof(aux1);
		break;
	case 102:
		*p = aux2;
		*srcArraylens = sizeof(aux2);
		break;
	case 103:
		*p = aux3;
		*srcArraylens = sizeof(aux3);
		break;
	case 104:
		*p = aux4;
		*srcArraylens = sizeof(aux4);
	default:
		*p = aux1;
		*srcArraylens = sizeof(aux1);
		printf("内存坏块或协议不存在，已切换为寝室空调默认协议");
		break;
	}
}


void AC_send()
{
				UINT8 *p = NULL;
			UINT16 srcArraylens = 0;
					AC_getsrcArray(&p, &srcArraylens);
				ir_binary_open(1, 0, p, srcArraylens);
				decode_len = ir_decode(KEY_AC_POWER, decoded, &ac_status, 0);
				ir_close();
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim2);
}
void setrgb(RGB_Color_TypeDef Color)
{
	rgb = Color;
}

// 上位机拥有最高权限，和asrpro,手势共用数据处理函数
void dataProcess(uint8_t receivedData)
{
	// 常开
	uint8_t data[11] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	if (receivedData == 0x00)
	{
		ENwork = 1;
		HAL_UART_Transmit(&huart2, &data[1], 1,0xfff);
		printf("ENworkMode");
		KEY_FIFO_Put(KEY_4_LONG);
	}
	//测试mpu6050
	if (receivedData==0x0D){
	printf("开始打印MPU6050平均值");
		ggx=0;
		ggz=0;
		ifcollectmpu=1;
	}
	else if (receivedData >= 100)
	{
		AC_update(receivedData);
		AC_send();
		HAL_UART_Transmit(&huart2, &data[9], 1,0xfff);
		printf("协议已修改，是否有反应？");
	}

	// 不在工作状态时不可以
	if (ENwork == 1)
	{
		switch (receivedData)
		{
		case 0x01: // kuertasi
			printf("kurtasi?");
			setrgb(RED);
			KEY_FIFO_Put(KEY_4_DOWN);
			break;
		case 0x02: // luokemote
			printf("lokemote?");
			setrgb(SKY);
			KEY_FIFO_Put(KEY_4_DOWN);
			break;
		case 0x03: // jinitaimei
			HAL_UART_Transmit(&huart2, &data[0], 1,0xfff);
			printf("食不食油饼？");
			setrgb(MAGENTA);
			KEY_FIFO_Put(KEY_4_DOWN);
			break;
		case 0x04: // tuichu
			HAL_UART_Transmit(&huart2, &data[10], 1,0xfff);
			printf("bye");
			ENwork = 0;
			setrgb(YELLOW);
			KEY_FIFO_Put(KEY_4_DOWN);

			break;
		case 0x05: // AC on
			AC_update(1);
			AC_send();
			printf("AC on");
			HAL_UART_Transmit(&huart2, &data[2], 1,0xfff);
			break;
		case 0x06: // AC off
			AC_update(0);
			AC_send();
			printf("AC off");
			HAL_UART_Transmit(&huart2, &data[3], 1,0xfff);
			break;
		case 0x07: // temp up
			AC_update(4);
			HAL_UART_Transmit(&huart2, &data[9], 1,0xfff);
			AC_send();
			printf("temprature up");

			break;
		case 0x08: // temp dowm
			AC_update(5);
			HAL_UART_Transmit(&huart2, &data[9], 1,0xfff);
			AC_send();
			printf("temperature down");

			break;
		case 0x09: // swing
			if (ac_status.ac_wind_dir == AC_SWING_OFF)
			{
				AC_update(6);
				HAL_UART_Transmit(&huart2, &data[6], 1,0xfff);
			}
			else
			{
				AC_update(7);
			}
			AC_send();
			printf("扫风模式改变");
			break;
		case 0x0A: // mode
			if (ac_status.ac_mode == AC_MODE_COOL)
			{
				AC_update(3);
				HAL_UART_Transmit(&huart2, &data[4], 1,0xfff);
			}
			else
			{
				AC_update(2);
				HAL_UART_Transmit(&huart2, &data[5], 1,0xfff);
			}
			AC_send();
			printf("空调模式改变");
			break;
		case 0x0B: // speed
			if (ac_status.ac_wind_speed == AC_WS_AUTO)
			{
				AC_update(9);
			}
			else if (ac_status.ac_wind_speed == AC_WS_LOW)
			{
				AC_update(10);
			}
			else if (ac_status.ac_wind_speed == AC_WS_MEDIUM)
			{
				AC_update(11);
			}
			else if (ac_status.ac_wind_speed == AC_WS_HIGH)
			{
				AC_update(8);
			}
			HAL_UART_Transmit(&huart2, &data[7], 1,0xfff);
			AC_send();
			printf("风速改变");

			break;
		case 0x0C: // misikamosika
			printf("米老鼠？");
			setrgb(OEANGE);
			KEY_FIFO_Put(KEY_4_DOWN);
			break;
		}
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
	for (i = 0; i < IMU_SEQUENCE_LENGTH_MAX; i++)
	{
		nnom_input_data[i * 3] = (int8_t)round((float)gyro[i][Roll] / IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
		nnom_input_data[i * 3 + 1] = (int8_t)round((float)gyro[i][Pitch] / IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
		nnom_input_data[i * 3 + 2] = (int8_t)round((float)gyro[i][Yaw] / IMU_GYRO_TRANS_RADIAN_CONSTANT * scale);
	}
}

int8_t model_get_output(void)
{
	volatile uint8_t i = 0;
	volatile int8_t max_output = -128;
	int8_t ret = 0;
	model_feed_data();
	model_run(model);
	for (i = 0; i < 13; i++)
	{

#ifdef SERIAL_DEBUG
		printf("Output[%d] = %.2f %%\n", i, (nnom_output_data[i] / 127.0) * 100);
#endif // SERIAL_DEBUG

		if (nnom_output_data[i] >= max_output)
		{
			max_output = nnom_output_data[i];
			ret = i;
		}
	}
	if (max_output < OUPUT_THRESHOLD || ret == NoMotion)
	{
		ret = Unrecognized;
	}
	// put into vitual fifo
	switch (ret)
	{
		uint8_t data[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
	case Unrecognized:
		printf("Unrecognized");
		HAL_UART_Transmit(&huart2, &data[8], 1,0xfff);
		break;
	case RightAngle:
		printf("RightAngle");
		dataProcess(0x01);
		break;
	case SharpAngle:
		printf("SharpAngle");
		dataProcess(0x02);

		break;
	case Lightning:
		printf("Lightning");
		dataProcess(0x03);
		break;
	case Triangle:
		printf("Triangle");
		dataProcess(0x04);
		break;
	case Letter_h:
		printf("Letter_h");
		dataProcess(0x05);
		break;
	case letter_R:
		printf("Letter_R");
		dataProcess(0x06);
		break;
	case letter_W:
		printf("Letter_W");
		dataProcess(0x07);
		break;
	case letter_phi:
		printf("Letter_phi");
		dataProcess(0x08);
		break;
	case Circle:
		printf("Circle");
		dataProcess(0x00);
		break;
	case UpAndDown:
		printf("UpAndDown");
		dataProcess(0x09);
		break;
	case Horn:
		printf("Horn");
		dataProcess(0x0A);
		break;
	case Wave:
		printf("Wave");
		dataProcess(0x0B);
		break;
	case NoMotion:
		printf("Unrecognized");
		HAL_UART_Transmit(&huart2, &data[8], 1,0xfff);
		break;
	}
	printf("\n");

	return ret;
}
void mouseclick()
{ifclicking=0x01;
	uint8_t data1[8] = {0x08, 0x00, 0xA1, 0x02, 0x01, 0x00, 0x00, 0x00};
	HAL_UART_Transmit(&huart3, data1, 8,0xfff);
}
void mousediscli()
{
	ifclicking=0x00;
	uint8_t data2[8] = {0x08, 0x00, 0xA1, 0x02, 0x00, 0x00, 0x00, 0x00};
	HAL_UART_Transmit(&huart3, data2, 8,0xfff);
}

void mouseMove(signed char x, signed char y)
{
	uint8_t data[8] = {0x08, 0x00, 0xA1, 0x02, ifclicking, x, y, 0x00};
	HAL_UART_Transmit(&huart3, data, 8,0xfff);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim2){
		// 微秒延迟,重载值11，故频率6mhz，计数6次才1微秒
		//看其他地方有用while函数硬等的，但是我这里不知道为什么似乎计数器的频率高于while的频率，导致计时不准确，故采用中断的方式
		
				if(decoded_index==decode_len){
					__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,0);
					HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
			HAL_TIM_Base_Stop_IT(&htim2);
			decoded_index=0;
			ifhigh=1;
		}else{
		__HAL_TIM_SetCounter(&htim2,0);
		__HAL_TIM_SET_AUTORELOAD(&htim2,decoded[decoded_index]*6-1);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ifhigh==1?(1894 / 3):0); 
		ifhigh=!ifhigh;
		decoded_index++;
		}

		
		
}else if (htim == &htim4)
	{
		KEY_Scan();
		// original exti0，原来采用外部中断，但我这里不知道是硬件问题还是咋，开外部中断就会卡i2c

		MPU_Get_Gyroscope(&gx, &gy, &gz);
		// printf("%d\r\n",gx);
		
		//测试用
		if(ifcollectmpu!=0){
		ggx+=gx;
			ggz+=gz;
			if(ifcollectmpu==100){
				printf("x:%d",(int)ggx/100);
				printf("z:%d",(int)ggz/100);
				ifcollectmpu=0;
			
			}else{
			ifcollectmpu++;
			}
		}
		//
		
		
		if (!HAL_GPIO_ReadPin(USER_Button2_GPIO_Port, USER_Button2_Pin))
		{
			// jianshumoshi
			
			x_sum+=(gx-xbias)*mouseGain;
			y_sum+=(gz-zbias)*mouseGain;
			
			//串口3的dma被占用了没办法，用中断模式会乱码（不知道为啥，可能是被其他中断打断了？），故移动到主循环
//			//for x
//			signed char i=0;
//			while(x_sum>countReloadvalue){
//				x_sum-=countReloadvalue;
//				i--;
//			
//			}
//			
//			while(x_sum<-countReloadvalue){
//			x_sum+=countReloadvalue;
//				i++;
//			}
//			mouseMove(i,0);
//			
//			
//			//for y
//			i=0;
//						while(y_sum>countReloadvalue){
//				y_sum-=countReloadvalue;
//							i++;
//			
//			}
//						while(y_sum<-countReloadvalue){
//			y_sum+=countReloadvalue;
//							i--;
//			}
//						mouseMove(0,i);

		}
		
		if (data_feed_On)
		{
			gyro[count][Roll] = gx;
			gyro[count][Pitch] = gy;
			gyro[count][Yaw] = gz;
			if (++count == 150)
			{
				model_feed_data();
				data_feed_On = 0;
				count = 0;
				model_get_output(); // recognize gesture
			}
		}
	}
}

// uart1 for hc05(shangweiji);uart2 for asrpro;uart3 for ble(only tx)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2 || huart == &huart1)
	{
		dataProcess(received);
		HAL_UART_Receive_IT(&huart1, &received, 1);
		HAL_UART_Receive_IT(&huart2, &received, 1);
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

	printf("连上了！");
// crate CNN model
#ifdef NNOM_USING_STATIC_MEMORY
	nnom_set_static_buf(static_buf, sizeof(static_buf));
#endif // NNOM_USING_STATIC_MEMORY

	MPU_Init();
	model = nnom_model_create();

	AC_init();

	// get data from flash

	KEY_Init();
	HAL_TIM_Base_Start_IT(&htim4);
	if (HAL_GPIO_ReadPin(USER_Button2_GPIO_Port, USER_Button2_Pin))
	{
		printf("初始化为手势模式");
	}
	else
	{
		printf("初始化为键鼠模式");
	}
	HAL_UART_Receive_IT(&huart1, &received, 1);
	HAL_UART_Receive_IT(&huart2, &received, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		
					//for x
			signed char i=0;
			while(x_sum>countReloadvalue){
				x_sum-=countReloadvalue;
				i--;
			
			}
			
			while(x_sum<-countReloadvalue){
			x_sum+=countReloadvalue;
				i++;
			}
			mouseMove(i,0);
			
			
			//for y
			i=0;
						while(y_sum>countReloadvalue){
				y_sum-=countReloadvalue;
							i++;
			
			}
						while(y_sum<-countReloadvalue){
			y_sum+=countReloadvalue;
							i--;
			}
						mouseMove(0,i);
			
			
		KeyCode = KEY_FIFO_Get();
		if (KeyCode != KEY_NONE)
		{
			//UINT8 *p = NULL;
			//UINT16 srcArraylens = 0;
			switch (KeyCode)
			{
			case KEY_DOWN_K1:
				printf("按下了按用户按键");
				HAL_GPIO_WritePin(PA1_LED_GPIO_Port, PA1_LED_Pin, GPIO_PIN_SET);
				if (!HAL_GPIO_ReadPin(USER_Button2_GPIO_Port, USER_Button2_Pin))
				{
					// click mouse
					mouseclick();
				}
				break;
			case KEY_1_LONG:
				if (HAL_GPIO_ReadPin(USER_Button2_GPIO_Port, USER_Button2_Pin))
				{
					//...rgb ,finger light slowly
					// 开始进行手势识别,键鼠模式无法手势识别、渐亮蓝灯
					data_feed_On = 1;
					rgb_breathe();
				}

				break;
			case KEY_1_UP:
				printf("松开了用户按键");
				HAL_GPIO_WritePin(PA1_LED_GPIO_Port, PA1_LED_Pin, GPIO_PIN_RESET);
				if (HAL_GPIO_ReadPin(USER_Button2_GPIO_Port, USER_Button2_Pin))
				{ // 键鼠模式也不能流水灯
					for (int i = 0; i < 12; i++)
					{
						RGB_SetColor(i, BLACK);
						Reset_Load();
						RGB_SendArray();
					}
				}
				else
				{
					// disclick mouse
					mousediscli();
				}
				break;
				//以下没啥用的按键
//			case KEY_DOWN_K2:
//				printf("超大角度1");
//				break;
//			case KEY_2_LONG:
//				break;
//			case KEY_DOWN_K3: //
//				printf("超大角度2");
//			break;
			case KEY_DOWN_K4:
				//rgb_loop(rgb);
				break;
			case KEY_4_LONG:
				setrgb(BLUE);
				rgb_loop(rgb);
				HAL_Delay(100);
				setrgb(BLUE);
				rgb_loop(rgb);
				HAL_Delay(100);
				setrgb(BLUE);
				rgb_loop(rgb);
				break;
			case KEY_4_UP:
				
				break;
			default:
				break;
			}
		}
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
