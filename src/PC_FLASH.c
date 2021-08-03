/*
 * PC_FLASH.c
 *
 *  Created on: Mar 23, 2017
 *      Author: Thomas
 */



#define START_PROG_FLASH 	0x08007800 								// Start address for variables 32 pages left = 64KB for variables

#include "main.h"



/*
 * ====================================
 * Update Flash 16-bit data (1 data per page)
 * ====================================
 */
uint16_t FLASH_FastUpdate16Bits(uint16_t flashDat, uint32_t flashAdd)
{
	FLASH_Unlock();
	FLASH_ErasePage(((flashAdd + START_PROG_FLASH) >> 10) << 10);
	FLASH_ProgramHalfWord((flashAdd + START_PROG_FLASH), flashDat);
	FLASH_Lock();

	return *((uint16_t *) (flashAdd + START_PROG_FLASH));
}

/*
 * ========================================
 * Read 16 bits in Main Flash Memory
 * ========================================
 */
uint16_t FLASH_Read(uint32_t flashAdd)
{
	return *((uint16_t *) (flashAdd + START_PROG_FLASH));
}
