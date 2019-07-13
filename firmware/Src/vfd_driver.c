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

#include "vfd_driver.h"
#include "stm32f1xx_hal.h"

#define SECTIONS 5

static uint8_t vfd_sections[SECTIONS];

// static const uint8_t num_lookup_table[16] = {
//     0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 
//     0x7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47
// };

static const uint8_t num_lookup_table[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
    0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

static uint8_t current_section;

static volatile uint8_t skip_every_int;
static uint8_t skip_counter;

void vfd_driver_init()
{
    skip_every_int = VFD_BRID_MAX;
    skip_counter = 0;
}

void vfd_driver_light_cust(uint8_t dig_num, uint8_t segs)
{
    dig_num = (dig_num > 1) ? dig_num + 1 : dig_num;
    dig_num %= SECTIONS;

    vfd_sections[dig_num] = segs;
}

void vfd_driver_light_dots(uint8_t dots)
{
    vfd_sections[2] = dots;
}

static void __vfd_driver_print_digit(uint8_t section, uint8_t digit)
{
    vfd_sections[section] = num_lookup_table[digit];
}

static void __vfd_driver_print(uint8_t num, uint8_t secL, uint8_t secR)
{
    uint8_t d1 = num / 10;
    uint8_t d2 = num - d1 * 10;

    d1 = d1 % 10;
    d2 = d2 % 10;

    __vfd_driver_print_digit(secL, d1);
    __vfd_driver_print_digit(secR, d2);
}

void vfd_driver_print_left(uint8_t num)
{
    __vfd_driver_print(num, 4, 3);
}

void vfd_driver_print_right(uint8_t num)
{
    __vfd_driver_print(num, 1, 0);
}

void vfd_driver_clear()
{
    vfd_sections[0] = 0;
    vfd_sections[1] = 0;
    vfd_sections[2] = 0;
    vfd_sections[3] = 0;
    vfd_sections[4] = 0;
}

#define SET_VFD(pin) \
HAL_GPIO_WritePin(C_ANODES_##pin##_GPIO_Port, C_ANODES_##pin##_Pin, GPIO_PIN_SET);

#define RESET_VFD(pin) \
HAL_GPIO_WritePin(C_ANODES_##pin##_GPIO_Port, C_ANODES_##pin##_Pin, GPIO_PIN_RESET);

static void __clear_all_sections()
{
    HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_RESET);
}

static void __select_section(uint8_t s)
{
    switch (s) {
    case 0:        
        HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_SET);
        break;
    case 1:
        HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_SET);
        break;
    case 2:
        HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_SET);
        break;
    case 3:
        HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_SET);
        break;
    case 4:
        HAL_GPIO_WritePin(C_GRID_SEC_1_GPIO_Port, C_GRID_SEC_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_2_GPIO_Port, C_GRID_SEC_2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_3_GPIO_Port, C_GRID_SEC_3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(C_GRID_SEC_4_GPIO_Port, C_GRID_SEC_4_Pin, GPIO_PIN_RESET);
        
        HAL_GPIO_WritePin(C_GRID_SEC_5_GPIO_Port, C_GRID_SEC_5_Pin, GPIO_PIN_SET);
        break;
    }
}

#define ENCODE_VFD_BIT(value, bit, seg) \
    if (value & bit) { \
        SET_VFD(seg); \
    } else { \
        RESET_VFD(seg); \
    }


static void __encode_section(uint8_t value)
{
    ENCODE_VFD_BIT(value, 0x1, A);
    ENCODE_VFD_BIT(value, 0x2, B);
    ENCODE_VFD_BIT(value, 0x4, C);
    ENCODE_VFD_BIT(value, 0x8, D);
    ENCODE_VFD_BIT(value, 0x10, E);
    ENCODE_VFD_BIT(value, 0x20, F);
    ENCODE_VFD_BIT(value, 0x40, G);
}

void vfd_driver_set_brightness(enum VfdBrightness b)
{
    switch (b) {
    case VFD_BRID_MIN:
        skip_every_int = 4;
        break;
    case VFD_BRID_25:
        skip_every_int = 3;
        break;
    case VFD_BRID_50:
        skip_every_int = 2;
        break;
    case VFD_BRID_MAX:
        skip_every_int = 0;
        break;
    }
}

void vfd_driver_int()
{
    __clear_all_sections();
    
    if (skip_counter >= skip_every_int) {
        skip_counter = 0;
    } else {
        ++skip_counter;
        return;
    }

    ++current_section;
    current_section = current_section % SECTIONS;

    if (current_section == 2) {
        if (vfd_sections[2] & VFD_DOT_H) {
            HAL_GPIO_WritePin(C_ANODE_DOT_H_GPIO_Port, 
                              C_ANODE_DOT_H_Pin, 
                              GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(C_ANODE_DOT_H_GPIO_Port, 
                              C_ANODE_DOT_H_Pin, 
                              GPIO_PIN_RESET);
        }

        if (vfd_sections[2] & VFD_DOT_L) {
            HAL_GPIO_WritePin(C_ANODE_DOT_L_GPIO_Port, 
                              C_ANODE_DOT_L_Pin, 
                              GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(C_ANODE_DOT_L_GPIO_Port, 
                              C_ANODE_DOT_L_Pin, 
                              GPIO_PIN_RESET);
        }
    } else {
        __encode_section(vfd_sections[current_section]);
    }

    __select_section(current_section);
}