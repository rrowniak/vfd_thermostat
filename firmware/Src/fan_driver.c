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

#include "fan_driver.h"

#include "usart.h"

static TIM_HandleTypeDef* triac_timer = NULL;
static volatile uint8_t manual_drive = 0;
static uint8_t prevPowerPerc = 0;

#define TIMER_DELAY_MIN 0
#define TIMER_DELAY_MAX 80

static inline void __fan_on()
{
    HAL_GPIO_WritePin(DRIVE_GPIO_Port, DRIVE_Pin, GPIO_PIN_SET);
}

static inline void __fan_off()
{
    HAL_GPIO_WritePin(DRIVE_GPIO_Port, DRIVE_Pin, GPIO_PIN_RESET);
}

void fan_driver_init(TIM_HandleTypeDef* triac_timer_)
{
    triac_timer = triac_timer_;

    __fan_off();
}

void fan_driver_set_power(uint8_t powerPercentage)
{
    if (powerPercentage == prevPowerPerc) {
        return;
    } else {
        prevPowerPerc = powerPercentage;
    }
    
    HAL_TIM_Base_Stop_IT(triac_timer);
    HAL_TIM_Base_DeInit(triac_timer);

    if (powerPercentage == 0) {
        manual_drive = 1;
        __fan_off();
    } else if (powerPercentage >= 100) {
        manual_drive = 1;
        __fan_on();
    } else {
        manual_drive = 0;
        // calculate duty cycle
        uint8_t inv = 100 - powerPercentage;
        uint32_t period = TIMER_DELAY_MIN + (TIMER_DELAY_MAX - TIMER_DELAY_MIN) 
            * inv / 100;
        LOG2("FAN DRIVER period= ", period);
        triac_timer->Init.Period = period;
        HAL_TIM_Base_Init(triac_timer);
    }
}

void fan_driver_zero_cross_int()
{
    if (manual_drive) {
        return;
    }
    
    __fan_off();
    
    HAL_TIM_Base_Start_IT(triac_timer);
}

void fan_driver_launch_triac_int()
{
    HAL_TIM_Base_Stop_IT(triac_timer);
    
    __fan_on();
}