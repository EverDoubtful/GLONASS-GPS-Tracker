/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : usb_prop.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : All processing related to Mass Storage Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_bot.h"
#include "hw_config.h"
#include "memory.h"
#include "mass_mal.h"
#include "usb_prop.h"
//#include <stdbool.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 uint32_t Max_Lun = 1;
extern u8 signUSBMass;
extern u8 ENDP0_RXADDR;  //define from usb_conf.h
extern u8 ENDP0_TXADDR;  //define from usb_conf.h
extern u8 ENDP1_TXADDR;
//extern u8 EP_NUM;

//DEVICE Device_Table =
//  {
//    EP_NUM,
//    1
//  };

DEVICE_PROP Device_Property =
  {
  
    CMPDevice_init,	  //MASS_init  and Virtual_Com_Port_init
    CMPDevice_Reset,
    CMPDevice_Status_In,
    CMPDevice_Status_Out,
    CMPDevice_Data_Setup,
    CMPDevice_NoData_Setup,
    CMPDevice_Get_Interface_Setting,
    CMPDevice_GetDeviceDescriptor,
    CMPDevice_GetConfigDescriptor,
    CMPDevice_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/

  };


USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    CMPDevice_GetConfiguration,
    CMPDevice_SetConfiguration,
    CMPDevice_GetInterface,
    CMPDevice_SetInterface,
    CMPDevice_GetStatus,
    CMPDevice_ClearFeature,
    CMPDevice_SetEndPointFeature,
    CMPDevice_SetDeviceFeature,
    CMPDevice_SetDeviceAddress
 };

ONE_DESCRIPTOR Device_Descriptor_Mass =
  {
    (uint8_t*)MASS_DeviceDescriptor,
    CMPDevice_SIZ_DEVICE_DESC
  };
ONE_DESCRIPTOR Device_Descriptor_VCP =
  {
    (uint8_t*)Virtual_Com_Port_DeviceDescriptor,
    CMPDevice_SIZ_DEVICE_DESC
  };
  

ONE_DESCRIPTOR Config_Descriptor_Mass =
  {
    (uint8_t*)MASS_ConfigDescriptor,
    MASS_SIZ_CONFIG_DESC
  };
ONE_DESCRIPTOR Config_Descriptor_VCP =
  {
	(uint8_t*)Virtual_Com_Port_ConfigDescriptor,
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR String_Descriptor_Mass[5] =
  {
    {(uint8_t*)MASS_StringLangID, MASS_SIZ_STRING_LANGID},
    {(uint8_t*)MASS_StringVendor, MASS_SIZ_STRING_VENDOR},
    {(uint8_t*)MASS_StringProduct, MASS_SIZ_STRING_PRODUCT},
    {(uint8_t*)MASS_StringSerial, MASS_SIZ_STRING_SERIAL},
    {(uint8_t*)MASS_StringInterface, MASS_SIZ_STRING_INTERFACE},
  };
ONE_DESCRIPTOR String_Descriptor_VCP[4] =
  {
     {(uint8_t*)Virtual_Com_Port_StringLangID, VIRTUAL_COM_PORT_SIZ_STRING_LANGID},
     {(uint8_t*)Virtual_Com_Port_StringVendor, VIRTUAL_COM_PORT_SIZ_STRING_VENDOR},
     {(uint8_t*)Virtual_Com_Port_StringProduct, VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT},
     {(uint8_t*)Virtual_Com_Port_StringSerial, VIRTUAL_COM_PORT_SIZ_STRING_SERIAL},  
  };
  
uint8_t Request = 0;

LINE_CODING linecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* no. of bits 8*/
  };

/* Extern variables ----------------------------------------------------------*/
extern unsigned char Bot_State;
extern Bulk_Only_CBW CBW;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MASS_init
* Description    : Mass Storage init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_init()
{
  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;

}

/*******************************************************************************
* Function Name  : MASS_Reset
* Description    : Mass Storage reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_Reset()
{
  if(signUSBMass)
   {
  /* Set the device as not configured */
  Device_Info.Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = MASS_ConfigDescriptor[7];


  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_BULK);
  SetEPRxAddr(ENDP2, ENDP2_RXADDR);
  SetEPRxCount(ENDP2, Device_Property.MaxPacketSize);
  SetEPRxStatus(ENDP2, EP_RX_VALID);
  SetEPTxStatus(ENDP2, EP_TX_DIS);


  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Set the device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;

  CBW.dSignature = BOT_CBW_SIGNATURE;
  Bot_State = BOT_IDLE;

   
   }
   else
   {
  /* Set Virtual_Com_Port DEVICE as not configured */
  pInformation->Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = Virtual_Com_Port_ConfigDescriptor[7];

  /* Set Virtual_Com_Port DEVICE with the default Interface*/
  pInformation->Current_Interface = 0;

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_INTERRUPT);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  SetEPTxStatus(ENDP2, EP_TX_NAK);

  /* Initialize Endpoint 3 */
  SetEPType(ENDP3, EP_BULK);
  SetEPRxAddr(ENDP3, ENDP3_RXADDR);
  SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPRxStatus(ENDP3, EP_RX_VALID);
  SetEPTxStatus(ENDP3, EP_TX_DIS);

  /* Set this device to response on default address */
  SetDeviceAddress(0);


  bDeviceState = ATTACHED;
   
   }

}

/*******************************************************************************
* Function Name  : Mass_Storage_SetConfiguration
* Description    : Handle the SetConfiguration request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_SetConfiguration(void)
{
    DEVICE_INFO *pInfo = &Device_Info;

 if(signUSBMass)
  { 
  if (pInformation->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;

    ClearDTOG_TX(ENDP1);
    ClearDTOG_RX(ENDP2);

    Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
  }
  }
 else
  {  

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  }

}

/*******************************************************************************
* Function Name  : Mass_Storage_ClearFeature
* Description    : Handle the ClearFeature request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Mass_Storage_ClearFeature(void)
{
  if(signUSBMass)
  {
  /* when the host send a CBW with invalid signature or invalid length the two
     Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset     */
  if (CBW.dSignature != BOT_CBW_SIGNATURE)
    Bot_Abort(BOTH_DIR);
  }
  else
  {
   ;
  }
}

/*******************************************************************************
* Function Name  : Mass_Storage_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : MASS_Status_In
* Description    : Mass Storage Status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_Status_In(void)
{
 if(signUSBMass)
  return;
 else 
  if (Request == SET_LINE_CODING)
    {
      //USART_Config();
      Request = 0;
    }

}

/*******************************************************************************
* Function Name  : MASS_Status_Out
* Description    : Mass Storage Status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CMPDevice_Status_Out(void)
{
  return;
}

/*******************************************************************************
* Function Name  : MASS_Data_Setup.
* Description    : Handle the data class specific requests..
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CMPDevice_Data_Setup(uint8_t RequestNo)
{
  uint8_t    *(*CopyRoutine)(uint16_t);
  
  CopyRoutine = NULL;
  
  if(signUSBMass)
  {
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01))
  {
    CopyRoutine = Get_Max_Lun;
  }
  else
  {
    return USB_UNSUPPORT;
  }
  
  }
  else
   {
   if (RequestNo == GET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = Virtual_Com_Port_GetLineCoding;
    }
  }
  else if (RequestNo == SET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = Virtual_Com_Port_SetLineCoding;
    }
    Request = SET_LINE_CODING;
  }
  
   }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);

  return USB_SUCCESS;
  
}

/*******************************************************************************
* Function Name  : MASS_NoData_Setup.
* Description    : Handle the no data class specific requests.
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CMPDevice_NoData_Setup(uint8_t RequestNo)
{
  if(signUSBMass)
  {
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == MASS_STORAGE_RESET) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00))
  {
    /* Initialize Endpoint 1 */
    ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    ClearDTOG_RX(ENDP2);

    /*initialize the CBW signature to enable the clear feature*/
    CBW.dSignature = BOT_CBW_SIGNATURE;
    Bot_State = BOT_IDLE;

    return USB_SUCCESS;
  }
  }
  else
  {
  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (RequestNo == SET_COMM_FEATURE)
    {
      return USB_SUCCESS;
    }
    else if (RequestNo == SET_CONTROL_LINE_STATE)
    {
      return USB_SUCCESS;
    }
  }
  
  }
  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : MASS_Get_Interface_Setting
* Description    : Test the interface and the alternate setting according to the
*                  supported one.
* Input          : uint8_t Interface, uint8_t AlternateSetting.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CMPDevice_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;/* in this application we don't have AlternateSetting*/
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;/*in this application we have only 1 interfaces*/
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : MASS_GetDeviceDescriptor
* Description    : Get the device descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CMPDevice_GetDeviceDescriptor(uint16_t Length)
{
   if(signUSBMass)
     return Standard_GetDescriptorData(Length, &Device_Descriptor_Mass );
   else
     return Standard_GetDescriptorData(Length, &Device_Descriptor_VCP );
	 
}

/*******************************************************************************
* Function Name  : MASS_GetConfigDescriptor
* Description    : Get the configuration descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CMPDevice_GetConfigDescriptor(uint16_t Length)
{
  if(signUSBMass)
     return Standard_GetDescriptorData(Length, &Config_Descriptor_Mass );
  else
     return Standard_GetDescriptorData(Length, &Config_Descriptor_VCP );

}

/*******************************************************************************
* Function Name  : MASS_GetStringDescriptor
* Description    : Get the string descriptors according to the needed index.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CMPDevice_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  uint8_t tempVal = 4;
  if(signUSBMass)
     tempVal = 5;
	 
  if (wValue0 > tempVal)
  {
    return NULL;
  }
  else
  {
   if(signUSBMass)
     return Standard_GetDescriptorData(Length, &String_Descriptor_Mass[wValue0]);
   else	
     return Standard_GetDescriptorData(Length, &String_Descriptor_VCP[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : Get_Max_Lun
* Description    : Handle the Get Max Lun request.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *Get_Max_Lun(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
    return 0;
  }
  else
  {
    return((uint8_t*)(&Max_Lun));
  }
}
//VCP

/*******************************************************************************
* Function Name  : Virtual_Com_Port_GetLineCoding.
* Description    : send the linecoding structure to the PC host.
* Input          : Length.
* Output         : None.
* Return         : Linecoding structure base address.
*******************************************************************************/
uint8_t *Virtual_Com_Port_GetLineCoding(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(uint8_t *)&linecoding;
}

/*******************************************************************************
* Function Name  : Virtual_Com_Port_SetLineCoding.
* Description    : Set the linecoding structure fields.
* Input          : Length.
* Output         : None.
* Return         : Linecoding structure base address.
*******************************************************************************/
uint8_t *Virtual_Com_Port_SetLineCoding(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(uint8_t *)&linecoding;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
