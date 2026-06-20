#include "st25r95.h"
#include <string.h>
#include <stdio.h>


#define ST25R95_MAX_BUFFER 64

static void ST25R95_SPI_Write(uint8_t *data, uint8_t length)
{
    ST25R95_CS_LOW();
    HAL_SPI_Transmit(&hspi1, data, length, 100);
    ST25R95_CS_HIGH();
}

static void ST25R95_SPI_Read(uint8_t *data, uint8_t length)
{
    ST25R95_CS_LOW();
    HAL_SPI_Receive(&hspi1, data, length, 100);
    ST25R95_CS_HIGH();
}


static void ST25R95_SendCommand(uint8_t *cmd, uint8_t len)
{
    uint8_t buffer[ST25R95_MAX_BUFFER];

    buffer[0] = ST25R95_SPI_SEND;
    memcpy(&buffer[1], cmd, len);

    ST25R95_SPI_Write(buffer, len + 1);
}

static uint8_t ST25R95_Poll(void)
{
    uint8_t tx[2] = {0x03,0x00};
    uint8_t rx[2] = {0};

    ST25R95_CS_LOW();

    HAL_SPI_TransmitReceive( &hspi1,tx, rx, 2,100);
    ST25R95_CS_HIGH();

    return rx[1];
}

static void ST25R95_ReadResponse(uint8_t *data, uint8_t length)
{
    uint8_t cmd = ST25R95_SPI_READ;

    ST25R95_CS_LOW();

    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi1, data, length, 100);

    ST25R95_CS_HIGH();
}


static uint8_t ST25R95_WaitIRQ(uint32_t timeout)
{
    uint32_t tick = HAL_GetTick();

    while(HAL_GPIO_ReadPin(RFID_IRQ_GPIO_Port, RFID_IRQ_Pin) == GPIO_PIN_RESET)
    {
        if((HAL_GetTick() - tick) > timeout)
        {
            return 0;
        }
    }

    return 1;
}


void ST25R95_Init(void)
{
    ST25R95_CS_HIGH();
    /* Reset / Wakeup ST25R95 */

    HAL_GPIO_WritePin(ST25R95_PWR_PORT, ST25R95_PWR_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);

    HAL_GPIO_WritePin(ST25R95_PWR_PORT, ST25R95_PWR_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
}


uint8_t ST25R95_Echo(void)
{
    uint8_t tx[2] = {0x00,0x55};

    ST25R95_CS_LOW();

    HAL_StatusTypeDef ret =
        HAL_SPI_Transmit(&hspi1, tx, 2, 100);

    ST25R95_CS_HIGH();

    return (ret == HAL_OK);
}

uint8_t ST25R95_IDN(uint8_t *buffer)
{
    uint8_t cmd[2];

    cmd[0] = ST25R95_CMD_IDN;
    cmd[1] = 0x00;

    ST25R95_SendCommand(cmd, 2);

    if(!ST25R95_WaitIRQ(100))
        return 0;

    if(ST25R95_Poll() != 0x00)
        return 0;

    ST25R95_ReadResponse(buffer, 16);

    return 1;
}

uint8_t ST25R95_ProtocolSelect(void)
{
    uint8_t cmd[4];
    uint8_t rx[8];

    cmd[0] = ST25R95_CMD_PROTOCOL_SELECT;
    cmd[1] = 0x02;
    cmd[2] = 0x00;     // ISO15693
    cmd[3] = 0x01;

    ST25R95_SendCommand(cmd, 4);

    if(!ST25R95_WaitIRQ(100))
        return 0;

    if(ST25R95_Poll() != 0x00)
        return 0;

    ST25R95_ReadResponse(rx, 8);

		char dbg[64];
		sprintf(dbg,
						"PROTO RESP: %02X %02X %02X %02X\r\n",
						rx[0], rx[1], rx[2], rx[3]);

		HAL_UART_Transmit(&huart1,
											(uint8_t*)dbg,
											strlen(dbg),
											100);

    if(rx[0] == 0x00)
        return 1;

    return 0;
}

uint8_t ST25R95_SendRecv(uint8_t *txData,
                        uint8_t txLen,
                        uint8_t *rxData,
                        uint8_t *rxLen)
{
    uint8_t cmd[ST25R95_MAX_BUFFER];
    uint8_t i;

    cmd[0] = ST25R95_CMD_SEND_RECEIVE;
    cmd[1] = txLen;

    for(i = 0; i < txLen; i++)
    {
        cmd[2 + i] = txData[i];
    }

    ST25R95_SendCommand(cmd, txLen + 2);

    if(!ST25R95_WaitIRQ(500))
        return 0;

    if(ST25R95_Poll() != 0x00)
        return 0;

    ST25R95_ReadResponse(rxData, 32);
		char dbg[64];

		sprintf(dbg,
						"CTRL=%02X LEN=%02X\r\n",
						rxData[0],
						rxData[1]);

		HAL_UART_Transmit(&huart1,(uint8_t*)dbg,strlen(dbg),100);
    *rxLen = rxData[1];

    return 1;
}

uint8_t ST25R95_ReadUID(uint8_t *uid, uint8_t *uidLen)
{
    uint8_t tx[3];
    uint8_t rx[32];
    uint8_t len;
    uint8_t i;

    tx[0] = 0x26;
    tx[1] = 0x01;
    tx[2] = 0x00;

    if(!ST25R95_SendRecv(tx, 3, rx, &len))
        return 0;

    if(len < 12)
        return 0;

    for(i = 0; i < 8; i++)
    {
        uid[i] = rx[4 + i];
    }

    *uidLen = 8;

    return 1;

}