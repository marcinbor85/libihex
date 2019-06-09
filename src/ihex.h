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

/**
 * Enum type with defined errors.
 */
enum ihex_error {
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
};

typedef enum ihex_error ihex_error_e; /* typedef with error type */

/**
 * Structure with data segment fields.
 */
struct ihex_data_segment {
	uint32_t adr_start; /* starting address of data segment */
	uint32_t data_size; /* continous data segment size */
	uint8_t *data; /* pointer to data (dynamically created) */
	struct ihex_data_segment *prev; /* pointer to previous data segment (two-dir list) */
	struct ihex_data_segment *next; /* pointer to next data segment (two-dir list) */
} ihex_data_segment;

/**
 * Structure with object internal data fields.
 */
struct ihex_object {
	struct ihex_data_segment *segments; /* pointer to data segments list */
	uint8_t pad_byte; /* pad byte value, used to fill unassigned addresses */
	uint8_t align_record; /* align width in bytes, used in data dumping to ihex file */
	uint32_t extended_address; /* temporary field with extended address used in data parsing */
	int finished_flag; /* flag used to indicate EOF line in ihex file */
	ihex_error_e error; /* field with error code during operating */
};

/**
 * Create and return pointer to created object instance.
 * 
 * @return pointer to object instance
 */
struct ihex_object *ihex_new(void);

/**
 * Delete object instance and all internal references to data memory segments.
 * 
 * @param self pointer to object instance
 */
void ihex_delete(struct ihex_object *self);

/**
 * Method to get error description if any.
 * 
 * @param self pointer to object instance
 * @return pointer to string with error description. NULL if there are no any error.
 */
const char *ihex_get_error_string(struct ihex_object *self);

/**
 * Main method to parse intelhex file.
 * All data are stored internally in dynamically created data segments.
 * 
 * @param self pointer to object instance
 * @param fp pointer to file stream handler (read mode)
 * @return 0 if no error, else if error
 */
int ihex_parse_file(struct ihex_object *self, FILE *fp);

/**
 * Main method to dump intelhex file.
 * 
 * @param self pointer to object instance
 * @param fp pointer to file stream handler (write mode)
 * @return 0 if no error, else if error
 */
int ihex_dump_file(struct ihex_object *self, FILE *fp);

/**
 * Method used to add binary data to segments.
 * Auto check for data overlaping.
 * 
 * @param self pointer to object instance
 * @param adr start address where data should be placed
 * @param data pointer to data
 * @param size size of data to add
 * @return 0 if no error, else if error
 */
int ihex_set_data(struct ihex_object *self, uint32_t adr, uint8_t *data, uint32_t size);

/**
 * Method used to get binary data from segments.
 * Auto fill unused addresses.
 * 
 * @param self pointer to object instance
 * @param adr start address where from data should be read
 * @param data pointer to place where data should be write
 * @param size size of data to read
 * @return 0 if no error, else if error
 */
int ihex_get_data(struct ihex_object *self, uint32_t adr, uint8_t *data, uint32_t size);

#endif /* __IHEX_H */