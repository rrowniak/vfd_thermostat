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

#include "usart.h"
#include "main.h"

static UART_HandleTypeDef* _huart;

void usart_config(UART_HandleTypeDef* huart)
{
	_huart = huart;
}

void send_char(char c)
{
	HAL_UART_Transmit(_huart, (uint8_t*)&c, sizeof(char), HAL_MAX_DELAY);
}

void send_string(const char* s)
{
	while (*s) {
		send_char(*s++);
	}
}

void send_int(uint32_t val)
{
	char buff[16];
	my_itoa(val, buff);
	send_string(buff);
}

void send_ln()
{
	send_string("\r\n");
}

void send_string_int(const char* s, uint32_t val)
{
	char buff[16];
	my_itoa(val, buff);

	send_string(s);
	send_string(buff);
}

void send_string_int_ln(const char* s, uint32_t val)
{
	send_string_int(s, val);
	send_string("\r\n");
}

char* my_itoa(uint32_t i, char b[])
{
	char const digit[] = "0123456789";
	char* p = b;

	uint32_t shifter = i;
	do {
		++p;
		shifter = shifter/10;
	} while(shifter);

	*p = '\0';

	do {
		*--p = digit[i % 10];
		i = i / 10;
	} while (i);

	return b;
}
