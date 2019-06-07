#include "ihex.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

ihex_handler_t ihex_new(void)
{
	ihex_handler_t self;

	self = (ihex_handler_t)malloc(sizeof(struct ihex_object));
	if (self == NULL)
		return NULL;

	self->segments = NULL;
	self->pad_byte = 0xFF;
        self->align_record = 16;
	self->extended_address = 0;
	self->finished_flag = 0;
	self->error = IHEX_NO_ERROR;

	return self;
}

void ihex_delete(ihex_handler_t self)
{
	struct ihex_data_segment *seg;
	struct ihex_data_segment *seg_next;

	seg = self->segments;
	while (seg != NULL) {
		free(seg->data);
		seg_next = seg->next;
		free(seg);
		seg = seg_next;
	}

	free(self);
}

const char *ihex_get_error_string(ihex_handler_t self)
{
	switch (self->error) {
	case IHEX_NO_ERROR:
		return NULL;
	case IHEX_ERROR_DATA_OVERLAPPING:
		return "Data segments overlapping";
	case IHEX_ERROR_NO_EOF_LINE:
		return "No EOF line";
	case IHEX_ERROR_PARSING_START_LINE:
		return "Wrong start line char";
	case IHEX_ERROR_PARSING_HEX_ENCODE:
		return "Wrong ascii hex encoding";
	case IHEX_ERROR_PARSING_END_LINE:
		return "Wrong EOL char";
	case IHEX_ERROR_ADDRESS_FIELD:
		return "Incorrect address field";
	case IHEX_ERROR_LINE_LENGTH:
		return "Line length error";
	case IHEX_ERROR_CHECKSUM:
		return "Checksum error";
	case IHEX_ERROR_RECORD_TYPE:
		return "Unsupported record type";
	case IHEX_ERROR_MALLOC:
		return "Memory allocation error";
        case IHEX_ERROR_DUMP:
                return "Write dump stream error";
	default:
		return "Unknown error";
	}
	return NULL;
}

static int ihex_check_data_overlapping(ihex_handler_t self, uint32_t adr, uint32_t size)
{
	uint32_t seg_start_adr;
	uint32_t seg_finish_adr;

	uint32_t data_start_adr;
	uint32_t data_finish_adr;

	struct ihex_data_segment *seg;

	assert(self != NULL);

	if (size == 0)
		return 0;

	seg = self->segments;
	while (seg != NULL) {
		seg_start_adr = seg->adr_start;
		seg_finish_adr = seg->adr_start + seg->data_size - 1;

		data_start_adr = adr;
		data_finish_adr = adr + size - 1;

		if ((data_start_adr >= seg_start_adr) && (data_start_adr <= seg_finish_adr)) {
			self->error = IHEX_ERROR_DATA_OVERLAPPING;
			return -1;
		}
		if ((data_finish_adr >= seg_start_adr) && (data_finish_adr <= seg_finish_adr)) {
			self->error = IHEX_ERROR_DATA_OVERLAPPING;
			return -1;
		}
		if ((data_start_adr <= seg_start_adr) && (data_finish_adr >= seg_finish_adr)) {
			self->error = IHEX_ERROR_DATA_OVERLAPPING;
			return -1;
		}

		seg = seg->next;
	}

	return 0;
}

static int ihex_new_segment(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size)
{
	uint8_t *seg_data;
        struct ihex_data_segment *seg;
	struct ihex_data_segment *seg_new;

	assert(self != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	seg_new = (struct ihex_data_segment *)malloc(sizeof(ihex_data_segment));
	if (seg_new == NULL) {
		self->error = IHEX_ERROR_MALLOC;
		return -1;
	}
	seg_new->data = (uint8_t *)malloc(size);
	if (seg_new->data == NULL) {
		self->error = IHEX_ERROR_MALLOC;
		return -1;
	}
	seg_new->adr_start = adr;
	seg_new->data_size = size;

	memcpy(seg_new->data, data, size);

        if (self->segments == NULL) {
		self->segments = seg_new;
		seg_new->next = NULL;
		seg_new->prev = NULL;
                return 0;
	}

        seg = self->segments;
        while (1) {
                if (seg_new->adr_start < seg->adr_start) {
                        seg_new->prev = seg->prev;
                        seg_new->next = seg;
                        seg->prev = seg_new;
                        if (seg_new->prev == NULL)
                                self->segments = seg_new;
                        break;
                }
                
                if (seg->next == NULL) {
                        seg->next = seg_new;
                        seg_new->next = NULL;
                        seg_new->prev = seg;
                        break;
                }

                seg = seg->next;
        }

	return 0;
}

static int ihex_join_left(ihex_handler_t self, struct ihex_data_segment *seg_before, uint8_t *data, uint32_t size)
{
	assert(self != NULL);
	assert(seg_before != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	seg_before->data = (uint8_t *)realloc(seg_before->data, seg_before->data_size + size);
	if (seg_before->data == NULL) {
		self->error = IHEX_ERROR_MALLOC;
		return -1;
	}

	memcpy(&seg_before->data[seg_before->data_size], data, size);
	seg_before->data_size += size;

	return 0;
}

static int ihex_join_right(ihex_handler_t self, struct ihex_data_segment *seg_after, uint32_t adr, uint8_t *data, uint32_t size)
{
	assert(self != NULL);
	assert(seg_after != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	seg_after->data = (uint8_t *)realloc(seg_after->data, seg_after->data_size + size);
	if (seg_after->data == NULL) {
		self->error = IHEX_ERROR_MALLOC;
		return -1;
	}

	memmove(&seg_after->data[size], seg_after->data, seg_after->data_size);
	memcpy(seg_after->data, data, size);

	seg_after->data_size += size;
	seg_after->adr_start = adr;

	return 0;
}

static int ihex_insert_between(ihex_handler_t self, struct ihex_data_segment *seg_before, struct ihex_data_segment *seg_after, uint8_t *data, uint32_t size)
{
	assert(self != NULL);
	assert(seg_before != NULL);
	assert(seg_after != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	seg_before->data = (uint8_t *)realloc(seg_before->data, seg_before->data_size + size + seg_after->data_size);
	if (seg_before->data == NULL) {
		self->error = IHEX_ERROR_MALLOC;
		return -1;
	}

	memcpy(&seg_before->data[seg_before->data_size], data, size);
	seg_before->data_size += size;
	memcpy(&seg_before->data[seg_before->data_size], seg_after->data, seg_after->data_size);
	seg_before->data_size += seg_after->data_size;

	free(seg_after->data);
	if (seg_after->prev == NULL) {
		self->segments = seg_after->next;
		self->segments->prev = NULL;
	} else {
		seg_after->prev->next = seg_after->next;
	}
	free(seg_after);

        return 0;
}

int ihex_set_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size)
{
	struct ihex_data_segment *seg;
	struct ihex_data_segment *seg_before = NULL;
	struct ihex_data_segment *seg_after = NULL;

	assert(self != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	if (ihex_check_data_overlapping(self, adr, size) != 0)
		return -1;

	seg = self->segments;
	while (seg != NULL) {
		if (adr == (seg->adr_start + seg->data_size)) {
			seg_before = seg;
			break;
		}
		seg = seg->next;
	}

	seg = self->segments;
	while (seg != NULL) {
		if ((adr + size) == seg->adr_start) {
			seg_after = seg;
			break;
		}
		seg = seg->next;
	}

	if ((seg_before == NULL) && (seg_after == NULL)) {
		if (ihex_new_segment(self, adr, data, size) != 0)
			return -1;
	} else if ((seg_before != NULL) && (seg_after == NULL)) {
		if (ihex_join_left(self, seg_before, data, size) != 0)
			return -1;
	} else if ((seg_before == NULL) && (seg_after != NULL)) {
		if (ihex_join_right(self, seg_after, adr, data, size) != 0)
			return -1;
	} else {
		if (ihex_insert_between(self, seg_before, seg_after, data, size) != 0)
			return -1;
	}

	return 0;
}

static int ihex_get_hex_nibble(const char ch, uint8_t *nibble)
{
	if ((ch >= '0') && (ch <= '9')) {
		*nibble = ch - '0';
	} else if ((ch >= 'A') && (ch <= 'F')) {
		*nibble = ch - 'A' + 10;
	} else if ((ch >= 'a') && (ch <= 'f')) {
		*nibble = ch - 'a' + 10;
	} else {
		return -1;
	}
	return 0;
}

static int ihex_get_hex_number(char const *hex, uint8_t *byte, uint8_t size)
{
	uint8_t b;
	uint8_t *out;

	out = &byte[size - 1];
	while (size > 0) {
		if (ihex_get_hex_nibble(*hex++, &b) != 0)
			return -1;
		*out = b << 4;
		if (ihex_get_hex_nibble(*hex++, &b) != 0)
			return -1;
		*out |= b;
		out--;
		size--;
	}

	return 0;
}

static int ihex_new_record(ihex_handler_t self, uint8_t size, uint16_t adr, uint8_t type, uint8_t *data)
{
	assert(self != NULL);

	switch (type) {
	case 0x00:
		if (ihex_set_data(self, (uint32_t)adr + self->extended_address, data, size) != 0)
			return -1;
		break;
	case 0x01:
		self->finished_flag = 1;
		break;
	case 0x04:
		if (adr != 0) {
			self->error = IHEX_ERROR_ADDRESS_FIELD;
			return -1;
		}
		self->extended_address = data[0] << 24 | data[1] << 16;
		break;
	case 0x05:
		break;
	default:
		self->error = IHEX_ERROR_RECORD_TYPE;
		return -1;
	}
	return 0;
}

static int ihex_parse_record(ihex_handler_t self, const char *record_line)
{
	uint8_t data_size;
	uint8_t index;
	uint16_t address;
	uint8_t record_type;
	uint8_t checksum;
	uint8_t sum;
	uint8_t *data;
	char ch;

	size_t line_length;

	assert(self != NULL);
	assert(record_line != NULL);

	line_length = strlen(record_line);

	if (line_length < 12) {
		self->error = IHEX_ERROR_LINE_LENGTH;
		return -1;
	}
	if (record_line[0] != ':') {
		self->error = IHEX_ERROR_PARSING_START_LINE;
		return -1;
	}
	if (ihex_get_hex_number(&record_line[1], &data_size, 1) != 0) {
		self->error = IHEX_ERROR_PARSING_HEX_ENCODE;
		return -1;
	}
	if (ihex_get_hex_number(&record_line[3], (uint8_t *)&address, 2) != 0) {
		self->error = IHEX_ERROR_PARSING_HEX_ENCODE;
		return -1;
	}
	if (ihex_get_hex_number(&record_line[7], &record_type, 1) != 0) {
		self->error = IHEX_ERROR_PARSING_HEX_ENCODE;
		return -1;
	}

	if (line_length - 12 < (int)data_size * 2) {
		self->error = IHEX_ERROR_LINE_LENGTH;
		return -1;
	}

	sum = data_size;
	sum += (uint8_t)(address >> 8);
	sum += (uint8_t)(address & 0x00FF);
	sum += record_type;

	index = 0;
	data = NULL;
	if (data_size > 0) {
		data = (uint8_t *)malloc(data_size);
		while (index < data_size) {
			if (ihex_get_hex_number(&record_line[(uint16_t)9 + (uint16_t)((uint16_t)index * 2)], &data[index], 1) != 0) {
				free(data);
				self->error = IHEX_ERROR_PARSING_HEX_ENCODE;
				return -1;
			}
			sum += data[index];
			index++;
		}
	}

	if (ihex_get_hex_number(&record_line[(uint16_t)9 + (uint16_t)((uint16_t)index * 2)], &checksum, 1) != 0) {
		if (data != NULL)
			free(data);
		self->error = IHEX_ERROR_PARSING_HEX_ENCODE;
		return -1;
	}

	sum += checksum;

	if ((uint8_t)(sum & 0xFF) != 0) {
		if (data != NULL)
			free(data);
		self->error = IHEX_ERROR_CHECKSUM;
		return -1;
	}

	ch = record_line[(uint16_t)11 + (uint16_t)((uint16_t)index * 2)];
	if ((ch != '\n') && (ch != '\r')) {
		if (data != NULL)
			free(data);
		self->error = IHEX_ERROR_PARSING_END_LINE;
		return -1;
	}

	if (ihex_new_record(self, data_size, address, record_type, data) != 0) {
		if (data != NULL)
			free(data);
		return -1;
	}

	if (data != NULL)
		free(data);
	return 0;
}

int ihex_parse_file(ihex_handler_t self, FILE *fp)
{
	char *line = NULL;
	size_t len = 0;
	int size;
	int s;

	assert(self != NULL);
	assert(fp != NULL);

	while ((size = getline(&line, &len, fp)) >= 0) {
		if (ihex_parse_record(self, line) != 0) {
			if (line != NULL)
				free(line);
			return -1;
		}
		if (self->finished_flag != 0)
			break;
	}

	if (self->finished_flag == 0) {
		if (line != NULL)
			free(line);
		self->error = IHEX_ERROR_NO_EOF_LINE;
		return -1;
	}

	if (line != NULL)
		free(line);

	return 0;
}

int ihex_get_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size)
{
	struct ihex_data_segment *seg;
	int exist_flag;

	assert(self != NULL);
	assert(data != NULL);

	if (size == 0)
		return 0;

	seg = self->segments;
	if (seg == NULL) {
		memset(data, self->pad_byte, size);
		return 0;
	}

	while (size > 0) {
		if ((adr >= seg->adr_start) && (adr < (seg->adr_start + seg->data_size))) {
			*data++ = seg->data[adr - seg->adr_start];
			size--;
			adr++;
		} else {
			seg = self->segments;
			exist_flag = 0;
			while (seg != NULL) {
				if ((adr >= seg->adr_start) && (adr < (seg->adr_start + seg->data_size))) {
					exist_flag = 1;
					break;
				}
				seg = seg->next;
			}
			if (exist_flag == 0) {
				seg = self->segments;
				*data++ = self->pad_byte;
				size--;
				adr++;
			}
		}
	}

	return 0;
}

static int ihex_dump_record(FILE *fp, uint16_t adr, uint8_t type, uint8_t *data, uint8_t size)
{
        uint8_t sum;
        uint8_t crc;
        int s;

	assert(fp != NULL);

        s = fprintf(fp, ":%02X%04X%02X", size, adr, type);
        if (s != 9)
                return -1;

        sum = size;
	sum += (uint8_t)(adr >> 8);
	sum += (uint8_t)(adr & 0x00FF);
	sum += type;

        while (size > 0) {
                s = fprintf(fp, "%02X", *data);
                if (s != 2)
                        return -1;
                sum += *data++;
                size--;
        }

        crc = 0x100 - sum;
        s = fprintf(fp, "%02X\n", crc);
        if (s != 3)
                return -1;

        return 0;
}

static int ihex_dump_segment(ihex_handler_t self, struct ihex_data_segment *seg, FILE *fp, uint32_t *old_address)
{
        uint32_t new_address;
        uint32_t total;
        uint32_t remained;
        uint32_t max_rec_size;
        uint32_t rec_size;
        uint8_t adr_data[2];

        assert(seg != NULL);
	assert(fp != NULL);
        assert(old_address != NULL);

        total = 0;
        while (total < seg->data_size) {
                new_address = seg->adr_start + total;
                max_rec_size = self->align_record - new_address % self->align_record;
                rec_size = max_rec_size;
                remained = seg->data_size - total;
                if (rec_size > remained)
                        rec_size = remained;

                if ((new_address & 0xFFFF0000) != (*old_address & 0xFFFF0000)) {
                        adr_data[0] = new_address >> 24;
                        adr_data[1] = (new_address >> 16) & 0xFF;
                        if (ihex_dump_record(fp, 0, 0x04, adr_data, 2) != 0) {
                                self->error = IHEX_ERROR_DUMP;
                                return -1;
                        }
                }

                if (ihex_dump_record(fp, new_address & 0xFFFF, 0x00, &seg->data[total], rec_size) != 0){
                        self->error = IHEX_ERROR_DUMP;
                        return -1;
                }

                *old_address = new_address;
                total += rec_size;
        }

        return 0;
}

int ihex_dump_file(ihex_handler_t self, FILE *fp)
{
        struct ihex_data_segment *seg;
        uint32_t old_address;

	assert(self != NULL);
	assert(fp != NULL);

        old_address = 0;
	seg = self->segments;
        while (seg != NULL) {
                if (ihex_dump_segment(self, seg, fp, &old_address) != 0)
                        return -1;
                seg = seg->next;
        }

        if (ihex_dump_record(fp, 0, 0x01, NULL, 0) != 0) {
                self->error = IHEX_ERROR_DUMP;
                return -1;
        }

        return 0;
}