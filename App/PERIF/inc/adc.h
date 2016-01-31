
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined (VER_3)
	#define REF_VOLTAGE      3.3	  //Opornoe voltage
	#define koefADC       (REF_VOLTAGE / 4096)
	// VBAT
	#define koefVBAT      (2.0 * koefADC)    //found by scheme R24/(R24+R25)=0.5 
	// VIN
	//#define koefVPower    (9.66 * koefADC)    //found by scheme R33/(R33+R34)=0.1 and by measure AIN(1.0V),VCHRG(10.0V)  = 10 
	#define koefStdADC    (10.0 * koefADC)    // should check
typedef enum  {
  BATT_CHECK_OFF = 0,
  BATT_CHECK_ON    
} batt_check_action;

#else
	#define REF_VOLTAGE      3.25
	#define koefADC       (REF_VOLTAGE / 4096)
	// VBAT
	#define koefVBAT      (11.25 * koefADC)    //found by measure  0.63v (AIN)- VBAT  = 7.09; 7.09 / 0.63 = 11.25
	// VIN
	#define koefVPower    (11.06 * koefADC)    //found by measure  1.07v (AIN)- VCHRG  = 11.84V => 11.84 / 1.07 = 11.06
	#define koefStdADC    (11.26 * koefADC)    // should check

#endif
//AIN
#define TTL_AIN1_PORT                         GPIOC  				   //adc123_IN10
#define TTL_AIN1_PORT_PIN                     GPIO_Pin_0
#define RCC_APB2Periph_GPIO_TTL_AIN1          RCC_APB2Periph_GPIOC
#define TTL_AIN2_PORT                         GPIOC  				   //adc123_IN11
#define TTL_AIN2_PORT_PIN                     GPIO_Pin_1
#define RCC_APB2Periph_GPIO_TTL_AIN2          RCC_APB2Periph_GPIOC

#if defined (VER_3)
	#define TTL_AIN3_PORT                         GPIOC  				   
	#define TTL_AIN3_PORT_PIN                     GPIO_Pin_2
	#define RCC_APB2Periph_GPIO_TTL_AIN3          RCC_APB2Periph_GPIOC
	#define TTL_AIN4_PORT                         GPIOC  				   
	#define TTL_AIN4_PORT_PIN                     GPIO_Pin_3
	#define RCC_APB2Periph_GPIO_TTL_AIN4          RCC_APB2Periph_GPIOC
	#define TTL_AIN5_PORT                         GPIOC  				   
	#define TTL_AIN5_PORT_PIN                     GPIO_Pin_4
	#define RCC_APB2Periph_GPIO_TTL_AIN5          RCC_APB2Periph_GPIOC
	#define TTL_AIN6_PORT                         GPIOC  				   
	#define TTL_AIN6_PORT_PIN                     GPIO_Pin_5
	#define RCC_APB2Periph_GPIO_TTL_AIN6          RCC_APB2Periph_GPIOC
	
	#define VIN_SENS_PORT                         GPIOB  				   //adc123_IN14
	#define VIN_SENS_PORT_PIN                     GPIO_Pin_0
	#define RCC_APB2Periph_GPIO_VIN_SENS          RCC_APB2Periph_GPIOB
	#define VBAT_SENS_PORT                        GPIOB  				  //adc12_IN14
	#define VBAT_SENS_PORT_PIN                    GPIO_Pin_1
	#define RCC_APB2Periph_GPIO_VBAT_SENS         RCC_APB2Periph_GPIOB

//POWER VBAT CHECK
#if defined (VER_3)
	#define POWER_VBAT_CHECK_PORT                 GPIOD  
	#define POWER_VBAT_CHECK_PORT_PIN             GPIO_Pin_11
	#define RCC_APB2Periph_GPIO_POWER_VBAT_CHECK  RCC_APB2Periph_GPIOD
#endif

#else
	#define VIN_SENS_PORT                         GPIOC  				   //adc123_IN14
	#define VIN_SENS_PORT_PIN                     GPIO_Pin_4
	#define RCC_APB2Periph_GPIO_VIN_SENS          RCC_APB2Periph_GPIOC
	#define VBAT_SENS_PORT                        GPIOC  				  //adc12_IN14
	#define VBAT_SENS_PORT_PIN                    GPIO_Pin_5
	#define RCC_APB2Periph_GPIO_VBAT_SENS         RCC_APB2Periph_GPIOC
#endif



/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void adcGPIOInit(void);
void adcInterruptsConfig(void);
void adcInit(void);
u16  ain_read(u8 channel);
void adcScan(void);
void handleTimerADC(void);
void setTimerADC(u32 val);
void adcProcessHandler(void);
void ainDebRead(u8 *buf, u16 len);
//void adcCalcKoef(void);
u16 setCalibrKoef(u8 * buf);

#if defined (VER_3)
void pwr_vbat_check(batt_check_action action);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
