
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MENU_H
#define __MENU_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
  /* menu */
#define SIZE_MENU_NAME            32


typedef enum 
{
  BOOL_TYPE_DATA,  /* bool 0 or 1*/
  CHAR_TYPE_DATA,  /* array of char*/
  U8_TYPE_DATA,     /* 1 byte */
  U16_TYPE_DATA,    /* 2 byte */
  U32_TYPE_DATA,    /* 4 byte */
  GUARD_TYPE_DATA   /* alarm */
}typeData;

/* IMPORTANT: name cannot be repeated - it MUST be unique for each menu item*/
__packed struct menu_item
{
  const struct        menu_item *child;
  const struct        menu_item *parent;
  const struct        menu_item *up;
  const struct        menu_item *down;
  const u8            name[SIZE_MENU_NAME];     
  //here comes the value in that position(5)  
  const u8            edit;
  const u8            id;
  const u8            typeData;  /* bool, char, digit*/
  const u8            min;  /*for char : it's min number of characters, for digits - it's min value*/
  const u32           max;  /*for char : it's max number of characters, for digits - it's max value*/
  //void (* on_enter) (void);
};
#if 0
__packed struct menu_itemCV  /*CV - current Values*/
{
  const struct        menu_item *child;
  const struct        menu_item *parent;
  const struct        menu_item *up;
  const struct        menu_item *down;
  const u8            name[SIZE_MENU_NAME];     
  //here comes the value in that position(5)  
  const u8            id;
  const u8            typeData;  /* bool, char, digit*/
};
#endif				      
/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
//u16 menuDisplay(u8 * buf);
u16  menuDisplay(u8 * buf, u8 * signLongPack, u16 limit);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
