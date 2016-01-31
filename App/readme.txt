/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
// TODO

1. usb_desc.h - should merge - they are different and do not cross
2. usb_conf.h - should make new vars to differ some values during run-time
3. usb_prop.h - should merge - they are different and do not cross
4. usb_pwr.h   - there 2 include files in mass stor and no in VCP (why?) 

// 31.03.12

SDIO_TRANSFER_CLK_DIV - defferent values in projects(mass storage and SDIO example)	- 
необходимо выяснить

//this is version fatfs + usb_mass_VCP - intermediate


#ifdef __cplusplus
 extern "C" {
#endif



#ifdef __cplusplus
}
#endif

//packet when GLN=0

$GPGGA,084730.00,5546.8634,N,04907.9700,E,1,14,0.7,00091.5,M,0003.6,M,,*61
$GPGSA,A,3,01,31,04,30,20,23,25,13,72,65,82,81,1.3,0.7,1.1*30
$GPGSV,6,1,23,01,23,202,38,04,17,321,50,11,03,197,35,13,15,252,35*7C
$GPGSV,6,2,23,14,03,074,,16,01,140,27,17,02,275,,20,75,231,41*7E
$GPGSV,6,3,23,23,46,254,45,25,14,028,42,30,19,122,18,31,48,065,36*79
$GPGSV,6,4,23,32,63,142,,65,63,310,49,66,09,301,43,71,04,118,*76
$GPGSV,6,5,23,72,56,111,21,73,23,031,32,74,64,082,,75,37,174,*76
$GPGSV,6,6,23,81,23,311,48,82,12,003,37,88,08,260,31*4F
$GPRMC,084730.00,A,5546.8634,N,04907.9700,E,000.08338,310.8,020412,,,A*56

//todo 

gps-gln library rework

STM32F10X_HD, - settings from c/c++ page

jX8pY23Ee5

//todo
sdcarduser:
   analyse time and create new directory and file
   
AT+SAPBR=3,1,"CONTYPE","GPRS"
AT+SAPBR=3,1,"APN","internet.mts.ru"
AT+SAPBR=3,1,"USER","mts"
AT+SAPBR=3,1,"PWD","mts"
AT+SAPBR=1,1

at+httpinit
at+httppara="CID",1
at+httppara="URL","31.13.128.67/?param1=test"
at+httpaction=0
at+httpread
at+httppara="URL","www.twitter.com/enter?login=alexKZN803&password=qWeRt07455"
at+httpterm

at+httppara="URL","www.easycounter.com/counter.php?peopleman" -really works


/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

packet to ask gen info
40 5e 13 09 00 01 00 04 00 ff ff ff ff 58 5d ad 6b 24
@	   id	  cmd aux len	  data		  crc		$

packet by USB for reading config

254, 3, 0, 0, 0, 196, 13, 60, 132

todo 1. sdcard: should make pos of log file in RTC saving.
	 2. make timeLabel (4 bytes) to write instead of string from rtcGetTime function

REGULAR EXPRESSIONS
((\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})) - ip

[0-9a-zA-Z]{3,32} - letters and digits from 3-32

[0-9]{11,13} - only digits (11-13)


1,N,N,20,Params,,0,10,1,0,0
N,10,N,2,ID block,594782,1,1,4,1,999999
N,10,1,3,Server Sending Interval,5,1,2,3,5,600
N,10,2,4,Battery  Sending Interval,35,1,3,3,30,600
N,10,3,5,Track Control,0,1,4,0,0,1
N,10,4,6,Velocity Trigger(km/h),63,1,5,3,5,600
N,10,5,7,Angle Trigger,15,1,6,3,5,360
N,10,6,N,Distance Trigger,1000,1,7,3,10,50000
21,N,10,N,GPRS,,0,20,1,0,0
30,20,N,22,Master,,0,21,1,0,0
N,21,N,31,APN1,internet.mts.ru,1,30,1,5,32
N,21,30,32,UserName1,mtc,1,31,1,3,32
N,21,31,33,Password1,mts,1,32,1,3,32
N,21,32,34,Server1,85.233.64.176,1,33,1,3,32
N,21,33,35,Port1,8085,1,34,3,1,65536
N,21,34,36,SMS Number1,+79196422100,1,35,1,5,32
N,21,35,N,Control Number1,+79196422100,1,36,1,5,32
50,20,21,N,Slave,,0,22,1,0,0
N,22,N,51,APN2,internet.mts.ru,1,50,1,5,32
N,22,50,52,UserName2,mts,1,51,1,3,32
N,22,51,53,Password2,mts,1,52,1,3,32
N,22,52,54,Server2,85.233.64.176,1,53,1,3,32
N,22,53,55,Port2,8084,1,54,3,1,65536
N,22,54,56,SMS Number2,+79196422100,1,55,1,5,32
N,22,55,N,Control Number2,+79196422100,1,56,1,5,32

#if defined (VER_3)
#endif


#if defined (VER_3)
#else
#endif


//py2exe
copy all python files to dir with setup.py
its content:

from distutils.core import setup
import py2exe
 
setup(
    windows=[{"script":"camtech.py"}],
    options={"py2exe": {"includes":["sip"]}}
)

Further run:
python setup.py py2exe



----------------------
at+cpol?

+CPOL: 1,0,"TELIA S"

OK
----------------
at+cops?

+COPS: 0,0,"MegaFon RUS"

OK

-------------
at+cops=?

+COPS: (2,"MegaFon","","25002"),(3,"MTS-RUS","MTS","25001"),(3,"RUS BMT","BMT","25007"),(3,"Beeline","Beeline","25099"),(3,"","","25003"),,(0,1,4),(0,1,2)

OK


COMMANDS from USB by terminal
1.get navi [254, 15, 0, 0, 0, 209, 203, 153, 206] - fe 0f 00 00 00 D1 CB 99 CE
2. get congig:         fe 03 00 00 00 c4 0D 3C 84


fact:  DMA from memory to usart sends 1024 for 132ms


chernovic
/*
  u8 rbd[16] = {0};
  u8 deb[16] = {0};
  u8 i = 0;
  u16 len;
  while(i < 16) 
     deb[i] = ++i;
  i = 0;
  fifo_buffer rbdf;
  FIFO_Init(&rbdf, rbd, sizeof(rbd));
  while(i < 8)
     FIFO_Put(&rbdf, i++);
  i = 0;
  while(i++ < 8)
     FIFO_Get(&rbdf);
  i = 0;
  while(i < 12)
     FIFO_Put(&rbdf, i++);
  len = FIFO_Count(&rbdf);
  //t = rbdf.tail;
  //memcpy(deb,(const u8 *) &rbdf.buffer[t],FIFO_Count(&rbdf));
  memcpy(deb, FIFO_PtrStart(&rbdf),FIFO_Count(&rbdf));
  //FIFO_memcpy(deb, &rbdf);

  if(!len)
      NVIC_Configuration();	//for  all peripheria
*/    


/*
timer6Init();
u16 temp, t2, arr = 50000;
u8 deb[1024] = {0};
deb[0] = 'H';
deb[1] = 'e';
deb[2] = 'l';
deb[3] = 'l';
deb[4] = 'o';
deb[5] = 'w';
deb[6] = 'o';
deb[7] = 'r';
deb[8] = 'l';
deb[9] = 'd';
deb[511] = 't';
deb[1023] = 'e';
  USART2_TX_DMA_Init(deb);
  StartDMAChannel7(1024);                 //запустить первую передачу

  temp = TIM6->CNT;

  while(GetStateDMAChannel7()==0) {};   //ждать окончания передачи

  t2 = TIM6->CNT;
  if(t2 >= temp)       t2 = t2 - temp;
  else                 t2 = arr - temp + t2;
  
  if(t2)
     led_dn(BOARD_LED_OFF);

//StartDMAChannel7(10);                 //запустить вторую передачу
//while(GetStateDMAChannel7()==0) {};   //ждать окончания передачи
*/
/*
  timer6Init();
  u16 temp, t2, arr = 50000;
  memset(bufTest,5,sizeof(bufTest));
  temp = TIM6->CNT;
  //cmdSend((char *)"HELLO WORLD");
  cmdSendLen(bufExtBin, 64);
  //memcpy(bufExtBin,bufTest,512);
  //Delay(1);
  t2 = TIM6->CNT;
  if(t2 >= temp)       t2 = t2 - temp;
  else                t2 = arr - temp + t2;
  

  if(t2)
     led_dn(BOARD_LED_OFF);
*/
21120 strings in project exzcept .h