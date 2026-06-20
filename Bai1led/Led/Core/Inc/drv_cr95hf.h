/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __DRV_CR95HF_H
#define __DRV_CR95HF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define CR95HF_SPI_TIMEOUT_MAX      												1000		// miliseconds
#define CR95HF_TX_BUFFER_SIZE																128			//0xFF
#define CR95HF_RX_BUFFER_SIZE																0xFF		//0xFF
#define NFC_TAG_MEMORY_MAX_SIZE          										128

/* Exported constants --------------------------------------------------------*/
/** @defgroup XX95HF_Exported_Constants
  * @{
  */
/* RFtransceiver HEADER command definition ---------------------------------------------- */
#define CR95HF_CONTROL_SEND																	0x00
#define CR95HF_CONTROL_RESET																0x01
#define CR95HF_CONTROL_RECEIVE															0x02
#define CR95HF_CONTROL_POLLING															0x03
#define CR95HF_CONTROL_IDLE																	0x07

/* RFtransceiver mask and data to check the data (SPI polling)--------------------------- */
#define CR95HF_FLAG_DATA_READY															0x08
#define CR95HF_FLAG_DATA_READY_MASK													0x08

/* RF transceiver status	--------------------------------------------------------------- */
#define CR95HF_SUCCESS_CODE																	NO_ERROR
#define CR95HF_NOREPLY_CODE																	0x01
#define	CR95HF_ERRORCODE_DEFAULT														0xFE
#define	CR95HF_ERRORCODE_TIMEOUT														0xFD
#define CR95HF_ERRORCODE_POR																0x44

/* RF transceiver polling status	------------------------------------------------------- */
#define CR95HF_POLLING_CR95HF																0x00
#define CR95HF_POLLING_TIMEOUT															0x01

/* RF transceiver Offset of the command and the response -------------------------------- */
#define CR95HF_CONTROL_BYTE_OFFSET													0x00
#define CR95HF_LENGTH_OFFSET																0x01
#define CR95HF_DATA_OFFSET																	0x02

/* ECHO response ------------------------------------------------------------------------ */
#define CR95HF_ECHORESPONSE																	0x55

/* Sleep parameters --------------------------------------------------------------------- */
#define IDLE_SLEEP_MODE																			0x00
#define IDLE_HIBERNATE_MODE																	0x01
#define IDLE_CMD_LENTH																			0x0E

#define WU_TIMEOUT																					0x01
#define WU_TAG																							0x02
#define WU_FIELD																						0x04
#define WU_IRQ																							0x08
#define WU_SPI																							0x10

#define HIBERNATE_ENTER_CTRL																0x0400
#define SLEEP_ENTER_CTRL																		0x0100
#define SLEEP_FIELD_ENTER_CTRL															0x0142

#define HIBERNATE_WU_CTRL																		0x0400
#define SLEEP_WU_CTRL																				0x3800

#define LEAVE_CTRL																					0x1800

/* Calibration parameters---------------------------------------------------------------- */
#define WU_SOURCE_OFFSET																		0x02
#define WU_PERIOD_OFFSET																		0x09
#define DACDATAL_OFFSET																			0x0C
#define DACDATAH_OFFSET																			0x0D
#define NBTRIALS_OFFSET																			0x0F


#define LISTEN																							0x05
#define DUMMY_BYTE																					0xFF


/* ROM CODE Revision  --------------------------------------------------------------*/
#define CR95HF_ROM_CODE_REVISION_OFFSET            					13

/* RFtrans 95HF family command definition  ---------------------------------------------------------------*/
#define CR95HF_COMMAND_IDN																	0x01
#define CR95HF_COMMAND_PROTOCOL_SELECT 											0x02
#define CR95HF_COMMAND_POLL_FIELD 													0x03
#define CR95HF_COMMAND_SEND_RECEIVE													0x04
#define CR95HF_COMMAND_LISTEN																0x05
#define CR95HF_COMMAND_SEND																	0x06
#define CR95HF_COMMAND_IDLE																	0x07
#define CR95HF_COMMAND_READ_REGISTER												0x08
#define CR95HF_COMMAND_WRITE_REGISTER												0x09
#define CR95HF_COMMAND_BAUD_RATE														0x0A
#define CR95HF_COMMAND_SUB_FREQ_RES													0x0B
#define CR95HF_COMMAND_AC_FILTER														0x0D
#define CR95HF_COMMAND_TEST_MODE														0x0E
#define CR95HF_COMMAND_SLEEP_MODE														0x0F
#define CR95HF_COMMAND_ECHO																	0x55



/* Analogue configuration register for protocol initialization -------------------------------------------*/
#define TIMER_WINDOW_REG_ADD                0x3A
#define TIMER_WINDOW_UPDATE_CONFIRM_CMD     0x04


#define AFE_ANALOG_CONF_REG_SELECTION       0x68
#define AFE_ANALOG_CONF_REG_UPDATE          0x69

#define AFE_ACCONFIGA_OFFSET                0x04

/* ROM CODE Revision  --------------------------------------------------------------*/
#define ROM_CODE_REVISION_OFFSET            13


typedef enum {
	CR95HF_IC_VERSION_QJA = 0x30,
	CR95HF_IC_VERSION_QJB,
	CR95HF_IC_VERSION_QJC,
	CR95HF_IC_VERSION_QJD,
	CR95HF_IC_VERSION_QJE
}CR95HF_IC_VERSION;


/**
  * @}
  */
  

uint32_t CR95HF_HWInit (uint8_t * icVersion, uint8_t ** resp);
uint32_t CR95HF_Set_FieldOff(void);
uint32_t CR95HF_SelectProtocol_ISO15693(const uint8_t parameters, uint8_t **pResponse);
uint32_t CR95HF_AjustAnalogRegister(uint8_t ** resp);
uint32_t CR95HF_Inventory(uint8_t ** resp, uint16_t * rcvLenght);
uint32_t CR95HF_GetCardSystemInfo(uint8_t ** resp, uint16_t * rcvLenght);
uint32_t CR95HF_Read_First16Block(uint8_t ** rcvData, uint16_t * rcvLenght);
uint32_t CR95HF_Write_First16Block(uint8_t * blocksData);
uint32_t CR95HF_SendReceive(const uint8_t *pCommand, uint8_t ** pTResponse);
uint32_t CR95HF_PORsequence( void );
uint32_t CR95HF_ReadIDN(uint8_t **pTResponse);
uint32_t CR95HF_GetUID(uint8_t uid[8]);
uint32_t CR95HF_ReadSingleBlock(uint8_t blockAddr, uint8_t *blockData, uint16_t *rcvLength);


	/*
	else if(uCR95HF_RxBuffer[0] == 0x86)
	{
		return ERROR_NFC_COMMUNICATION;
	}
	else if(uCR95HF_RxBuffer[0] == 0x87)
	{
		return ERROR_NFC_FRAME_WAIT_TIMEOUT_OR_NO_TAG;
	}
	else if(uCR95HF_RxBuffer[0] == 0x88)
	{
		return ERROR_NFC_INVALID_SOF;
	}	
	else if(uCR95HF_RxBuffer[0] == 0x89)
	{
		return ERROR_NFC_RECEIVE_BUFFER_OVERFLOW;
	}		
	else if(uCR95HF_RxBuffer[0] == 0x8A)
	{
		return ERROR_NFC_FRAMING_ERROR;
	}			
	else if(uCR95HF_RxBuffer[0] == 0x8B)
	{
		return ERROR_NFC_EGT_TIMEOUT;
	}			
	else if(uCR95HF_RxBuffer[0] == 0x8C)
	{
		return ERROR_NFC_INVALID_LENGTH;
	}			
	else if(uCR95HF_RxBuffer[0] == 0x8D)
	{
		return ERROR_NFC_CRC_ERROR;
	}	
	else if(uCR95HF_RxBuffer[0] == 0x8E)
	{
		return ERROR_NFC_RECEPTION_LOST_WITHOUT_EOF;
	}	*/

/* RFtrans 95HF family command definition  ---------------------------------------------------------------*/
#define IDN																					0x01
#define PROTOCOL_SELECT 														0x02
#define POLL_FIELD 																	0x03
#define SEND_RECEIVE																0x04
#define LISTEN																			0x05
#define SEND																				0x06
#define IDLE																				0x07
#define READ_REGISTER																0x08
#define WRITE_REGISTER															0x09
#define BAUD_RATE																		0x0A
#define SUB_FREQ_RES																0x0B
#define AC_FILTER																		0x0D
#define TEST_MODE																		0x0E
#define SLEEP_MODE																	0x0F
#define ECHO																				0x55

/*  poll field status ------------------------------------------------------------------ */
#define POLLFIELD_RESULTSCODE_OK										0x00



//  Send command field status
#define SEND_RESULTSCODE_OK													0x00
#define SEND_ERRORCODE_LENGTH												0x82
#define SEND_ERRORCODE_PROTOCOL											0x83
/*  Idle command field status ----------------------------------------------------------------- */
#define IDLE_RESULTSCODE_OK													0x00
#define IDLE_ERRORCODE_LENGTH												0x82
/*  read register command field status -------------------------------------------------------- */
#define READREG_RESULTSCODE_OK											0x00
#define READREG_ERRORCODE_LENGTH										0x82
/*  write register command field status ------------------------------------------------------- */
#define WRITEREG_RESULTSCODE_OK											0x00
/*  Baud rate command field status ------------------------------------------------------------ */
#define BAUDRATE_RESULTSCODE_OK											0x55
/*  AC filter command field status ------------------------------------------------------------ */
#define ACFILTER_RESULTSCODE_OK											0x00
#define ACFILTER_ERRORCODE_LENGTH										0x82
/*  sub freq command field status ------------------------------------------------------------- */
#define SUBFREQ_RESULTSCODE_OK											0x00
/*  Test mode command field status ------------------------------------------------------------ */
#define TESTMODE_RESULTSCODE_OK											0x00

#define ASK_FOR_SESSION											0x0000
#define TAKE_SESSION												0xFFFF

#define XX95_ACTION_COMPLETED							  0x9000	

/* Error codes */
#define PCDNFCT5_OK 											1
#define PCDNFCT5_ERROR 										2
#define PCDNFCT5_ERROR_MEMORY_TAG					3
#define PCDNFCT5_ERROR_MEMORY_INTERNAL		4
#define PCDNFCT5_ERROR_LOCKED 						5
#define PCDNFCT5_ERROR_NOT_FORMATED				6

/* Extended commands defined by the NFC Forum Type5 */
#define PCDNFCT5_CMDCODE_EXTREADSINGLEBLOCK		0x30
#define PCDNFCT5_CMDCODE_EXTWRITESINGLEBLOCK	0x31
#define PCDNFCT5_CMDCODE_EXTENDEDLOCKBLOCK    0x32
#define PCDNFCT5_CMDCODE_EXTREADMULBLOCKS			0x33
#define PCDNFCT5_CMDCODE_EXTENDEDGETSYSINFO		0x3B
/**
  * @}
  */	 

	 
#ifdef __cplusplus
}
#endif

#endif /* __DRV_CR95HF_H */
