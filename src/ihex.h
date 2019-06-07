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