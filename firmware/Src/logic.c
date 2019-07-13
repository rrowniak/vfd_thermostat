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

#include "logic.h"
#include "vfd_driver.h"
#include "fan_driver.h"

#include "usart.h"
#include "flash.h"

// ----------------------------------------
// ADC light and temps
// ----------------------------------------
#define V_REF 3.3
#define ADC_RES 4095

static uint8_t ambient_t = 0;
static uint8_t chamber_t = 0;

static volatile uint8_t convCompleted = 0;

static ADC_HandleTypeDef* adc_temp = NULL;
static ADC_HandleTypeDef* adc_light = NULL;

// ----------------------------------------
// Timer
// ----------------------------------------
struct Timer
{
    uint32_t Period_ms;
    uint32_t Prev_ms;
};

static int __timer_update(struct Timer* t, uint32_t now_ms)
{
    if ((now_ms - t->Prev_ms >= t->Period_ms)
        || (t->Prev_ms == 0)) {

        t->Prev_ms = now_ms;
        return 1;
    }
    return 0;
}

static struct Timer tim5s = { .Period_ms = 5000, .Prev_ms = 0};
static struct Timer tim05s = { .Period_ms = 500, .Prev_ms = 0};
static struct Timer tim5ms = { .Period_ms = 5, .Prev_ms = 0};

// ----------------------------------------
// Configuration
// ----------------------------------------

#define MAGIC_CONF_NUM1 0xfa
#define MAGIC_CONF_NUM2 0xcd

#define CONF_FAN_SLOW 0
#define CONF_FAN_NORMAL 1
#define CONF_FAN_FAST 2

#define CONF_DEFAULT_TEMP_TH 70

struct Configuration
{
    uint8_t magicNum1;
    uint8_t fanSpeed;
    uint8_t tempThreshold;
    uint8_t magicNum2;
};

static struct Configuration currentConfig;
static uint8_t configChanged = 0;

static void __save_configuration(struct Configuration* cfg)
{
    HAL_StatusTypeDef s = flash_write((uint16_t *)cfg, 
                sizeof(struct Configuration) / sizeof(uint16_t));

    if (s != HAL_OK) {
        LOG2("Unable to save data to Flash: ", s);
    }
}

static void __load_configuration()
{
    struct Configuration fromFlash;
    
    LOG("__load_configuration");

    flash_read((uint16_t*)&fromFlash, 
                sizeof(struct Configuration) / sizeof(uint16_t));

    // check magics
    if ((fromFlash.magicNum1 == MAGIC_CONF_NUM1)
        && (fromFlash.magicNum2 == MAGIC_CONF_NUM2)) {
        // valid config
        currentConfig = fromFlash;
        LOG("Reading configuration from Flash");
    } else {
        // first time
        currentConfig.magicNum1 = MAGIC_CONF_NUM1;
        currentConfig.fanSpeed = CONF_FAN_FAST;
        currentConfig.tempThreshold = CONF_DEFAULT_TEMP_TH;
        currentConfig.magicNum2 = MAGIC_CONF_NUM2;
        __save_configuration(&currentConfig);
        LOG("First run - default configuration saved to Flash");
        LOG3("Magic nums ", fromFlash.magicNum1, fromFlash.magicNum2);        
    }
}

// ----------------------------------------
// User interface
// ----------------------------------------
enum ButtonStates
{
    BTN_RELEASED,
    BTN_PRESSED
};

enum ButtonEvents
{
    BTN_EV_NONE,
    BTN_EV_PRESSED,
    BTN_EV_RELEASED
};

struct Button
{
    GPIO_TypeDef* Gpio;
    uint16_t Gpio_pin;

    enum ButtonStates prevState;
};

static enum ButtonEvents __button_update(struct Button* btn)
{
    enum ButtonEvents ret = BTN_EV_NONE;

    uint8_t pressed = 
        (GPIO_PIN_RESET == HAL_GPIO_ReadPin(btn->Gpio, btn->Gpio_pin));

    if (pressed && (btn->prevState == BTN_RELEASED)) {
        ret = BTN_EV_PRESSED;
    } else if (!pressed && (btn->prevState == BTN_PRESSED)) {
        ret = BTN_EV_RELEASED;
    }

    if (ret != BTN_EV_NONE) {
        btn->prevState = pressed ? BTN_PRESSED : BTN_RELEASED;
    }

    return ret;
}

static struct Button Btn1 = { 
    .Gpio = MODE_GPIO_Port, 
    .Gpio_pin = MODE_Pin,
    .prevState = BTN_RELEASED
};

static struct Button Btn2 = { 
    .Gpio = SELECT_GPIO_Port, 
    .Gpio_pin = SELECT_Pin,
    .prevState = BTN_RELEASED
};

// ----------------------------------------
// Display logic
// ----------------------------------------

static uint8_t tempToogle = 0;

static void __display_temp(uint8_t t1, uint8_t t2)
{
    if ((t1 < 100) && (t2 < 100)) {
        vfd_driver_print_left(t1);
        vfd_driver_print_right(t2);
    } else {
        // toogle mode
        uint8_t t = (tempToogle) ? t1 : t2;
        tempToogle ^= 0x01;

        if (t >= 100) {
            // we assume that the temp is 1xy celcius deg
            // so print 1
            vfd_driver_light_cust(1, VFD_SEG_B | VFD_SEG_C);
            vfd_driver_print_right(t - 100);
        } else {
            vfd_driver_print_right(t);
        }
    }
}

static void __display(uint8_t t1, uint8_t t2)
{
    if (!configChanged) {
        __display_temp(t1, t2);
    } else {
        vfd_driver_print_left(currentConfig.fanSpeed);
        vfd_driver_print_right(currentConfig.tempThreshold);
    }
}

static void __adjust_brightness(uint8_t level) {
    if (level < BRIGHTNESS_LEV_0) {
        vfd_driver_set_brightness(VFD_BRID_MAX);
    } else if (level < BRIGHTNESS_LEV_1) {
        vfd_driver_set_brightness(VFD_BRID_50);
    } else if (level < BRIGHTNESS_LEV_2) {
        vfd_driver_set_brightness(VFD_BRID_25);
    }
}

// ----------------------------------------
// Logic implementation
// ----------------------------------------
void dma_conv_int()
{
    convCompleted = 1;
}

void logic_init(ADC_HandleTypeDef* adc_temp_,
                ADC_HandleTypeDef* adc_light_)
{
    adc_temp = adc_temp_;
    adc_light = adc_light_;

    HAL_ADC_Start(adc_light);

    __load_configuration();
}

static uint8_t __conv_temp(uint16_t adc)
{
    float tf = (((float)adc) / ADC_RES) * V_REF;
    uint16_t t = 100 * tf;

    return t;
}

static void __get_temp_lm35()
{
    uint16_t rawValues[2];
    rawValues[0] = 0;
    rawValues[1] = 0;
    convCompleted = 0;
    HAL_ADC_Start_DMA(adc_temp, (uint32_t*)rawValues, 2);
    while (!convCompleted);
    HAL_Delay(1);
    HAL_ADC_Stop_DMA(adc_temp);

    uint16_t adc_t1 = rawValues[0];
    ambient_t = __conv_temp(adc_t1);

    uint16_t adc_t2 = rawValues[1];
    chamber_t = __conv_temp(adc_t2);
}

static uint8_t __get_light()
{
    HAL_StatusTypeDef r = HAL_ADC_PollForConversion(adc_light, 200);

    if (r == HAL_ERROR) {
        return 0;
    } else if (r == HAL_TIMEOUT) {
        return 1;
    }

    uint16_t adc_l = HAL_ADC_GetValue(adc_light);
    return 100 * ((float)adc_l) / ADC_RES;
}

void logic_init_selfcheck()
{
    LOG("Selfcheck");
    fan_driver_set_power(0);
    // print all
    LOG("Print all");
    vfd_driver_light_cust(0, 0xFF);
    vfd_driver_light_cust(1, 0xFF);
    vfd_driver_light_cust(2, 0xFF);
    vfd_driver_light_cust(3, 0xFF);
    vfd_driver_light_dots(0xf);
    HAL_Delay(2000);

    vfd_driver_clear();
    HAL_Delay(200);

    // print version
    LOG("Version");
    vfd_driver_print_left(VER_MAJOR);
    vfd_driver_print_right(VER_MINOR);
    HAL_Delay(2000);

    vfd_driver_clear();
    HAL_Delay(200);
    LOG("Brightness");
    // test brightness
    for (uint8_t i = VFD_BRID_MIN; i <= VFD_BRID_MAX; ++i) {        
        vfd_driver_set_brightness(i);
        vfd_driver_light_cust(0, VFD_SEG_G);
        vfd_driver_light_cust(1, VFD_SEG_G);
        vfd_driver_print_right(__get_light());
        HAL_Delay(1000);
        vfd_driver_clear();
    }
    vfd_driver_set_brightness(VFD_BRID_MAX);
    // test motor driver
    LOG("Motor driver");
    fan_driver_set_power(100);
    HAL_Delay(1000);

    for (uint8_t i = 100;; i -= 5) {
        fan_driver_set_power(i);
        vfd_driver_print_left(i);
        HAL_Delay(50);
        if (i == 0) {
            break;
        }
    }
    // test temperature sensors
    LOG("Temp sensors");
    vfd_driver_clear();
    __get_temp_lm35();
    vfd_driver_print_left(ambient_t);
    vfd_driver_print_right(chamber_t);
    HAL_Delay(2000);

    LOG("Selftests finished");
}

void logic_update()
{
    uint32_t now_ms = HAL_GetTick();

    // every 5 seconds
    if (__timer_update(&tim5s, now_ms)) {
        __get_temp_lm35();

        __display(ambient_t, chamber_t);

        uint8_t l = __get_light();

        __adjust_brightness(l);

        LOG4("Readings [t1, t2, l]: ", ambient_t, chamber_t, l);

        // fan drive logic
        if (chamber_t > currentConfig.tempThreshold + TEMP_DELTA_MAX) {
            fan_driver_set_power(100);
        } else if (chamber_t > currentConfig.tempThreshold) {
            fan_driver_set_power(50);
        } else {
            // turn off the fan
            fan_driver_set_power(0);
        }
    }

    // every 0.5 second
    if (__timer_update(&tim05s, now_ms)) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }

    // every 5 ms
    if (__timer_update(&tim5ms, now_ms)) {

        enum ButtonEvents ev = __button_update(&Btn1);

        if (ev == BTN_EV_RELEASED) {
            configChanged = 1;
            ++currentConfig.fanSpeed;
            if (currentConfig.fanSpeed > CONF_FAN_FAST) {
                currentConfig.fanSpeed = CONF_FAN_SLOW;
            }
        }

        ev = __button_update(&Btn2);

        if (ev == BTN_EV_RELEASED) {
            configChanged = 1;
            currentConfig.tempThreshold += 5;
            if (currentConfig.tempThreshold > 100) {
                currentConfig.tempThreshold = 0;
            }
        }

        if (configChanged) {
            __display(0, 0);
            __save_configuration(&currentConfig);
            configChanged = 0;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_5) {
        fan_driver_zero_cross_int();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3) {
        fan_driver_launch_triac_int();
    } else if (htim->Instance == TIM4) {
        vfd_driver_int();
    }
}