# libihex
Plain C library for manipulating IntelHex files.

## Features
* robust intelhex parsing
* binary data segmentation
* dynamic memory allocation
* automatic records aligning
* automatic segments sorting and joining
* data overlapping detection
* CRLF and LF compatible
* small footprint, very fast and resources friendly
* padding byte for unspecified addresses
* trivial api
* unit tests
* error raports

## Build

Build and install:

```sh
cmake .
make
make check
sudo make install
```

## API
```c
struct ihex_object *ihex_new(void);
void ihex_delete(struct ihex_object *self);
const char *ihex_get_error_string(struct ihex_object *self);
int ihex_parse_file(struct ihex_object *self, FILE *fp);
int ihex_dump_file(struct ihex_object *self, FILE *fp);
int ihex_set_data(struct ihex_object *self, uint32_t adr, uint8_t *data, uint32_t size);
int ihex_get_data(struct ihex_object *self, uint32_t adr, uint8_t *data, uint32_t size);
```

See ```ihex.h``` header file for details.

## Examples

### Loading IntelHex file
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
        struct ihex_object *ihex;
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