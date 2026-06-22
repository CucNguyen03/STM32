#include "drv_cr95hf.h"
#include "main.h"
#include "stdint.h"
#include "string.h"
#include "debug.h"
/* External variables --------------------------------------------------------*/
extern SPI_HandleTypeDef *hspi_nfc_reader;
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
static uint8_t	uCR95HF_RxBuffer [CR95HF_RX_BUFFER_SIZE];
/* set state on SPI_NSS pin */
#define CR95HF_NSS_LOW() 						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define CR95HF_NSS_HIGH()  					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
/* set state on IRQ_In pin */
#define CR95HF_IRQIN_LOW() 					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)	 
#define CR95HF_IRQIN_HIGH()					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)	
//char  buffer1[64];	
/* Private macros ------------------------------------------------------------*/
 


static void CR95HF_DelayMs(uint32_t miliseconds)
{
	HAL_Delay(miliseconds);			
}

static uint8_t CR95HF_SendReceiveByte(uint8_t data) 
{	
  uint8_t txData[1], rxData[1];
	txData[0] = data;
	rxData[0] = 0;
  HAL_StatusTypeDef halRet = HAL_SPI_TransmitReceive(hspi_nfc_reader, txData, rxData, 1, CR95HF_SPI_TIMEOUT_MAX);
  if(halRet != HAL_OK)
	{
		ASSERT_FAIL();					
	}
  return rxData[0];
}

static void CR95HF_SendReceiveBuffer(uint8_t *pCommand, uint8_t length, uint8_t *pResponse) 
{
  HAL_StatusTypeDef halRet = HAL_SPI_TransmitReceive(hspi_nfc_reader, pCommand, pResponse, length, CR95HF_SPI_TIMEOUT_MAX);
  if(halRet != HAL_OK)
	{
		ASSERT_FAIL();				
	}
}

static void CR95HF_SendIRQINPulse(void)
{
	/* Send a pulse on IRQ_IN */
	CR95HF_IRQIN_HIGH() ;
	CR95HF_DelayMs(1);
	CR95HF_IRQIN_LOW() ;
	CR95HF_DelayMs(1);
	CR95HF_IRQIN_HIGH();
  /* Need to wait 10ms after the pulse before to send the first command */
  CR95HF_DelayMs(100); 
}


static void CR95HF_ResetSPI(void)
{	
  /* Deselect Rftransceiver over SPI */
  CR95HF_NSS_HIGH();
  CR95HF_DelayMs(1);
  /* Select 95HF device over SPI */
  CR95HF_NSS_LOW();
  /* Send reset control byte	*/
  CR95HF_SendReceiveByte(CR95HF_CONTROL_RESET);
  /* Deselect 95HF device over SPI */
  CR95HF_NSS_HIGH();
  CR95HF_DelayMs(3);
  /* send a pulse on IRQ_in*/
  CR95HF_SendIRQINPulse();
  CR95HF_DelayMs(10);	
}

static uint32_t CR95HF_SendSPICommand(const uint8_t *pData)
{ 
	/* Select xx95HF over SPI */
  CR95HF_NSS_LOW();
  
  /* Send a sending request to xx95HF  */
  CR95HF_SendReceiveByte(CR95HF_CONTROL_SEND);
  
  if(*pData == CR95HF_COMMAND_ECHO)
  {
    /* Send a sending request to xx95HF */ 
    CR95HF_SendReceiveByte(CR95HF_COMMAND_ECHO);
  }
  else
  {
    /* Transmit the buffer over SPI */
		uint16_t xferLenght = pData[CR95HF_LENGTH_OFFSET] + CR95HF_DATA_OFFSET;
		if(xferLenght <= CR95HF_RX_BUFFER_SIZE)
		{
			uint8_t dummyRxBuffer[CR95HF_RX_BUFFER_SIZE];						
			CR95HF_SendReceiveBuffer((uint8_t *)pData, xferLenght, dummyRxBuffer);
		}
		else
		{
			ASSERT_FAIL();	
			//Debug_Printf("\n\r-I-End CR95HF_SendSPICommand, ERROR_NFC_INVALID_LENGTH");		
//		sprintf(buffer1, "\r\n-E-ERROR_NFC_INVALID_LENGTH");		
//    Debug_Show(buffer1)	;
//		HAL_Delay(100);
			return ERROR_NFC_INVALID_LENGTH;
		}
  }
  
  /* Deselect xx95HF over SPI  */
  CR95HF_NSS_HIGH();
	return NO_ERROR;
}

static uint32_t CR95HF_SPIPollingCommand( void )
{
  uint8_t pollingStatus = 0;
  uint8_t counter = 0;
	// Polling timeout = 300ms
	/* Low level on NSS  */
	CR95HF_NSS_LOW();	
	do
	{	
		/*  poll the CR95HF transceiver until he's ready ! */
		pollingStatus  = CR95HF_SendReceiveByte(CR95HF_CONTROL_POLLING);
		if((pollingStatus & CR95HF_FLAG_DATA_READY_MASK)	== CR95HF_FLAG_DATA_READY)
		{
			/* High level on NSS  */
			CR95HF_NSS_HIGH();
			return NO_ERROR;	
		}
		CR95HF_DelayMs(10);										
	}
	while(( pollingStatus	!= CR95HF_FLAG_DATA_READY) && (counter++ < 30));
	CR95HF_NSS_HIGH();	
	return ERROR_TIMEOUT;	
}

static uint32_t CR95HF_ReceiveSPIResponse(uint8_t ** pTResponse)
{
  uCR95HF_RxBuffer[0] = CR95HF_ERRORCODE_DEFAULT;
  uCR95HF_RxBuffer[1] = 0x00;
	*pTResponse = uCR95HF_RxBuffer;	
  /* Select 95HF transceiver over SPI */
  CR95HF_NSS_LOW();
  
  /* Request a response from 95HF transceiver */
  CR95HF_SendReceiveByte(CR95HF_CONTROL_RECEIVE);
  
  /* Recover the "Command" byte */
  uCR95HF_RxBuffer[CR95HF_CONTROL_BYTE_OFFSET] = CR95HF_SendReceiveByte(DUMMY_BYTE);
  
  if(uCR95HF_RxBuffer[CR95HF_CONTROL_BYTE_OFFSET] == CR95HF_ECHORESPONSE)
  {
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET]  = 0x00;
    /* In case we were in listen mode error code cancelled by user (0x85 0x00) must be retrieved */
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET+1] = CR95HF_SendReceiveByte(DUMMY_BYTE);
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET+2] = CR95HF_SendReceiveByte(DUMMY_BYTE);
  }
  else if(uCR95HF_RxBuffer[CR95HF_CONTROL_BYTE_OFFSET] == 0xFF)
  {
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET]  = 0x00;
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET+1] = CR95HF_SendReceiveByte(DUMMY_BYTE);
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET+2] = CR95HF_SendReceiveByte(DUMMY_BYTE);
  }
  else
  {
    /* Recover the "Length" byte */
    uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET]  = CR95HF_SendReceiveByte(DUMMY_BYTE);
    /* Checks the data length */
    if(uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET] != 0x00)
		{
			uint16_t xferLenght = uCR95HF_RxBuffer[CR95HF_LENGTH_OFFSET];
			if(xferLenght <= CR95HF_TX_BUFFER_SIZE)
			{
				uint8_t dummyTxBuffer[CR95HF_TX_BUFFER_SIZE];			
				memset(dummyTxBuffer, (uint8_t)DUMMY_BYTE, xferLenght);
				CR95HF_SendReceiveBuffer(dummyTxBuffer, xferLenght, &uCR95HF_RxBuffer[CR95HF_DATA_OFFSET]);
			}
			else
			{
				ASSERT_FAIL();
				return ERROR_NFC_INVALID_LENGTH;				
			}
		}
  }
  
  /* Deselect xx95HF over SPI */
  CR95HF_NSS_HIGH();	
	return NO_ERROR;
}

uint32_t CR95HF_SendReceive(const uint8_t *pTCommand, uint8_t ** pTResponse)
{	
	uint32_t ret;
	/* Assign resp pointer to allocated buffer, in case of exception happen before resp is really assign */
  uCR95HF_RxBuffer[0] = CR95HF_ERRORCODE_DEFAULT;
  uCR95HF_RxBuffer[1] = 0x00;	
	*pTResponse = uCR95HF_RxBuffer;
	
	/* First step  - Sending command 	*/
	ret = CR95HF_SendSPICommand(pTCommand);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	/* Second step - Polling	*/
	if (CR95HF_SPIPollingCommand() != NO_ERROR)
	{	
		return ERROR_TIMEOUT;	
	}
	/* Third step  - Receiving bytes */
	ret = CR95HF_ReceiveSPIResponse(pTResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}	
  return NO_ERROR; 
}

uint32_t CR95HF_PORsequence( void )
{
	uint8_t * pTResp;
  uint16_t NthAttempt=0;
  const uint8_t command[]= {CR95HF_COMMAND_ECHO};
  
  /* Power up sequence: Pulse on IRQ_IN to select UART or SPI mode */
  CR95HF_SendIRQINPulse();
  
  /* SPI Reset */
  CR95HF_ResetSPI();		

  do
  {
    /* send an ECHO command and checks response */
    if(CR95HF_SendReceive(command, &pTResp) == NO_ERROR)
		{
			if(pTResp[0] == CR95HF_ECHORESPONSE)
			{
				return NO_ERROR;	
			}
		}
    /* if the SPI interface is selected then send a reset command*/
    CR95HF_ResetSPI();		
  } while (pTResp[0] != CR95HF_ECHORESPONSE && NthAttempt++ <5);
  
  return ERROR_TIMEOUT;
}

uint32_t CR95HF_ReadIDN(uint8_t **pTResponse)
{
  const uint8_t cmdToSend[] = {CR95HF_COMMAND_IDN	,0x00};
  
  /* send the command to the PICC and retrieve its response */
  if(CR95HF_SendReceive(cmdToSend, pTResponse) != NO_ERROR)
	{
		return ERROR_TIMEOUT;
	}
	if(	((*pTResponse)[CR95HF_ROM_CODE_REVISION_OFFSET] == CR95HF_IC_VERSION_QJA) || 
			((*pTResponse)[CR95HF_ROM_CODE_REVISION_OFFSET] == CR95HF_IC_VERSION_QJB) || 
			((*pTResponse)[CR95HF_ROM_CODE_REVISION_OFFSET] == CR95HF_IC_VERSION_QJC) || 
			((*pTResponse)[CR95HF_ROM_CODE_REVISION_OFFSET] == CR95HF_IC_VERSION_QJD) || 
		((*pTResponse)[CR95HF_ROM_CODE_REVISION_OFFSET] == CR95HF_IC_VERSION_QJE))
	{
		return NO_ERROR;
	}
	else
	{
		return ERROR_NFC_INVALID_IC_VERSION;
	}
}


uint32_t CR95HF_HWInit (uint8_t * icVersion, uint8_t ** chipIdnResponse)
{
	uint32_t ret;
  /* initilialize the RF transceiver */
  if (CR95HF_PORsequence( ) != NO_ERROR)
  {
    /* nothing to do, this is a trap for debug purpose you can use it to detect HW issue */
    /* or GPIO config issue */
		return ERROR_TIMEOUT;
  }
  /* Retrieve the IC version of the chip */
	ret = CR95HF_ReadIDN(chipIdnResponse);
  if(ret != NO_ERROR)
	{
		return ret;
	}
  *icVersion = (uint8_t) ((* chipIdnResponse)[CR95HF_ROM_CODE_REVISION_OFFSET]);
  return NO_ERROR;
}

uint32_t CR95HF_Echo()
{
	uint8_t * pTResp;	
  const uint8_t command[]= {CR95HF_COMMAND_ECHO};

	// send an ECHO command and checks response
	if(CR95HF_SendReceive(command, 	&pTResp) == NO_ERROR)
	{
		if(pTResp[0] == CR95HF_ECHORESPONSE)
		{
			return NO_ERROR;	
		}
	}
	else
	{
		return ERROR_TIMEOUT;
	}
	return ERROR_TIMEOUT;
}

uint32_t CR95HF_Set_FieldOff( void )
{
	// Check communication
	uint32_t ret = CR95HF_Echo();
	if (ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_SELECT_FIELD_OFF, "CR95HF_Echo", ret);				
		/* reset the device */
		if (CR95HF_PORsequence( ) != NO_ERROR)
		{
			return ERROR_TIMEOUT;
		}
	}
	
	uint8_t * pTResp;		
	const uint8_t cmdToSend[] = {CR95HF_COMMAND_PROTOCOL_SELECT,0x02,0x00,0x00};

	if(CR95HF_SendReceive(cmdToSend, &pTResp) != NO_ERROR)
	{
		return ERROR_TIMEOUT;
	}
	if(pTResp[0] == 0)
	{
		return NO_ERROR;
	}	
	else if(pTResp[0] == 0x82)
	{
		return ERROR_NFC_INVALID_COMMAND_LENGHT;
	}
	else if(pTResp[0] == 0x83)
	{
		return ERROR_NFC_INVALID_PROTOCOL;
	}
	else
	{
		//Debug_Logout_Error(ERROR_NFC_SELECT_FIELD_OFF, "CR95HF_SendReceive", pTResp[0]);	
////		sprintf(buffer1, "\r\n-E-CR95HF_SendReceive,: pTResp[0] : %d", pTResp[0]);		
////    Debug_Show(buffer1)	;
////		HAL_Delay(100);
		return pTResp[0];
	}	
}


uint32_t CR95HF_SelectProtocol_ISO15693(const uint8_t parameters,uint8_t ** pTResponse)
{	
	uint32_t ret = CR95HF_Echo();
	if (ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_SELECT_PROTOCOL, "CR95HF_Echo", ret);			
//		sprintf(buffer1, "\r\n-E-CR95HF_Echo,: %d",ret);		
//    Debug_Show(buffer1)	;
//		HAL_Delay(100);		
		/* reset the device */
		if (CR95HF_PORsequence( ) != NO_ERROR)
		{
			return ERROR_TIMEOUT;
		}
	}		
	
uint8_t selectProtocolCmd[] = {CR95HF_COMMAND_PROTOCOL_SELECT,0x02,0x01,0x01};
	selectProtocolCmd[3] = parameters;
  if(CR95HF_SendReceive(selectProtocolCmd, pTResponse) != NO_ERROR)
	{
		return ERROR_TIMEOUT;
	}	
		
	if((*pTResponse)[0] == 0)
	{
		return NO_ERROR;
	}
	else
	{
		return(*pTResponse)[0];
	}
}
uint32_t CR95HF_PollField(void)
{
    uint8_t cmd[] =
    {
        CR95HF_COMMAND_POLL_FIELD,
        0x00
    };

    uint8_t *resp;

    return CR95HF_SendReceive(cmd,&resp);
}
uint32_t CR95HF_AjustAnalogRegister(uint8_t ** resp)
{
	
	const uint8_t adjustAnalogCmd[] = {CR95HF_COMMAND_WRITE_REGISTER,0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,0x01,0xD0}; // Last 02 bytes gain parameters = {0x01, 0xD0}; 	// 0x01, 0xD1

	// Send inventory command
  if(CR95HF_SendReceive(adjustAnalogCmd, resp) != NO_ERROR)
	{
		return ERROR_TIMEOUT;
	}	
	
	if((*resp)[0] == 0)
	{
		return NO_ERROR;
	}
	else
	{
		return (*resp)[0];
	}
}

uint32_t CR95HF_Inventory(uint8_t **resp, uint16_t *rcvLength)
{
    uint8_t *pResp;

    const uint8_t cmd[] =
    {
        CR95HF_COMMAND_SEND_RECEIVE,
        0x03,
        0x26,
        0x01,
        0x00
    };

    if(CR95HF_SendReceive(cmd, &pResp) != NO_ERROR)
    {
        return ERROR_TIMEOUT;
    }

    /*
     * CR95HF success = 0x80
     */
    if(pResp[0] != 0x80)
    {
        switch(pResp[0])
        {
            case 0x86: return ERROR_NFC_COMMUNICATION;
            case 0x87: return ERROR_NFC_FRAME_WAIT_TIMEOUT_OR_NO_TAG;
            case 0x88: return ERROR_NFC_INVALID_SOF;
            case 0x89: return ERROR_NFC_RECEIVE_BUFFER_OVERFLOW;
            case 0x8A: return ERROR_NFC_FRAMING_ERROR;
            case 0x8B: return ERROR_NFC_EGT_TIMEOUT;
            case 0x8C: return ERROR_NFC_INVALID_LENGTH;
            case 0x8D: return ERROR_NFC_CRC_CHECK_FAILED;
            case 0x8E: return ERROR_NFC_RECEPTION_LOST_WITHOUT_EOF;
            default:
                return ERROR_NFC_ISO15693_DEFAULT;
        }
    }

    *resp = pResp;
    *rcvLength = pResp[1];

    return NO_ERROR;
}

uint32_t CR95HF_GetUID(uint8_t uid[8])
{
    uint8_t *resp;
    uint16_t len;
    uint32_t ret;

    ret = CR95HF_Inventory(&resp, &len);
    if(ret != NO_ERROR)
    {
        return ret;
    }

    memcpy(uid, &resp[4], 8);

    return NO_ERROR;
}