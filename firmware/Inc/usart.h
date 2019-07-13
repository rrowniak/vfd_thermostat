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

#ifndef __USART_H__
#define __USART_H__

#include <stm32f1xx.h>
#include <stdint.h>

#define LOG(m) send_string(m"\n\r")
#define LOG1(m) LOG(m)
#define LOG2(m, i) send_string_int_ln(m, i)
#define LOG3(m, i, j) \
    { send_string(m); send_int(i); send_string(", "); \
    send_int(j); send_ln(); }
#define LOG4(m, i, j, k) \
    { send_string(m); send_int(i); send_string(", "); \
    send_int(j); send_string(", "); send_int(k); send_ln(); }

void usart_config(UART_HandleTypeDef* huart);
void send_char(char c);
void send_int(uint32_t val);
void send_ln();
void send_string(const char* s);
void send_string_int(const char* s, uint32_t val);
void send_string_int_ln(const char* s, uint32_t val);

char* my_itoa(uint32_t i, char b[]);

#endif // __USART_H__