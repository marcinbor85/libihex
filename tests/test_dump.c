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

static char output_hex[] = \
":08FFF8003A3032303030303075\n"
":020000040001F9\n"
":10000000343038303046320A3A31303030303430E3\n"
":0800100030464646463031320D\n"
":020000040800F2\n"
":0C000400FFFF0120E50A0008290B00089E\n"
":04001000290B0008B0\n"
":00000001FF\n";

int main(int argc, char **argv)
{
	ihex_handler_t ihex;
        FILE *fp;
        struct ihex_data_segment *seg;
        char out[512];

	ihex = ihex_new();
        assert(ihex != NULL);

	fp = fmemopen(input_hex, sizeof(input_hex), "r");
        assert(ihex_parse_file(ihex, fp) == 0);
	fclose(fp);

	fp = fmemopen(out, sizeof(out), "w");
	assert(ihex_dump_file(ihex, fp) == 0);
        fclose(fp);

        assert(memcmp(out, output_hex, sizeof(output_hex)) == 0);

	ihex_delete(ihex);

	return 0;
}