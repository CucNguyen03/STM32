
#ifndef RINGBUFFER_DMA_H
#define RINGBUFFER_DMA_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32g0xx_hal.h"
#include "stdbool.h"

typedef struct {
  volatile uint8_t* buffer;
  uint16_t size;
  volatile uint8_t* tailPtr;
  DMA_HandleTypeDef* dmaHandle;
} RingBufferDmaU8_TypeDef;

void RingBufferDmaU8_initUSARTRx(RingBufferDmaU8_TypeDef* ring, UART_HandleTypeDef* husart, uint8_t* buffer, uint16_t size);
uint16_t RingBufferDmaU8_available(RingBufferDmaU8_TypeDef* ring);
uint32_t RingBufferDmaU8_nReadBytes(RingBufferDmaU8_TypeDef* ring, uint8_t * readBuffer, uint32_t maxLenght);
#define min(a,b) (((a)<(b))?(a):(b))

#ifdef __cplusplus
}
#endif

#endif /* RINGBUFFER_DMA_H */
