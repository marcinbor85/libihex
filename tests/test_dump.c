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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <ihex.h>

static char input_hex[] =
":020000040800F2\n"
":0C000400FFFF0120E50A0008290B00089E\n"
":04001000290B0008B0\n"
":020000040000FA\n"
":08FFF8003A3032303030303075\n"
":020000040001F9\n"
":10000000343038303046320A3A31303030303430E3\n"
":0800100030464646463031320D\n"
":00000001FF\n";

static char output_hex[] =
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