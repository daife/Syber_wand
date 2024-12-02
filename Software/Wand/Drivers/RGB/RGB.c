#include "RGB.h"
#include "tim.h"
 
/*Some Static Colors------------------------------*/

 
/*二维数组存放最终PWM输出数组，每一行24个
数据代表一个LED，最后一行24个0代表RESET码*/
uint32_t Pixel_Buf[Pixel_NUM+1][24];       
 

/*
功能：设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
参数：LedId为LED序号，Color：定义的颜色结构体
*/
void RGB_SetColor(uint8_t LedId,RGB_Color_TypeDef Color)
{
	uint8_t i; 
	if(LedId > Pixel_NUM)return; //avoid overflow 防止写入ID大于LED总数
	
	for(i=0;i<8;i++) Pixel_Buf[LedId][i]   = ( (Color.G & (1 << (7 -i)))? (CODE_1):CODE_0 );//数组某一行0~7转化存放G
	for(i=8;i<16;i++) Pixel_Buf[LedId][i]  = ( (Color.R & (1 << (15-i)))? (CODE_1):CODE_0 );//数组某一行8~15转化存放R
	for(i=16;i<24;i++) Pixel_Buf[LedId][i] = ( (Color.B & (1 << (23-i)))? (CODE_1):CODE_0 );//数组某一行16~23转化存放B
}
 
/*
功能：最后一行装在24个0，输出24个周期占空比为0的PWM波，作为最后reset延时，这里总时长为24*1.2=30us > 24us(要求大于24us)
*/
void Reset_Load(void)
{
	uint8_t i;
	for(i=0;i<24;i++)
	{
		Pixel_Buf[Pixel_NUM][i] = 0;
	}
}
 
/*
功能：发送数组
参数：(&htim1)定时器1，(TIM_CHANNEL_1)通道1，((uint32_t *)Pixel_Buf)待发送数组，
			(Pixel_NUM+1)*24)发送个数，数组行列相乘
*/
void RGB_SendArray(void)
{
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf,(Pixel_NUM+1)*24);
}
 
 
//也可以继续添加其他颜色，和颜色变化函数等
// 流水灯效
void rgb_loop(RGB_Color_TypeDef Color){
	uint16_t i;
	for(i=0;i<Pixel_NUM-1;i++){
		RGB_SetColor(i,Color);
		RGB_SetColor(i+1,Color);
					Reset_Load();
	RGB_SendArray();
		HAL_Delay(60);

	}
	RGB_Color_TypeDef black={0,0,0};
			for(int i=0;i<12;i++){
			RGB_SetColor(i,black);
							Reset_Load();
	RGB_SendArray();
			}
}
// 呼吸灯效
void rgb_breathe(){
	int g,b ;

	g = 0;
	b = 0;

	// 变亮
	for(int i=0;i<50;i++){

		if(g!=255){
			g+=5;
		}
		if(b!=255){
			b+=5;
		}
		RGB_Color_TypeDef Color_change={0,g,b};
		RGB_SetColor(2,Color_change);
		Reset_Load();
		RGB_SendArray();
		HAL_Delay(10);
	}
}