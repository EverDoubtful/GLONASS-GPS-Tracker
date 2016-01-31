
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DIO_LED_H
#define __DIO_LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum  {
  BOARD_LED_OFF = 0,
  BOARD_LED_ON,
  BOARD_LED_XOR  
} led_action;
typedef enum  {
  DOUT_OFF = 0,
  DOUT_ON    
} dout_action;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void led_mid( led_action act );
void led_dn( led_action act);
#if defined (VER_3)
  void led_up( led_action act);
#endif
void ttl_dout( u8 maskNumTU, u8 maskAction);
u8   controlTU(u8 * buf);
u8   readStateInput(void);
u8   readStateOutput(void);
void ledDioGPIOInit(void);
void handleTimerLeds(void);
#if defined (VER_3)	
  void setDutyCycle(u8 duty);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DIO_LED_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
