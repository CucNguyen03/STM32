/**
  ******************************************************************************
  * @file    miscellaneous.h
  * @author  MMY Application Team
  * @version V0.1
  * @date    15/03/2011
  * @brief   
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  */ 


/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32f10x
  * @{
  */
    
#ifndef __CR95HF_MISC_H
#define __CR95HF_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup Library_configuration_section
  * @{
  */
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */	 
		 
#ifndef MAX
#define MAX(x,y) 				((x > y)? x : y)
#endif
#ifndef MIN
#define MIN(x,y) 				((x < y)? x : y)
#endif
#define ABS(x) 					((x)>0 ? (x) : -(x))  
#define CHECKVAL(val, min,max) 	((val < min || val > max) ? false : true) 
	 
#define GETMSB(val) 		((val & 0xFF00 )>>8 ) 
#define GETLSB(val) 		( val & 0x00FF ) 
 
#define RESULTOK							0x00 
#define ERRORCODE_GENERIC			1 

//#ifndef errchk
//#define errchk(fCall) if (status = (fCall), status != RESULTOK) \
//	{goto Error;}
//#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

struct X; /* incomplete ("forward") declaration */
struct Thing {
	int i;
	struct X* x_ptr;
};



uint8_t HexIntToChar (uint8_t HexInt);
int8_t HexToString (uint8_t *HexString, uint8_t NbVal, char *ASCIIString);

uint16_t Uint8ToUint16 (uint8_t *data);
void Uint16ToUint8 (uint16_t data, uint8_t *uint8tab);
uint32_t Uint8ToUint32 (uint8_t *data);
void Uint32ToUint8 (uint32_t data, uint8_t *uint8tab);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_H */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
