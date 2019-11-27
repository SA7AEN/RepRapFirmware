//SD

#ifndef SOFTWARERESETDATA_H_
#define SOFTWARERESETDATA_H_

#include "Core.h"
#include "iap.h"


bool LPC_IsSoftwareResetDataSlotVacant(uint8_t slot);
uint32_t *LPC_GetSoftwareResetDataSlotPtr(uint8_t slot);
void LPC_ReadSoftwareResetDataSlot(uint8_t slot, void *data, uint32_t dataLength);
bool LPC_EraseSoftwareResetDataSlots();
bool LPC_WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength);

//Compatibility
inline void EraseAndReset(){};


#endif /* SOFTWARERESETDATA_H_ */
