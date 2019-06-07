#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <ihex.h>

static char input_hex[] = \
":020000040800F2\n"
":0C000400FFFF0120E50A0008290B00089E\n"
":04001000290B0008B0\n"
":020000040000FA\n"
":08FFF8003A3032303030303075\n"
":020000040001F9\n"
":10000000343038303046320A3A31303030303430E3\n"
":0800100030464646463031320D\n"
":00000001FF\n";

int main(int argc, char **argv)
{
	ihex_handler_t ihex;
        FILE *fp;
        struct ihex_data_segment *seg;

	ihex = ihex_new();
        assert(ihex != NULL);

	fp = fmemopen(input_hex, sizeof(input_hex), "r");
        assert(ihex_parse_file(ihex, fp) == 0);
	fclose(fp);

        assert(ihex->segments != NULL);
	seg = ihex->segments;

        assert(seg->adr_start == 0x0000FFF8);
        assert(seg->data_size == 32);
        assert(memcmp(seg->data, ":020000040800F2\n:10000400FFFF012", 32) == 0);

        assert(seg->next != NULL);
        seg = seg->next;

        assert(seg->adr_start == 0x08000004);
        assert(seg->data_size == 16);
        assert(memcmp(seg->data, "\xFF\xFF\x01\x20\xE5\x0A\x00\x08\x29\x0B\x00\x08\x29\x0B\x00\x08", 16) == 0);

	ihex_delete(ihex);

	return 0;
}
