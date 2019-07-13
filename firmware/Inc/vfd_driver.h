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

#ifndef _VFD_DRIVER_H_
#define _VFD_DRIVER_H_

#include <stdint.h>

void vfd_driver_init();

enum VfdSegment
{
    VFD_SEG_A = 0x01,
    VFD_SEG_B = 0x02,
    VFD_SEG_C = 0x04,
    VFD_SEG_D = 0x08,
    VFD_SEG_E = 0x10,
    VFD_SEG_F = 0x20,
    VFD_SEG_G = 0x40
};

enum VfdDot
{
    VFD_DOT_H = 0x1,
    VFD_DOT_L = 0x2
};

void vfd_driver_light_cust(uint8_t dig_num, uint8_t segs);
void vfd_driver_light_dots(uint8_t dots);

void vfd_driver_print_left(uint8_t num);
void vfd_driver_print_right(uint8_t num);

void vfd_driver_clear();

enum VfdBrightness
{
    VFD_BRID_MIN,
    VFD_BRID_25,
    VFD_BRID_50,
    VFD_BRID_MAX
};

void vfd_driver_set_brightness(enum VfdBrightness b);

void vfd_driver_int();

#endif // _VFD_DRIVER_H