#include <zlib.h>
#include <stdio.h>
#include "compress.h"

int inflate_data(char *in, int in_size, char *out, int out_size) {
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = in_size;
    strm.next_in = in;
    strm.avail_out = out_size;
    strm.next_out = out;

    inflateInit(&strm);
    int ret;
    if ((ret = inflate(&strm, Z_FINISH)) != Z_STREAM_END) {
        printf("inflate: return code %d\n", ret);
    }
    inflateEnd(&strm);

    return strm.total_out;
}
