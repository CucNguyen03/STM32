#ifndef __ST25R95_H
#define __ST25R95_H

#include "main.h"
#include <stdint.h>

/* ST25R95 Commands */
#define ST25R95_CMD_IDN              0x01
#define ST25R95_CMD_PROTOCOL_SELECT  0x02
#define ST25R95_CMD_SEND_RECEIVE     0x04
#define ST25R95_CMD_ECHO             0x55

/* SPI Control Bytes */
#define ST25R95_SPI_SEND             0x00
#define ST25R95_SPI_READ             0x02
#define ST25R95_SPI_POLL             0x03


#define ST25R95_CS_PORT SPI1_NSS_GPIO_Port
#define ST25R95_CS_PIN  SPI1_NSS_Pin

#define ST25R95_IRQ_OUT_PORT RFID_IRQ_GPIO_Port
#define ST25R95_IRQ_OUT_PIN  RFID_IRQ_Pin

#define ST25R95_PWR_PORT      RFID_PWR_EN_GPIO_Port
#define ST25R95_PWR_PIN       RFID_PWR_EN_Pin



/* CS Control */
#define ST25R95_CS_LOW()  HAL_GPIO_WritePin(ST25R95_CS_PORT, ST25R95_CS_PIN, GPIO_PIN_RESET)
#define ST25R95_CS_HIGH() HAL_GPIO_WritePin(ST25R95_CS_PORT, ST25R95_CS_PIN, GPIO_PIN_SET)

/* SPI Handle */
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

/* Basic Functions */
void ST25R95_Init(void);
uint8_t ST25R95_Echo(void);
uint8_t ST25R95_IDN(uint8_t *buffer);
uint8_t ST25R95_ProtocolSelect(void);

/* RFID Communication */
uint8_t ST25R95_SendRecv(uint8_t *txData, uint8_t txLen,
                        uint8_t *rxData, uint8_t *rxLen);

uint8_t ST25R95_ReadUID(uint8_t *uid, uint8_t *uidLen);

#endif /* __ST25R95_H */