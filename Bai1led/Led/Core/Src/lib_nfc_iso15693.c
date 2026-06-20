#include "lib_nfc_iso15693.h"
#include "drv_cr95hf.h"
#include "debug.h"
#include "miscellaneous.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
//static uint8_t cr95hfBuffer [CR95HF_RX_BUFFER_SIZE+3];
static uint32_t PCD_IsReaderResultCodeOk(uint8_t CmdCode, const uint8_t *ReaderReply)
{

	if (ReaderReply[READERREPLY_STATUSOFFSET] == PCD_ERRORCODE_DEFAULT)
		return PCD_NOREPLY_CODE;

  switch (CmdCode)
	{
		case ECHO: 
			if (ReaderReply[PSEUDOREPLY_OFFSET] == ECHO)
				return PCD_SUCCESSCODE;
			else 
				return PCD_ERRORCODE_DEFAULT;
			
		case IDN: 
			if (ReaderReply[READERREPLY_STATUSOFFSET] == IDN_RESULTSCODE_OK)
				return PCD_SUCCESSCODE;
			else 
				return PCD_ERRORCODE_DEFAULT;
			
		case PROTOCOL_SELECT: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDN_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
					
				case PROTOCOLSELECT_ERRORCODE_CMDLENGTH :
					return PCD_ERRORCODE_DEFAULT;
					
				case PROTOCOLSELECT_ERRORCODE_INVALID :
					return PCD_ERRORCODE_DEFAULT;
					
				default : return PCD_ERRORCODE_DEFAULT;
					
			}

		case SEND_RECEIVE: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case SENDRECV_RESULTSCODE_OK :
					if (ReaderReply[READERREPLY_STATUSOFFSET+1] != 0)
						return PCD_SUCCESSCODE;
					else
						return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_RESULTSRESIDUAL :
					return PCD_SUCCESSCODE;
					
				case SENDRECV_ERRORCODE_COMERROR :
					return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_ERRORCODE_FRAMEWAIT :
					return ERROR_NFC_FRAME_WAIT_TIMEOUT_OR_NO_TAG;
					
				case SENDRECV_ERRORCODE_SOF :
					//return PCD_ERRORCODE_DEFAULT;
					return ERROR_NFC_FRAME_WAIT_TIMEOUT_OR_NO_TAG;
				case SENDRECV_ERRORCODE_OVERFLOW :
					return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_ERRORCODE_FRAMING :
					return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_ERRORCODE_EGT :
					return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
					
				case SENDRECV_ERRORCODE_CRC :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_RECEPTIONLOST :
					return PCD_ERRORCODE_DEFAULT;
					
				default :
					return PCD_ERRORCODE_DEFAULT;
					
			}
			
		case IDLE: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDLE_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
					
				case IDLE_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
					
				default : return PCD_ERRORCODE_DEFAULT;
				
			}
			
		case READ_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case READREG_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
					
				case READREG_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
					
				default : return PCD_ERRORCODE_DEFAULT;
				
			}
			
		case WRITE_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case WRITEREG_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
					
				default : return PCD_ERRORCODE_DEFAULT;
				
			}
			
		case BAUD_RATE: 
			return PCD_ERRORCODE_DEFAULT;
			
		default: 
			return ERRORCODE_GENERIC;
			
	}
}

uint32_t NFC_ISO15693_Init(void)
{
	uint32_t ret;
	uint8_t *	pTResponse;
	
	uint8_t 	parametersByte = 0;
	parametersByte =  	((ISO15693_APPENDCRC << ISO15693_OFFSET_APPENDCRC ) 	&  ISO15693_MASK_APPENDCRC) |
											((ISO15693_SINGLE_SUBCARRIER << ISO15693_OFFSET_SUBCARRIER)	& ISO15693_MASK_SUBCARRIER)	|
											((ISO15693_MODULATION_100 << ISO15693_OFFSET_MODULATION) & ISO15693_MASK_MODULATION) |
											((ISO15693_WAIT_FOR_SOF <<  ISO15693_OFFSET_WAITORSOF ) & ISO15693_MASK_WAITORSOF) 	|
											((ISO15693_TRANSMISSION_26 <<   ISO15693_OFFSET_DATARATE  )	& ISO15693_MASK_DATARATE);
	
	ret = CR95HF_SelectProtocol_ISO15693(parametersByte,&pTResponse);
	if(ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_INIT_ISO15693, "CR95HF_SelectProtocol", ret);
		return ret;
	}
	ret = CR95HF_AjustAnalogRegister(&pTResponse);
	if(ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_INIT_ISO15693, "CR95HF_AjustAnalogRegister", ret);		
		return ret;
	}
	return NO_ERROR;
}


static uint8_t NFC_ISO15693_CreateRequestFlag(const uint8_t SubCarrierFlag, const uint8_t DataRateFlag,const uint8_t InventoryFlag,const uint8_t ProtExtFlag,
																					const uint8_t SelectOrAFIFlag,const uint8_t AddrOrNbSlotFlag,const uint8_t OptionFlag,const uint8_t RFUFlag)
{
	uint32_t FlagsByteBuf=0;

	FlagsByteBuf = 	(SubCarrierFlag 	& 0x01)					|
									((DataRateFlag  	& 0x01)	<< 1)		|
									((InventoryFlag 	& 0x01) << 2)		|
									((ProtExtFlag		& 0x01)	<< 3)			|
									((SelectOrAFIFlag   & 0x01)	<< 4)	|
									((AddrOrNbSlotFlag  & 0x01)	<< 5)	|
									((OptionFlag  		& 0x01) << 6)		|
									((RFUFlag  			& 0x01) << 7);

	return (uint8_t) FlagsByteBuf; 
}

static uint32_t NFC_ISO15693_Inventory(const uint8_t flagByte, uint8_t **pTResponse)
{
	uint32_t ret;			
	uint8_t inventoryCmd[5] = {CR95HF_COMMAND_SEND_RECEIVE,0x03,0x26,ISO15693_CMDCODE_INVENTORY,0x00};
					inventoryCmd[2] = flagByte;	// corrent flag byte
	
  /* Wait 310탎 before sending request, in the case a response was previously sent */
	HAL_Delay(1);
	// Send inventory command
  if(CR95HF_SendReceive(inventoryCmd, pTResponse) != NO_ERROR)
	{
		return ERROR_TIMEOUT;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pTResponse);
	return ret;
}


uint32_t ISO15693_GetUID (uint8_t **UIDout) 
{ 
	uint32_t 	ret;
	uint8_t 	FlagsByteData;
	uint8_t 	Tag_error_check;
	
	/* select 15693 protocol */
	ret= NFC_ISO15693_Init();
	if(ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_ISO15693_GetUID, "NFC_ISO15693_Init", ret);		
		return ret;
	}	
  /* Min time to respect before sending Inventory */
  HAL_Delay(5);
  
	FlagsByteData = NFC_ISO15693_CreateRequestFlag 	(	
													ISO15693_REQFLAG_SINGLESUBCARRIER,
													ISO15693_REQFLAG_HIGHDATARATE,
													ISO15693_REQFLAG_INVENTORYFLAGSET,
													ISO15693_REQFLAG_NOPROTOCOLEXTENSION,
													ISO15693_REQFLAG_NOTAFI,
													ISO15693_REQFLAG_1SLOT,
													ISO15693_REQFLAG_OPTIONFLAGNOTSET,
													ISO15693_REQFLAG_RFUNOTSET);
	
  ret = NFC_ISO15693_Inventory (FlagsByteData, UIDout );
	if(ret != NO_ERROR)
	{
		//Debug_Logout_Error(ERROR_NFC_ISO15693_GetUID, "NFC_ISO15693_Inventory", ret);								
		return ret;
	}	
	Tag_error_check = (*UIDout)[ISO15693_OFFSET_LENGTH]+1;
	if(((*UIDout)[Tag_error_check] & ISO15693_CRC_MASK) != 0x00 )
	{
		return ERROR_NFC_CRC_CHECK_FAILED;
	}
	return ret;
}

uint32_t Nfc_Tag_Hunting(uint8_t ** tagUID)
{
	uint32_t ret;
	ret = CR95HF_Set_FieldOff();
	if(ret != NO_ERROR)
	{
		return ret;
	}
	
	HAL_Delay(5);
	ret = ISO15693_GetUID (tagUID);
	if(ret == NO_ERROR)	
	{
		return NO_ERROR;
	}
  /* Turn off the field if no tag has been detected*/
	if(CR95HF_Set_FieldOff() != NO_ERROR)
	{
		return ERROR_NFC_ISO15693_DEFAULT;
	}	
	return ret;
}


/********************************************************************** ISO15693 read/write data library******************************************/

static uint32_t PCD_SendRecv(const uint8_t Length, const uint8_t *Parameters, uint8_t **pResponse)
{
	uint32_t ret;
	uint8_t DataToSend[CR95HF_TX_BUFFER_SIZE];	
//	/* check the function parameters	*/
//	if (CHECKVAL (Length,1,255)==false)
//		return PCD_ERRORCODE_PARAMETERLENGTH; 

	DataToSend[PCD_COMMAND_OFFSET ] = CR95HF_COMMAND_SEND_RECEIVE;
	DataToSend[PCD_LENGTH_OFFSET  ]	= Length;

	/* DataToSend CodeCmd Length Data*/
	/* Parameters[0] first byte to emmit	*/
	if(Length < CR95HF_TX_BUFFER_SIZE)
	{
		memcpy(&(DataToSend[PCD_DATA_OFFSET]),Parameters,Length);
	}
	else
	{
		ASSERT_FAIL();	
		return ERROR_NFC_INVALID_LENGTH;		
	}
		
	/* Send the command the Rf transceiver	*/
	ret = CR95HF_SendReceive(DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (CR95HF_COMMAND_SEND_RECEIVE, *pResponse);
	if (ret != PCD_SUCCESSCODE)
	{
		return ret;
//		if((*pResponse)[0] == PCD_ERRORCODE_NOTAGFOUND)
//			return ERROR_NFC_FRAME_WAIT_TIMEOUT_OR_NO_TAG;
//		else
//			return ERROR_NFC_ISO15693_DEFAULT;
	}
	return NO_ERROR;
}

static int8_t ISO15693_GetAddressOrNbSlotsFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_ADDRORNBSLOTSFLAG) != 0x00)
		return true ;
	else
		return false ;
}

static int8_t ISO15693_GetProtocolExtensionFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_PROTEXTFLAG) != 0x00)
		return true ;
	else
		return false ;
}

static int8_t ISO15693_ReadSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,uint8_t ** pResponse)
{
	uint32_t ret;
	uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK],
	NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READSINGLEBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{ /* M24LR16 and M24LR64 specific read single block command */
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;	
	}
  
  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
	
	ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return ISO15693_SUCCESSCODE;
}

/**  
* @brief  	this function send an ExtendedReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param		BlockNumber	:  	index of block to read
* @param		pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ExtendedReadSingleBlock (const uint8_t Flags, const uint8_t *UIDin, uc16 BlockNumber,uint8_t ** pResponse )
{
	uint32_t ret;
  uint8_t DataToSend[ISO15693_MAXLENGTH_EXTREADSINGLEBLOCK],
		NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_EXTREADSINGLEBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber & 0x00FF;
	DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;	

  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
  
	ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return ISO15693_SUCCESSCODE;
}


static uint8_t ISO15693_ReadSingleTagData(uint8_t Tag_Density, uint8_t * Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read)
{
	uint8_t /*ReadBuffer [4]={0x02, 0x20, 0x00, 0x00},*/
				//RepBuffer[16],
				Requestflags = 0x02;
	uint16_t NthDataToRead= 0x0000;
	uint16_t Num_DataToRead = FirstBlock_To_Read;
	uint8_t * pTResp;
		// update the RequestFlags
		if (Tag_Density == ISO15693_LOW_DENSITY)
			Num_DataToRead &=  0x00FF;
		else if (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY)
			Requestflags = 0x0A;	
		else if (Tag_Density == ISO15693_HIGH_DENSITY)
      Num_DataToRead &=  0xFFFF;
    else
			return ISO15693_ERRORCODE_DEFAULT;

	for ( NthDataToRead= 0; NthDataToRead < NbBlock_To_Read; NthDataToRead++)
	{			
 				Num_DataToRead = FirstBlock_To_Read + NthDataToRead;
        if(((Num_DataToRead <= 0xFF)&&(Tag_Density == ISO15693_LOW_DENSITY)) || (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY))
        {
          /* For lower blocks, or the high density tags relying on the ISO15693 ext. protocol bit */
         if ( ISO15693_ReadSingleBlock (Requestflags, 0x00,Num_DataToRead, &pTResp ) !=ISO15693_SUCCESSCODE)
						return ISO15693_ERRORCODE_DEFAULT;
        } else {
          /* Over the 256th block call the NFC-Forum Type5 Extended commands */
          if ( ISO15693_ExtendedReadSingleBlock (Requestflags, 0x00,Num_DataToRead, &pTResp) !=ISO15693_SUCCESSCODE)
						return ISO15693_ERRORCODE_DEFAULT;
        }					
 				 /*Data Temp*/
				 memcpy(&Data_To_Read[NthDataToRead*4],&pTResp[3],ISO15693_NBBYTE_BLOCKLENGTH);
	}
				
	return ISO15693_SUCCESSCODE;
}


/**  
* @brief  this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ReadMultipleBlock(const uint8_t Flags, const uint8_t *UIDin, uint16_t BlockNumber, const uint8_t NbBlock, uint8_t **pResponse)
{
	uint32_t ret;
uint8_t DataToSend[ISO15693_MAXLENGTH_READMULBLOCK],
		NthByte=0;


	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READMULBLOCKS;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	BlockNumber=BlockNumber<<5; // *32
	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{ /* M24LR16 and M24LR64 specific read multiple block command */
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
		
	}
	
	DataToSend[NthByte++] = NbBlock;

  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
  
	ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);
	if (ret != PCD_SUCCESSCODE)
		return ret;

  /* Check also Response flag here */
  if((*pResponse)[2] != 0)
    return ISO15693_ERRORCODE_DEFAULT;
  
	return ISO15693_SUCCESSCODE;

}


/**  
* @brief  this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ExtendedReadMultipleBlock (const uint8_t Flags, const uint8_t *UIDin, uint16_t BlockNumber, uint16_t NbBlock, uint8_t ** pResponse )
{
	uint32_t ret;
uint8_t DataToSend[ISO15693_MAXLENGTH_EXTREADMULBLOCK],
		NthByte=0;


	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_EXTREADMULBLOCKS;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	BlockNumber=BlockNumber<<5; // *32
  
  DataToSend[NthByte++] = BlockNumber & 0x00FF;
	DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
	
  DataToSend[NthByte++] = NbBlock & 0x00FF;
	DataToSend[NthByte++] = (NbBlock & 0xFF00 ) >> 8;

  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
  
	ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);
	if (ret != PCD_SUCCESSCODE)
		return ret;

  /* Check also Response flag here */
  if((*pResponse)[2] != 0)
    return ISO15693_ERRORCODE_DEFAULT;
  
	return ISO15693_SUCCESSCODE;

}

/**
* @brief  Read Multiple Block in the TAG (sector size : 0x20 = 32 Blocks)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBlock_To_Read : Number of block to read in the TAG
* @param  FirstBlock_To_Read : First block to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/
static uint8_t ISO15693_ReadMultipleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read)
{
	uint8_t /*ReadMultipleBuffer [5]={0x02, 0x23, 0x00, 0x00, 0x00},*/
				//RepBuffer[140],
				Requestflags = 0x02;

	uint16_t NbSectorToRead = 0,
				SectorStart = 0,
				NthDataToRead;
	uint8_t * pTResp;
	
	/*Convert the block number in sector number*/
  /* The ISO specify a maximum of 256 blocks for a read multiple blocks */
  /* But the tags doesn't necessarily support max number of blocks -> use 32 blocks for inter-op. */
  NbSectorToRead = NbBlock_To_Read/32+1;
	SectorStart = FirstBlock_To_Read/32;

	// update the RequestFlags
	/*if (Tag_Density == ISO15693_LOW_DENSITY)
		NumSectorToRead = (NumSectorToRead*0x20) & 0x00FF;
	else */if (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY)
		Requestflags = 0x0A;	
	/*else
		return ISO15693_ERRORCODE_DEFAULT;*/
	for ( NthDataToRead=0; NthDataToRead < NbSectorToRead; NthDataToRead++)
	{			
			//NumSectorToRead += NthDataToRead;
    /* A sector is 32 bytes */
    if((((NthDataToRead+SectorStart) <= 8)&&(Tag_Density == ISO15693_LOW_DENSITY)) || (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY))
    {
      /* For lower blocks, or the high density tags relying on the ISO15693 ext. protocol bit */
			if ( ISO15693_ReadMultipleBlock (Requestflags, 0x00,NthDataToRead+SectorStart,0x1F, &pTResp ) !=ISO15693_SUCCESSCODE)	
			{			
				//Debug_Show("\n\r-E-ISO15693_ReadMultipleBlock failed, ISO15693_ERRORCODE_DEFAULT");										
				return ISO15693_ERRORCODE_DEFAULT;	
			}
		} 
		else 
		{
      /* Over the 256th block call the Extended commands */
			if ( ISO15693_ExtendedReadMultipleBlock (Requestflags, 0x00,NthDataToRead+SectorStart,0x1F, &pTResp ) !=ISO15693_SUCCESSCODE)
			{	
				//Debug_Show("\n\r-E-ISO15693_ExtendedReadMultipleBlock failed, ISO15693_ERRORCODE_DEFAULT");														
				return ISO15693_ERRORCODE_DEFAULT;	
			}
    }      
		/*Data Temp*/
		memcpy(&Data_To_Read[NthDataToRead*128],&pTResp[3],128);
	}
	//Debug_Show("\n\r-I-ISO15693_ExtendedReadMultipleBlock successfully");																		
	return ISO15693_SUCCESSCODE;
}


/**
* @brief  Read data by Bytes in the TAG (with read multiple if it is possible)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  IC_Ref_Tag : The IC_Ref is use to use read multiple or read single
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBytes_To_Read : Number of Bytes to read in the TAG
* @param  FirstBytes_To_Read : First Bytes to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
uint8_t ISO15693_ReadBytesTagData(uint8_t Tag_Density, uint8_t IC_Ref_Tag, uint8_t *Data_To_Read, uint16_t NbBytes_To_Read, uint16_t FirstBytes_To_Read)
{
	uint8_t status = ISO15693_ERRORCODE_DEFAULT;
/*0x80 => Page size read multiple 0x20 & 1 block = 4 bytes*/
	uint16_t NbBlock_To_Read = NbBytes_To_Read/4;
	uint16_t FirstBlock_To_Read = FirstBytes_To_Read/4;
	
	/*LRiS2K don't support read multiple*/
	if(IC_Ref_Tag  == ISO15693_LRiS2K)
	{	
		NbBlock_To_Read = NbBytes_To_Read/4;		
		status = ISO15693_ReadSingleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);
		//Debug_Show("\n\r-I-ISO15693_ReadBytesTagData, ISO15693_LRiS2K");					
	}
	
	else
	{		
		status = ISO15693_ReadMultipleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);		
		//Debug_Show("\n\r-I-ISO15693_ReadBytesTagData");						
	}
	//memcpy(Data_To_Read,&Data_To_Read[FirstBytes_To_Read], NbBytes_To_Read);
	
	return status;
	
}


uint32_t NFC_ReadTagMemory( uint8_t * tagMemoryBuffer )
{
//	uint16_t size;
//	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
  /* Could be 2 or 4 bytes (when NDEF size > 0xff) */
  //uint8_t tlv_size = 2;
  /* Could be 4 or 8 (whem MLEN > 0xff) */
  //uint8_t  ccfile_size = 4;
  /* NDEF eof, always 1 byte: 0xFE */
  //const uint8_t  eof_size = 1;

	// Try to determine the density by reading the first sector (128 bytes)
  /* The density flag is used for the tags relying on ISO15693 extended protocol bit for higher addresses, such as M24LR64k (released before the NFC-forum Type Type5 specification) */
  /* While the tags like the ST25DV64k are not flagged HIGH_DENSITY as they rely on the extended commands from Type Tag standard */
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, tagMemoryBuffer, 127, 0) != ISO15693_SUCCESSCODE)
	{
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, tagMemoryBuffer, 127, 0) != ISO15693_SUCCESSCODE)
    {
      if(ISO15693_ReadBytesTagData(ISO15693_STLEGLR_HIGH_DENSITY, ISO15693_LRiS64K, tagMemoryBuffer, 127, 0) != ISO15693_SUCCESSCODE)
			{
				//Debug_Show("\n\r-E-ISO15693_ReadBytesTagData failed, ISO15693_STLEGLR_HIGH_DENSITY");							
				return PCDNFCT5_ERROR;
			}  
			//Debug_Show("\n\r-I-ISO15693_ReadBytesTagData successfully, tagDensity = ISO15693_STLEGLR_HIGH_DENSITY");						
      //tagDensity = ISO15693_STLEGLR_HIGH_DENSITY;
    }
    else
    {
      //tagDensity = ISO15693_LOW_DENSITY;
			//Debug_Show("\n\r-I-ISO15693_ReadBytesTagData successfully, tagDensity = ISO15693_LOW_DENSITY");			
    }
	}
	
	// NDEF capable ?
	//if ((tagMemoryBuffer[0] != 0xE1) && (tagMemoryBuffer[0] != 0xE2))
	//	return PCDNFCT5_ERROR_NOT_FORMATED;
	/*
	// Check read access
	if ((tagMemoryBuffer[1]&0x0C) != 0)
		return PCDNFCT5_ERROR_LOCKED;
	
	// Get the size of the message
	if (tagMemoryBuffer[2] == 0x00)
  {
    // 8 byte CCfile
    ccfile_size = 8;
    if (tagMemoryBuffer[9] == 0xFF)
    {
      tlv_size = 4;
      size = (tagMemoryBuffer[10]<<8)|tagMemoryBuffer[11];
    } 
		else 
		{
      size = 0x00FF&tagMemoryBuffer[9];    
    }
  } 
	else 
	{
    // 4 bytes CCfile
    if (tagMemoryBuffer[5] == 0xFF)
    {
      tlv_size = 4;
      size = (tagMemoryBuffer[6]<<8)|tagMemoryBuffer[7];
    }
		else
		{
      size = 0x00FF&tagMemoryBuffer[5];
    }
	}
  
	// Check if there is enough memory to read the tag
	// If CC3 bit3 = 1 the size is higher than 2KB but we don't know the size...
	if ((size+ccfile_size+eof_size+tlv_size) > NFC_TAG_MEMORY_MAX_SIZE)
		return PCDNFCT5_ERROR_MEMORY_INTERNAL;
	
	// Read the rest of the tag if needed
	if (size > (128 - (ccfile_size+eof_size+tlv_size)))
	{
    if( tagDensity == ISO15693_STLEGLR_HIGH_DENSITY )
    {
      if (ISO15693_ReadBytesTagData(tagDensity, ISO15693_LRiS64K, &tagMemoryBuffer[128], size - 128 + ccfile_size+eof_size+tlv_size, 128) != ISO15693_SUCCESSCODE)
      {
				//Debug_Show("\n\r-E-ISO15693_ReadBytesTagData failed (read the rest), tagDensity == ISO15693_STLEGLR_HIGH_DENSITY");															
        return PCDNFCT5_ERROR;
      }
    }
    else
    {
      if (ISO15693_ReadBytesTagData(tagDensity, ISO15693_LRiS64K, &tagMemoryBuffer[128], size - 128 + ccfile_size+eof_size+tlv_size, 128) != ISO15693_SUCCESSCODE)
      {
				//Debug_Show("\n\r-E-ISO15693_ReadBytesTagData failed (read the rest), tagDensity != ISO15693_STLEGLR_HIGH_DENSITY");											
        return PCDNFCT5_ERROR;
      }
    }
  }
	*/
	return PCDNFCT5_OK;	
}

/******************************************ISO15693 writting library*****************************************************/
/**  
* @brief  this function send an GetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
uint32_t ISO15693_GetSystemInfo(const uint8_t Flags, const uint8_t *UIDin, uint8_t ** pResponse)
{
	uint32_t ret;
	uint8_t DataToSend[ISO15693_MAXLENGTH_GETSYSTEMINFO],
	NthByte=0;
	//int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_GETSYSINFO;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
  
  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);

	ret = PCD_SendRecv(NthByte,DataToSend,pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);	
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return NO_ERROR;
}

/**  
* @brief  this function send an ExtendedGetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
uint32_t ISO15693_ExtendedGetSystemInfo ( const uint8_t Flags, const uint8_t Parameters, const uint8_t *UIDin, uint8_t **pResponse)
{
	uint32_t ret;
  uint8_t DataToSend[ISO15693_MAXLENGTH_EXTGETSYSTEMINFO],
		NthByte=0;
  //int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_EXTGETSYSINFO;
  DataToSend[NthByte++] = Parameters;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
  
  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);

	ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);	
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return NO_ERROR;
}

/**  
* @brief  this function send an Extended GetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
uint32_t PCDNFCT5_ExtendedGetSystemInfo ( const uint8_t Flags, const uint8_t ParamRequest, const uint8_t *UIDin, uint8_t **pResponse)
{
	uint32_t ret;
uint8_t DataToSend[ISO15693_MAXLENGTH_GETSYSTEMINFO],
		NthByte=0;
//int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTENDEDGETSYSINFO;
	DataToSend[NthByte++] = ParamRequest;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	ret = PCD_SendRecv(NthByte,DataToSend,pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);	
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return NO_ERROR;
}

/**
 *	@brief  this function send an EOF pulse to a contacless tag
 *  @param  pResponse : pointer on the PCD device reply
 *  @retval PCD_SUCCESSCODE : the function is succesful 
 */
static uint32_t PCD_SendEOF(uint8_t **pResponse)
{
	uint32_t ret;
	const uint8_t DataToSend[] = {SEND_RECEIVE	,0x00};

	ret = CR95HF_SendReceive(DataToSend, pResponse);
	if(ret != NO_ERROR)
	{
		return ret;
	}
	return NO_ERROR;
}

/**  
* @brief  	this function send an EOF pulse to contactless tag.
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static uint32_t ISO15693_SendEOF(uint8_t **pResponse  )
{
	
	PCD_SendEOF(pResponse);
	uint32_t ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);
	if (ret != NO_ERROR)
		return ret;
		
	return NO_ERROR;

}

/**  
* @brief  this function returns Option flag  (depending on inventory flag)
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Option flag
*/
static int8_t ISO15693_GetOptionFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_OPTIONFLAG) != 0x00)
		return true ;
	else
		return false ;
}


/**  
* @brief  	this function send an WriteSingleblock command and returns ISO15693_SUCCESSCODE if the command 
* @brief  	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to write
* @param	BlockLength :	Nb of byte of block length
* @param	DataToWrite :	Data to write into contacless tag memory
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_WriteSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,const uint8_t *DataToWrite,uint8_t ** pResponse )
{
	uint32_t ret;
uint8_t DataToSend[CR95HF_TX_BUFFER_SIZE],
		NthByte=0,
		BlockLength = ISO15693_NBBYTE_BLOCKLENGTH;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_WRITESINGLEBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{ /* M24LR16 and M24LR64 specific write single block command */
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
		
	}
	
	memcpy(&(DataToSend[NthByte]),DataToWrite,BlockLength);
	NthByte +=BlockLength;

  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
  
	if (ISO15693_GetOptionFlag (Flags) == false)
  {
		ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
  }
	else 
	{
		ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
	 	HAL_Delay(20);
		ret = ISO15693_SendEOF (pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
	}	

	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);	
	if (ret != PCD_SUCCESSCODE)
		return ret;

	return ISO15693_SUCCESSCODE;
	
}

/**  
* @brief  	this function send an ExtendedWriteSingleblock command and returns ISO15693_SUCCESSCODE if the command 
* @brief  	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to write
* @param	BlockLength :	Nb of byte of block length
* @param	DataToWrite :	Data to write into contacless tag memory
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ExtendedWriteSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,const uint8_t *DataToWrite,uint8_t ** pResponse )
{
	uint32_t ret;
  uint8_t DataToSend[CR95HF_TX_BUFFER_SIZE],
  NthByte=0,
  BlockLength = ISO15693_NBBYTE_BLOCKLENGTH;

	
  DataToSend[NthByte++] = Flags;
  DataToSend[NthByte++] = ISO15693_CMDCODE_EXTWRITESINGLEBLOCK;

  if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
  {	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
	NthByte +=ISO15693_NBBYTE_UID;	
  }

  DataToSend[NthByte++] = BlockNumber & 0x00FF;
  DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
	
  memcpy(&(DataToSend[NthByte]),DataToWrite,BlockLength);
  NthByte +=BlockLength;

  /* Wait 310탎 before sending request, in the case a response was previously sent */
  //HAL_Delay_Us(310);
	HAL_Delay(1);
 
  if (ISO15693_GetOptionFlag (Flags) == false)
	{
		ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
  }
	else 
  {	
		ret = PCD_SendRecv(NthByte,DataToSend, pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
		HAL_Delay(20);
		ret = ISO15693_SendEOF (pResponse);
		if(ret != NO_ERROR)
		{
			return ret;
		}		
  }	

	ret = PCD_IsReaderResultCodeOk (SEND_RECEIVE, *pResponse);  
	if (ret != PCD_SUCCESSCODE)
	return ret;

  return ISO15693_SUCCESSCODE;
}


/**
* @brief  Save the data from a TAG block for the uncomplete block to write
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  NbByte_To_Write : Numbre of Byte to write in the TAG
* @param  FirstByte_To_Write : First Byte to write in the TAG
* @param  *Data_To_Save : Data store before write the complete block of the TAG
* @param  *Length_Low_Limit : Number of Byte save for the first block to write (0 =< Length_Low_Limit =< 4)
* @param  *Length_High_Limit : Number of Byte save for the last block to write (0 =< Length_High_Limit =< 4)	
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_TagSave (uint8_t Tag_Density, uint16_t NbByte_To_Write, uint16_t FirstByte_To_Write, uint8_t *Data_To_Save, uint8_t *Length_Low_Limit, uint8_t *Length_High_Limit)
{				 
	uint32_t ret;
	const uint8_t NbBytePerBlock = 0x04; 		 
	uint8_t *pTResp;
	uint8_t //RepBuffer[32],
					ReadSingleBuffer [4]={0x02, 0x20, 0x00, 0x00};

	uint8_t Nb_Byte_To_Save_Low;
	uint8_t Nb_Byte_To_Save_High;						
	uint16_t Num_Block_Low;
	uint16_t Num_Block_High;
					
					
/**********Find Limit Block*****************/ 
			 Num_Block_Low = FirstByte_To_Write / NbBytePerBlock;
			 Num_Block_High = (FirstByte_To_Write + NbByte_To_Write) / NbBytePerBlock;
					
			 Nb_Byte_To_Save_Low	= (FirstByte_To_Write) % NbBytePerBlock;
			 Nb_Byte_To_Save_High	= 4-((FirstByte_To_Write + NbByte_To_Write/*+ Nb_Byte_To_Save_Low*/) % NbBytePerBlock); 

/**********Read Low Limit******************/					 
				if(Tag_Density == ISO15693_LOW_DENSITY)		
				{
          ReadSingleBuffer[2] = Num_Block_Low;
          ret = PCD_SendRecv (0x03,ReadSingleBuffer, &pTResp);
					if(ret != NO_ERROR)
					{
						return ret;
					}					
				}
				else if(Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY)
				{
           ReadSingleBuffer[0] = 0x0A;
           ReadSingleBuffer[2] =  Num_Block_Low & 0x00FF;
           ReadSingleBuffer[3] = (Num_Block_Low & 0xFF00) >> 8;
           ret = PCD_SendRecv (0x04,ReadSingleBuffer, &pTResp);
						if(ret != NO_ERROR)
						{
							return ret;
						}
				}
				else if(Tag_Density == ISO15693_HIGH_DENSITY)
				{
           ReadSingleBuffer[2] =  Num_Block_Low & 0x00FF;
           ReadSingleBuffer[3] = (Num_Block_Low & 0xFF00) >> 8;
           ret = PCD_SendRecv (0x04,ReadSingleBuffer, &pTResp);
						if(ret != NO_ERROR)
						{
							return ret;
						}					
				}		
				
				if(pTResp[0] != 0x80)
					return ISO15693_ERRORCODE_DEFAULT;
				
				memcpy(&Data_To_Save[0], &pTResp[3], Nb_Byte_To_Save_Low);
				*Length_Low_Limit = Nb_Byte_To_Save_Low;
				
/**********Read High Limit******************/				 		
				if(Tag_Density == ISO15693_LOW_DENSITY)	
				{	
					ReadSingleBuffer[2] = Num_Block_High;
					ret = PCD_SendRecv (0x03,ReadSingleBuffer, &pTResp);
					if(ret != NO_ERROR)
					{
						return ret;
					}					
				}
				
				else if(Tag_Density == ISO15693_HIGH_DENSITY)
				{
				 ReadSingleBuffer[0] = 0x0A;
				 ReadSingleBuffer[2] =  Num_Block_High & 0x00FF;
				 ReadSingleBuffer[3] = (Num_Block_High & 0xFF00) >> 8;
				 ret = PCD_SendRecv (0x04,ReadSingleBuffer, &pTResp);	
					if(ret != NO_ERROR)
					{
						return ret;
					}										
				}
				
				if(pTResp[0] != 0x80)
					return ISO15693_ERRORCODE_DEFAULT;
				
				/*Data temp*/
				memcpy(&Data_To_Save[4], &pTResp[3+(4-Nb_Byte_To_Save_High)], (Nb_Byte_To_Save_High));
				*Length_High_Limit = Nb_Byte_To_Save_High;
			return ISO15693_SUCCESSCODE;
}	

/**
* @brief  Write data by blocks in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBlock_To_Write : Number of block to write in the TAG
* @param  FirstBlock_To_Write : First block to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_WriteTagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBlock_To_Write, uint16_t FirstBlock_To_Write)
{
//	uint8_t WriteBuffer [8]={0x02, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	uint8_t * pTResp;
	uint8_t		//RepBuffer[32],
				Requestflags =0x02;
	uint16_t NthDataToWrite =0,
				 IncBlock =0;

		// update the RequestFlags
		if (Tag_Density == ISO15693_LOW_DENSITY)
			FirstBlock_To_Write &=  0x00FF;
		else if (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY)
			Requestflags = 0x0A;	
		else if (Tag_Density == ISO15693_HIGH_DENSITY)
      FirstBlock_To_Write &=  0xFFFF;
    else
			return ISO15693_ERRORCODE_DEFAULT;

		for ( NthDataToWrite=FirstBlock_To_Write; NthDataToWrite<(FirstBlock_To_Write+NbBlock_To_Write); NthDataToWrite++)
		{
      if(((NthDataToWrite <= 0xFF)&&(Tag_Density == ISO15693_LOW_DENSITY)) || (Tag_Density == ISO15693_STLEGLR_HIGH_DENSITY))
      {
        /* For lower blocks, or the high density tags relying on the ISO15693 ext. protocol bit */
        if ( ISO15693_WriteSingleBlock (Requestflags, 0x00,NthDataToWrite,&Data_To_Write[(NthDataToWrite - FirstBlock_To_Write)<<2], &pTResp ) !=ISO15693_SUCCESSCODE)
          return ISO15693_ERRORCODE_DEFAULT;
      } else {
        /* Over the 256th block call the NFC-Forum Type5 Extended commands */
        if ( ISO15693_ExtendedWriteSingleBlock (Requestflags, 0x00,NthDataToWrite,&Data_To_Write[(NthDataToWrite - FirstBlock_To_Write)<<2], &pTResp ) !=ISO15693_SUCCESSCODE)
          return ISO15693_ERRORCODE_DEFAULT;
      }
     IncBlock += 4;
		}
					
	return ISO15693_SUCCESSCODE;				
}	


/**
* @brief  Write data by bytes in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBytes_To_Write : Number of Bytes to write in the TAG
* @param  FirstBytes_To_Write : First Bytes to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	 
uint32_t ISO15693_WriteBytes_TagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBytes_To_Write, uint16_t FirstBytes_To_Write)
{
	/*1 block = 4 bytes*/
	uint16_t NbBlock_To_Write;

	/*Convert in Blocks the number of bytes to write*/
	uint16_t FirstBlock_To_Write = FirstBytes_To_Write/4;

	uint8_t Length_Low_Limit =0;
	uint8_t Length_High_Limit = 0;
	uint8_t Data_To_Save[8];
	
	if( (NbBytes_To_Write % 4) == 0 )
  {
    NbBlock_To_Write = (NbBytes_To_Write/4);
  }
  else
  {
    NbBlock_To_Write = (NbBytes_To_Write/4)+1;
  }
	
	/*Save the data of the uncomplete block to write*/
	ISO15693_TagSave(Tag_Density, NbBytes_To_Write, FirstBytes_To_Write, Data_To_Save, &Length_Low_Limit, &Length_High_Limit);	
	
	memcpy(&Data_To_Write[-Length_Low_Limit],&Data_To_Save[0], Length_Low_Limit);
	memcpy(&Data_To_Write[NbBytes_To_Write],&Data_To_Save[4], Length_High_Limit);
	
	if (ISO15693_WriteTagData(Tag_Density, &Data_To_Write[-Length_Low_Limit], NbBlock_To_Write, FirstBlock_To_Write) != ISO15693_SUCCESSCODE)
		return ERROR_NFC_ISO15693_DEFAULT;
	else Debug_Show("\n\r-I-ISO15693_WriteTagData OK");
  HAL_Delay(1000);
	return NO_ERROR;	
}

/**
 * @brief  This function writes the NDEF message to a tag type V from the TT5Tag buffer
 * @retval PCDNFCT5_OK : Command success
 * @retval PCDNFCT5_ERROR : Transmission error
 * @retval PCDNFCT5_ERROR_LOCKED : The tag cannot be write or read (CC lock)
 * @retval PCDNFCT5_ERROR_MEMORY : Not enough memory available on the tag
 */
uint32_t PCDNFCT5_WriteNDEF( uint8_t * tagMemBufferToWrite, uint16_t writeLen )
{
	uint32_t ret;
	uint8_t * pTResp;
	//uint8_t RepBuffer[30];
	uint8_t firstSector[140];
	uint16_t //size, 
	tagSize;
	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
  /* Could be 0xE1 or 0xE2 (for tags supporting the extended commands) */
  //uint8_t ndefCapable = 0xE1;
  /* Could be 2 or 4 bytes (when NDEF size > 0xff) */
  //uint8_t tlv_size = 2;
  /* Could be 4 or 8 (whem MLEN > 0xff) */
  //uint8_t  ccfile_size = 4;
  /* NDEF eof, always 1 byte: 0xFE */
  //const uint8_t  eof_size = 1;
  
	// Try to determine the density by ready the first sector (128 bytes)
  /* The density flag is used for the tags relying on ISO15693 extended protocol bit for higher addresses, such as M24LR64k (released before the NFC-forum Type Type5 specification) */
  /* While the tags like the ST25DV64k are not flagged HIGH_DENSITY as they rely on the extended commands from Type Tag standard */
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
	{
    if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
    {
      if(ISO15693_ReadBytesTagData(ISO15693_STLEGLR_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
        return PCDNFCT5_ERROR;
      
      tagDensity = ISO15693_STLEGLR_HIGH_DENSITY;
    }
    else
    {
      tagDensity = ISO15693_LOW_DENSITY;
    }
	}
	HAL_Delay(100);
  #ifdef USING_NDEF_FORMAT
	// Get the size of the message to write
  /* At this point, NDEF message is formatted for a 4 bytes CCfile */
  if (TT5Tag[5] == 0xFF)
  {
    size = (TT5Tag[6]<<8)|TT5Tag[7];
    tlv_size = 4;
  } else {
    size = 0x00FF&TT5Tag[5];
  }
  
	// NDEF CCfile present?
	if ((firstSector[0] != 0xE1) && (firstSector[0] != 0xE2))
	{
		/* Create the CC file */
		// We need the size
		if (tagDensity == ISO15693_STLEGLR_HIGH_DENSITY)
		{
      ISO15693_GetSystemInfo (0x0A , 0x00, &pTResp);
    }
		else
    {
			ISO15693_GetSystemInfo (0x02 , 0x00, &pTResp);
      if((pTResp[3] & 0x4) == 0)
      {
        /* Memory size is not present in GetSystemInfo response (try the extended command) */
        PCDNFCT5_ExtendedGetSystemInfo(0x2,0x3F,0x00, &pTResp);
        ndefCapable = 0xE2;
      }
		}
    
    /* Nblock & Block size are @byte 12/13/14 but first 2 bytes of the response for the reader status */
    if (RepBuffer[14] == 0xFF)
			tagSize = (((RepBuffer[15]<<8)|RepBuffer[14])+1)*(RepBuffer[16]+1);
		else
			tagSize = (RepBuffer[14]+1)*(RepBuffer[15]+1);
		// NDEF capable
		TT5Tag[0] = ndefCapable;
		// Version + Read/Write allowed
		TT5Tag[1] = 0x40;
		// Size
		if (tagSize > 2040) {
      /* This is the 8byte CCfile as defined by NFC-Forum Type 5 */
      memmove(&TT5Tag[8],&TT5Tag[4],size+tlv_size+eof_size);
      ccfile_size += 4;
			TT5Tag[2] = 0x0;
      if (tagDensity == ISO15693_STLEGLR_HIGH_DENSITY)
      {
        TT5Tag[3] = 0x00;
      }
      else
      {
        TT5Tag[3] = 0x01;
      }
			TT5Tag[4] = 0x0;
			TT5Tag[5] = 0x0;
			TT5Tag[6] = (tagSize/8) >> 8;
			TT5Tag[7] = (tagSize/8) & 0xFF;
    }
		else
		{
      /* Low density regular 4bytes CCfile */
			TT5Tag[2] = tagSize/8;
			TT5Tag[3] = 0x01;
		}
	}
	else
	{
    /* Read & copy the CCfile */
    if(firstSector[2] == 0)
    {
      /* 8 bytes CCfile - original NDEF message start at address 4, need to move it */
      memmove(&TT5Tag[8],&TT5Tag[4],size+tlv_size+eof_size);
      ccfile_size += 4;
      memcpy(TT5Tag,firstSector,8);
      tagSize = firstSector[6];
      tagSize = ((tagSize << 8) + firstSector[7])*8;
    } else {
      /* 4 bytes CCfile */
      memcpy(TT5Tag,firstSector,4);
      tagSize = firstSector[2] * 8;
    }
    
		// Check read and write access
		if ((TT5Tag[1]&0x0F) != 0)
			return PCDNFCT5_ERROR_LOCKED;
	}
	// Check if the memory available on the tag is enough
  if (tagSize < (size+ccfile_size+tlv_size+eof_size))
		return PCDNFCT5_ERROR_MEMORY_TAG;
	#endif
	// Check if the memory available on the tag is enough
	if (tagDensity == ISO15693_STLEGLR_HIGH_DENSITY)
	{
		ret = ISO15693_GetSystemInfo (0x0A , 0x00, &pTResp);
		if(ret != NO_ERROR)
		{
			Debug_Show("\n\r-E-ISO15693");
			HAL_Delay(1000);
			return ret;
		}
	}
	else
	{
		ret = ISO15693_GetSystemInfo (0x02 , 0x00, &pTResp);
		if(ret != NO_ERROR)
		{
			Debug_Show("\n\r-E-ISO15693");
			HAL_Delay(1000);
			return ret;
		}		
		if((pTResp[3] & 0x4) == 0)
		{
			/* Memory size is not present in GetSystemInfo response (try the extended command) */
			ret = PCDNFCT5_ExtendedGetSystemInfo(0x2,0x3F,0x00, &pTResp);
			if(ret != NO_ERROR)
			{
				Debug_Show("\n\r-E-PCDNFCT5");
				HAL_Delay(1000);
				return ret;
			}			
			//ndefCapable = 0xE2;
		}
	}
	
	/* Nblock & Block size are @byte 12/13/14 but first 2 bytes of the response for the reader status */
	if (pTResp[14] == 0xFF)
		tagSize = (((pTResp[15]<<8)|pTResp[14])+1)*(pTResp[16]+1);
	else
		tagSize = (pTResp[14]+1)*(pTResp[15]+1);
		
  if (tagSize < writeLen)
	{		
		Debug_Show("\n\r-E-PCDNFCT5_ERROR_MEMORY_TAG");
		HAL_Delay(1000);
		return PCDNFCT5_ERROR_MEMORY_TAG;	
	}
	// Write the tag
	Debug_Show("\r\n\r\n-I-Write the tag\r\n");
	HAL_Delay(1000);
	ret = ISO15693_WriteBytes_TagData(tagDensity, tagMemBufferToWrite, writeLen, 0);
	return ret;	
}


