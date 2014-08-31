#ifndef MEM_H
#define MEM_H

#define nilai 0

#define NumbOfVar ((uint8_t)0x01)

void MEM_Setup(void);

uint16_t read_mem(uint16_t addr);
void save_mem(uint16_t addr, uint16_t data);

#endif
