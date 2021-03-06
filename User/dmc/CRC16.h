/* =================================================================================

 File name:		CRC16.h              
 Originator:	BLJ
 Description: 	Header file containing function prototypes for the CRC16.
 
=====================================================================================
 History:
-------------------------------------------------------------------------------------
 03-07-2015		Version 1.0.0
-----------------------------------------------------------------------------------*/


#ifndef __CRC16_H__
#define __CRC16_H__

#include "stm32f0xx.h"


//get CRC16
u16 GetCRC16(u8* pMsg, u16 wDataLen);

#endif

//===========================================================================
// No more.
//===========================================================================


