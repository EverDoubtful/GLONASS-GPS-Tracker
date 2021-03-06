
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACCEL_H
#define __ACCEL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
struct ais326dq_data_t {
	uint16_t ax;
	uint16_t ay;
	uint16_t az;
};

/* Exported constants --------------------------------------------------------*/
//
#define ACCEL_CS_PORT                         GPIOB  
#define ACCEL_CS_PORT_PIN                     GPIO_Pin_12
#define RCC_APB2Periph_GPIO_ACCEL_CS          RCC_APB2Periph_GPIOB
#define ACCEL_RDY_PORT                        GPIOD  
#define ACCEL_RDY_PORT_PIN                    GPIO_Pin_8
#define RCC_APB2Periph_GPIO_ACCEL_RDY         RCC_APB2Periph_GPIOD

#define ACCEL_RW_READ_SIGN                       0x80
#define ACCEL_RW_WRITE_SIGN                      0x0
#define ACCEL_MS_NO_INCR                         0x0
#define ACCEL_MS_INCR                            0x40

/* Exported macro ------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */
#define SPI_ACCEL_CS_LOW()       GPIO_ResetBits(ACCEL_CS_PORT, ACCEL_CS_PORT_PIN)
/* Deselect SPI FLASH: Chip Select pin high */
#define SPI_ACCEL_CS_HIGH()      GPIO_SetBits(ACCEL_CS_PORT, ACCEL_CS_PORT_PIN)

#define SELECT_ACCEL			 SPI_ACCEL_CS_LOW()
#define DESELECT_ACCEL			 SPI_ACCEL_CS_HIGH()

/* Exported functions ------------------------------------------------------- */

void spiAccelInit(void);
//REGISTER ADDRESS MAP

#define WHO_AM_I                     0x0F

#define OFFSET_X                     0x16
#define OFFSET_Y                     0x17
#define OFFSET_Z                     0x18
#define GAIN_X                       0x19
#define GAIN_Y                       0x1A
#define GAIN_Z                       0x1B

#define CTRL_REG1                    0x20
#define CTRL_REG2                    0x21
#define CTRL_REG3                    0x22

#define HP_FILTER_RESET              0x22
#define STATUS_REG                   0x22
#define OUTX_L                       0x28 //0x28
#define OUTX_H                       0x29
#define OUTY_L                       0x22
#define OUTY_H                       0x22
#define OUTZ_L                       0x22
#define OUTZ_H                       0x22

#define FF_WU_CFG                    0x22
#define FF_WU_SRC                    0x22
#define FF_WU_ACK                    0x22
#define FF_WU_THS_L                  0x22
#define FF_WU_THS_H                  0x22
#define FF_WU_DURATION               0x22
#define DD_CFG                       0x22
#define DD_SRC                       0x22
#define DD_ACK                       0x22
#define DD_THSI_L                    0x22
#define DD_THSI_H                    0x22
#define DD_THSE_L                    0x22
#define DD_THSE_H                    0x22
//BITS
#define CTRL_REG1_XEN                        ((uint8_t)0x0001)            /*!< , bit 0 */
#define CTRL_REG1_YEN                        ((uint8_t)0x0002)            /*!< , bit 1 */
#define CTRL_REG1_ZEN                        ((uint8_t)0x0004)            /*!< , bit 2 */
#define CTRL_REG1_ST                         ((uint8_t)0x0008)            /*!< , bit 3 */
#define CTRL_REG1_DF0                        ((uint8_t)0x0010)            /*!< , bit 4 */
#define CTRL_REG1_DF1                        ((uint8_t)0x0020)            /*!< , bit 5 */
#define CTRL_REG1_PD0                        ((uint8_t)0x0040)            /*!< , bit 6 */
#define CTRL_REG1_PD1                        ((uint8_t)0x0080)            /*!< , bit 7 */

// FUNCTIONS DECLARATIONS
void ais326dq_init(void);
void ais326dq_read(void);
void ais326dq_data(struct ais326dq_data_t *data);

void SPI_WriteREG(uint8_t address, uint8_t data);
void SPI_ReadREG(uint8_t address, uint8_t *data);
//void SPI_init(void);
uint8_t SPI_ReadByte(void);
uint8_t SPI_SendByte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* __ACCEL_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
