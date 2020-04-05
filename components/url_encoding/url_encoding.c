/*
 * url_encoding.c
 *
 *  Created on: 21 mar 2020
 *      Author: szymo
 */

#include <stdlib.h>

static inline unsigned char to_hex(unsigned char x)
{
	return x + (x > 9 ? ('A' - 10) : '0');
}

static inline unsigned char from_hex(unsigned char ch)
{
	if (ch <= '9' && ch >= '0')
		ch -= '0';
	else if (ch <= 'f' && ch >= 'a')
		ch -= 'a' - 10;
	else if (ch <= 'F' && ch >= 'A')
		ch -= 'A' - 10;
	else
		ch = 0;
	return ch;
}

void urlEncode(const char* input, char* output, size_t outputSize)
{
	while (*input && outputSize > 1)
	{
		unsigned char c = (unsigned char)*input;

		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			c == '-' || c == '_' || c == '.')
		{ // allowed
			*output = c;
		}
		else if (c == ' ')
		{
			*output = '+';
		}
		else
		{
			*output = '%';
			output++;
			outputSize--;
			if (outputSize <= 1) break;

			*output = to_hex(c >> 4);
			output++;
			outputSize--;
			if (outputSize <= 1) break;

			*output = to_hex(c % 16);
		}

		output++;
		outputSize--;
		input++;
	}
	*output = 0;
}

void urlDecode(const char* input, char* output, size_t outputSize)
{
	while (*input && outputSize > 1)
	{
		unsigned char c = *input;
		unsigned char c1 = *(input + 1);
		unsigned char c2 = c1 != 0 ? *(input + 2) : 0;

		if (c == '+')
		{
			*output = ' ';
		}
		else if (c == '%' && c2 != 0)
		{
			unsigned char ch1 = from_hex(c1);
			unsigned char ch2 = from_hex(c2);
			*output = (ch1 << 4) | ch2;
			input += 2;
		}
		else
		{
			*output = c;
		}

		output++;
		outputSize--;
		input++;
	}
	*output = 0;
}
