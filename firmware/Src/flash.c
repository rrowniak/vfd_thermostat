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

#include "flash.h"

#define WAIT_TIMEOUT 5

#define GO_IF_SUCCESS(stat) \
    if (stat != HAL_OK) \
        return stat;

#define CLEAN_IF_FAILURE(stat) \
	if (stat != HAL_OK) \
        goto clean;

static inline void __flash_page_erase(uint32_t pageAddress)
{
	SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, pageAddress);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
}

static inline HAL_StatusTypeDef __flash_program_halfword(uint32_t address,
														 uint16_t* data,
														 uint32_t size)
{
	HAL_StatusTypeDef ret = HAL_OK;
	SET_BIT(FLASH->CR, FLASH_CR_PG);

	for (uint32_t i = 0; i < size; ++i) {
		// while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY));
		ret = FLASH_WaitForLastOperation(WAIT_TIMEOUT);
		GO_IF_SUCCESS(ret);

		*(__IO uint16_t*)(address + i * 2) = data[i];
	}
	
	CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
	return ret;
}
 
HAL_StatusTypeDef flash_write(uint16_t* data, uint32_t size)
{
    HAL_StatusTypeDef ret;
	
    ret = HAL_FLASH_Unlock();
    GO_IF_SUCCESS(ret);

    ret = FLASH_WaitForLastOperation(WAIT_TIMEOUT);
    CLEAN_IF_FAILURE(ret);

	__flash_page_erase(FLASH_PAGE_ADDR);

	ret = FLASH_WaitForLastOperation(WAIT_TIMEOUT);
    CLEAN_IF_FAILURE(ret);

	FLASH->CR &= ~FLASH_CR_PER; // Page Erase Clear
	
    ret = __flash_program_halfword(FLASH_PAGE_ADDR, data, size);
	CLEAN_IF_FAILURE(ret);
	
	return HAL_FLASH_Lock();

clean:
	HAL_FLASH_Lock();
	return ret;
}
 
void flash_read(uint16_t* data, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i) {
		data[i] = *(__IO uint16_t*)(FLASH_PAGE_ADDR + i * 2);
	}
 
}