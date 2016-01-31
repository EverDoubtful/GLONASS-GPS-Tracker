
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WAVEPLAYER_H
#define __WAVEPLAYER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
#define SIZE_DAC_BUFF  512                  
#define FREQ_CLK       48000000             

typedef struct
{
  u8  Channels;                                                       
  u16 SamplesPerSec;     
  u8  BitsPerSample;     
  u32 Size;              
  u16 AvgBytesPerSec;    
} wp_format;

typedef enum
{
  WP_OK = 0,                  
  WP_ERROR_FILE,              
  WP_ERROR_READ_FILE,         
  WP_STOP,                    
  WP_PAUSE,                   
  WP_PLAY,                    
  WP_NONE,                    
  WP_ERROR                    
}WPRESULT;

WPRESULT       wp_open    ( FIL *file,  const char *FileName, wp_format *format );
void     wp_init    ( wp_format *format );
WPRESULT wp_play    ( const char *FileName );

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* __WAVEPLAYER_H */

