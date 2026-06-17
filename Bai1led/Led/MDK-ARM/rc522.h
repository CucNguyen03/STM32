#ifndef RC522_H
#define RC522_H

#include "main.h"

void RC522_Init(void);
uint8_t RC522_ReadReg(uint8_t addr);
void RC522_WriteReg(uint8_t addr, uint8_t value)
	
#endif
