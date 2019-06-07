/*
MIT License

Copyright (c) 2019 Marcin Borowicz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __IHEX_H
#define __IHEX_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
	IHEX_NO_ERROR = 0,
	IHEX_ERROR_DATA_OVERLAPPING,
	IHEX_ERROR_NO_EOF_LINE,
	IHEX_ERROR_PARSING_START_LINE,
	IHEX_ERROR_PARSING_HEX_ENCODE,
	IHEX_ERROR_PARSING_END_LINE,
	IHEX_ERROR_ADDRESS_FIELD,
	IHEX_ERROR_LINE_LENGTH,
	IHEX_ERROR_CHECKSUM,
	IHEX_ERROR_RECORD_TYPE,
	IHEX_ERROR_MALLOC,
	IHEX_ERROR_DUMP
} ihex_error;

struct ihex_data_segment {
	uint32_t adr_start;
	uint32_t data_size;
	uint8_t *data;
	struct ihex_data_segment *prev;
	struct ihex_data_segment *next;
} ihex_data_segment;

struct ihex_object {
	struct ihex_data_segment *segments;
	uint8_t pad_byte;
	uint8_t align_record;
	uint32_t extended_address;
	int finished_flag;
	ihex_error error;
};

typedef struct ihex_object* ihex_handler_t;

ihex_handler_t ihex_new(void);
void ihex_delete(ihex_handler_t self);

const char *ihex_get_error_string(ihex_handler_t self);

int ihex_parse_file(ihex_handler_t self, FILE *fp);
int ihex_dump_file(ihex_handler_t self, FILE *fp);

int ihex_set_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size);
int ihex_get_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size);

#endif /* __IHEX_H */