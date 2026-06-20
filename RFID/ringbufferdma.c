#include "ringbufferdma.h"


void RingBufferDmaU8_initUSARTRx(RingBufferDmaU8_TypeDef* ring, UART_HandleTypeDef* husart, uint8_t* buffer, uint16_t size) {
  ring->buffer = buffer;
  ring->size = size;
  ring->tailPtr = buffer;
  ring->dmaHandle = husart->hdmarx;
  HAL_UART_Receive_DMA(husart, buffer, size);
}
uint16_t RingBufferDmaU8_available(RingBufferDmaU8_TypeDef* ring) {
#ifdef __HAL_DMA_GET_COUNTER
  uint32_t leftToTransfer = __HAL_DMA_GET_COUNTER(ring->dmaHandle);
#else
  uint32_t leftToTransfer = ring->dmaHandle->Instance->CNDTR;
#endif
  volatile uint8_t const* head = ring->buffer + ring->size - leftToTransfer;
  volatile uint8_t const* tail = ring->tailPtr;
  if (head >= tail) {
    return head - tail;
  } else {
    return head - tail + ring->size;
  }
}


uint32_t RingBufferDmaU8_nReadBytes(RingBufferDmaU8_TypeDef* ring, uint8_t * readBuffer, uint32_t maxLenght)
{
  uint8_t * out = readBuffer;
	uint16_t numberAvailbleBytes = RingBufferDmaU8_available(ring);
	if(numberAvailbleBytes == 0) 
	{
    return 0;
  }
	else
	{
		uint32_t size = numberAvailbleBytes;
		if(size > maxLenght)
		{
			size = maxLenght;
		}
		for (uint32_t i = 0; i < size; i++) 
		{
			*(out++) = *ring->tailPtr++;    
			if (ring->tailPtr >= ring->buffer + ring->size)
			{
				ring->tailPtr -= ring->size;
			}
		}
		return size;
	}
}




