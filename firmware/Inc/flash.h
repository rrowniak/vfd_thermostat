/*
 * Copyright (c) 2019 Rafal Rowniak rrowniak.com
 * 
 * The author hereby grant you a non-exclusive, non-transferable,
 * free of charge right to copy, modify, merge, publish and distribute,
 * the Software for the sole purpose of performing non-commercial
 * scientific research, non-commercial education, or non-commercial 
 * artistic projects.
 * 
 * Any other use, in particular any use for commercial purposes,
 * is prohibited. This includes, without limitation, incorporation
 * in a commercial product, use in a commercial service, or production
 * of other artefacts for commercial purposes.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash_ex.h"
#include <stdint.h>

#define FLASH_PAGE_START    0x8000000UL
// F103 has 64k flash
#define FLASH_NUM_PAGES     64
// use the last flash page, only 1k EEPROM
#define FLASH_EEPROM_PAGE   0x3F
// the address
#define FLASH_PAGE_ADDR (FLASH_PAGE_START + (FLASH_PAGE_SIZE * FLASH_EEPROM_PAGE))

// max data size is 1k
// data: half-word table
// size: size of the data array
HAL_StatusTypeDef flash_write(uint16_t* data, uint32_t size);

// max data size is 1k
// data: half-word table
// size: size of the data array
void flash_read(uint16_t* data, uint32_t size);

#endif // __FLASH_H__