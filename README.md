# libihex
Plain C library for parsing and dumping IntelHex files.

## Features:
* robust intelhex parsing
* binary data segmentation
* dynamic memory allocation
* automatic records aligning
* automatic segments sorting and joining
* CRLF and LF compatible
* small footprint, very fast and resources friendly
* padding byte for unspecified addresses
* trivial api
* unit tests
* error raports

## Api:
```c
ihex_handler_t ihex_new(void);
void ihex_delete(ihex_handler_t self);

const char *ihex_get_error_string(ihex_handler_t self);

int ihex_parse_file(ihex_handler_t self, FILE *fp);
int ihex_dump_file(ihex_handler_t self, FILE *fp);

int ihex_set_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size);
int ihex_get_data(ihex_handler_t self, uint32_t adr, uint8_t *data, uint32_t size);
```

## Examples:

### Loading IntelHex file:
```c
#include <stdio.h>

/* include library header */
#include <ihex.h>

/* example intelhex data file content */
static char input_hex[] =
":020000040800F2\n"
":0400020001020304F0\n"
":00000001FF\n";

int main(int argc, char **argv)
{
        ihex_handler_t ihex;
        FILE *fp;
        unsigned char data[8];
        int i;

        /* create new intelhex parser instance */
	ihex = ihex_new();

        /* open file with intelhex data */
	fp = fmemopen(input_hex, sizeof(input_hex), "r");

        /* parsing intelhex stream */
        ihex_parse_file(ihex, fp);

        /* close file */
	fclose(fp);

        /* get binary data */
        ihex_get_data(ihex, 0x08000000, data, sizeof(data));

        /* print data output: FF FF 01 02 03 04 FF FF*/
        for (i = 0; i < sizeof(data); i++)
                printf("%02X ", data[i]);

        /* free resources */
        ihex_delete(ihex);

        return 0;
}
```