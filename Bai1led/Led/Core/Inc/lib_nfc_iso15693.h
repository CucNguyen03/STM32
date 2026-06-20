#ifndef LIB_NFC_ISO15693_H
#define LIB_NFC_ISO15693_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "lib_iso15693pcd.h"

uint32_t Nfc_Tag_Hunting(uint8_t ** tagUID);
uint32_t NFC_ReadTagMemory( uint8_t * tagMemoryBuffer );
uint32_t PCDNFCT5_WriteNDEF( uint8_t * tagMemBufferToWrite, uint16_t writeLen );











#ifdef __cplusplus
}
#endif

#endif /* LIB_NFC_ISO15693_H */

