/* ==================================================================================

 File name:     LED_flicker.h
 Originator:    BLJ
 Description:   控制单个LED灯，实现1，2，3级故障码输出功能

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-26-2016		Version 1.0.0			正式版本
-----------------------------------------------------------------------------------*/



#include "stm32f0xx.h"


#define	LED_ON			1
#define	LED_OFF			0

#define	LED_ON_TIME		600					// 600ms
#define	LED_OFF_TIME	400					// 600ms
#define	LED_IDLE_TIME	2000				// 2000ms

struct LED_INPUT {
	u16	LEDLength1;
	u16	LEDLength2;
	u16	LEDLength3;
};

typedef struct {
	struct 	LED_INPUT	Input;
	u16	LEDONLength;
	u16	LEDOFFLength;
	u16	LEDWaitLength1;
	u16	LEDWaitLength2;
	u16	LEDStep;
	u16	LEDState;
	u16	DelayLength;
	u16	DelayCnt;
}LED_Flicker_structDef;

//void LED_Flicker_DeviceInit(void);
void LED_Flicker_Update(LED_Flicker_structDef* v);


// End of LED_Flicker.h												

