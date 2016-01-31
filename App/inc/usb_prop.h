/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : usb_prop.h
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : All processing related to Mass Storage Demo (Endpoint 0)
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usb_prop_H
#define __usb_prop_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t bitrate;
  uint8_t format;
  uint8_t paritytype;
  uint8_t datatype;
}LINE_CODING;

/* Exported constants --------------------------------------------------------*/
#define CMPDevice_GetConfiguration          NOP_Process
/* #define CMPDevice_SetConfiguration          NOP_Process*/
#define CMPDevice_GetInterface              NOP_Process
#define CMPDevice_SetInterface              NOP_Process
#define CMPDevice_GetStatus                 NOP_Process
/* #define CMPDevice_ClearFeature              NOP_Process*/
#define CMPDevice_ClearFeature              NOP_Process
#define CMPDevice_SetEndPointFeature        NOP_Process
#define CMPDevice_SetDeviceFeature          NOP_Process
/*#define Mass_Storage_SetDeviceAddress          NOP_Process*/

/* MASS Storage Requests*/
#define GET_MAX_LUN                0xFE
#define MASS_STORAGE_RESET         0xFF
#define LUN_DATA_LENGTH            1

//VCP
//#define Virtual_Com_Port_GetConfiguration          NOP_Process
////#define Virtual_Com_Port_SetConfiguration          NOP_Process
//#define Virtual_Com_Port_GetInterface              NOP_Process
//#define Virtual_Com_Port_SetInterface              NOP_Process
//#define Virtual_Com_Port_GetStatus                 NOP_Process
//#define Virtual_Com_Port_ClearFeature              NOP_Process
//#define Virtual_Com_Port_SetEndPointFeature        NOP_Process
//#define Virtual_Com_Port_SetDeviceFeature          NOP_Process
////#define Virtual_Com_Port_SetDeviceAddress          NOP_Process

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void CMPDevice_init(void);
void CMPDevice_Reset(void);
void CMPDevice_SetConfiguration(void);
void Mass_Storage_ClearFeature(void);
void CMPDevice_SetDeviceAddress (void);
void CMPDevice_Status_In (void);
void CMPDevice_Status_Out (void);
RESULT CMPDevice_Data_Setup(uint8_t);
RESULT CMPDevice_NoData_Setup(uint8_t);
RESULT CMPDevice_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *CMPDevice_GetDeviceDescriptor(uint16_t );
uint8_t *CMPDevice_GetConfigDescriptor(uint16_t);
uint8_t *CMPDevice_GetStringDescriptor(uint16_t);
uint8_t *Get_Max_Lun(uint16_t Length);

uint8_t *Virtual_Com_Port_GetLineCoding(uint16_t Length);
uint8_t *Virtual_Com_Port_SetLineCoding(uint16_t Length);
#ifdef __cplusplus
}
#endif

#endif /* __usb_prop_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

