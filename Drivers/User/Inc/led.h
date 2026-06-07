#ifndef __LED_H
#define __LED_H

/*------------------------------------------ LED配置宏 ----------------------------------*/

#define LED1_PIN            			 GPIO_PIN_1       				 	// LED1 引脚      
#define LED1_PORT           			 GPIOB                 			 	// LED1 GPIO端口     
#define __HAL_RCC_LED1_CLK_ENABLE    __HAL_RCC_GPIOB_CLK_ENABLE() 	// LED1 GPIO端口时钟
 
/*------------------------------------------ BEEP配置宏 ----------------------------------*/
#define BEEP_PIN            			 GPIO_PIN_8       				 	// LED1 引脚      
#define BEEP_PORT           			 GPIOE             			 	// LED1 GPIO端口     
#define __HAL_RCC_BEEP_CLK_ENABLE    __HAL_RCC_GPIOE_CLK_ENABLE() 	// LED1 GPIO端口时钟
 
/*----------------------------------------- LED控制宏 ----------------------------------*/
						
#define LED1_ON 	  	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)		// 输出高电平，点亮LED1	
#define LED1_OFF 	  	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)			// 输出低高电平，关闭LED1	
#define LED1_Toggle	HAL_GPIO_TogglePin(LED1_PORT,LED1_PIN);							// 翻转IO口状态

/*----------------------------------------- BEEP控制宏 ----------------------------------*/
	
#define BEEP_ON 	  	HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET)		// 输出高电平，打开BEEP	
#define BEEP_OFF 	  	HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET)			// 输出低电平，关闭BEEP	
#define BEEP_Toggle	HAL_GPIO_TogglePin(BEEP_PORT,BEEP_PIN);							// 翻转IO口状态


/*---------------------------------------- 函数声明 ------------------------------------*/

void LED_Init(void);
void BEEP_Init(void);
void BEEP_Control(int sw);
#endif //__LED_H


