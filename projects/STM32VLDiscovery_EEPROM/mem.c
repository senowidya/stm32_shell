#include "srcconf.h"

volatile uint16_t VirtAddVarTab[NumbOfVar]={0x3333};

void MEM_Setup(void){
//   SystemInit();
  FLASH_Unlock();
  EE_Init();
}

uint16_t read_mem(uint16_t addr){
	uint16_t data;
	EE_ReadVariable(addr, &data);
	return data;
}

void save_mem(uint16_t addr, uint16_t data){
	EE_WriteVariable(addr, data);
}
