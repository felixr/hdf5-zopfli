#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "hdf5.h"
#include "zopfli_filter.h"
#include "zopfli/deflate.h"
#include "zopfli/gzip_container.h"
#include "zopfli/zlib_container.h"
#include "zlib.h"

#define PUSH_ERR(func, minor, str)  H5Epush1(__FILE__, func, __LINE__, H5E_PLINE, minor, str)

static size_t zopfli_filter(unsigned flags, size_t cd_nelmts,
        const unsigned cd_values[], size_t nbytes, size_t *buf_size, void **buf);

int register_zopfli(void){
    int retval;

    H5Z_class2_t filter_class = {
        H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
        (H5Z_filter_t)H5_FILTER_ZOPFLI,		/* Filter id number		*/
        1,              /* encoder_present flag (set to true) */
        1,              /* decoder_present flag (set to true) */
        "zopfli",			/* Filter name for debugging	*/
        NULL,                       /* The "can apply" callback     */
        NULL,                       /* The "set local" callback     */
        (H5Z_func_t)zopfli_filter,         /* The actual filter function	*/
    };

    retval = H5Zregister(&filter_class);
    if(retval<0){
        PUSH_ERR("register_zopfli", H5E_CANTREGISTER, "Can't register Zopfli filter");
    }
    return retval;
}

#define H5Z_DEFLATE_SIZE_ADJUST(s) (HDceil(((double)(s))*1.001)+12)


    static size_t
zopfli_filter(unsigned flags, size_t cd_nelmts,
        const unsigned cd_values[], size_t nbytes,
        size_t *buf_size, void **buf)
{
    /* void	*outbuf = NULL;         /1* Pointer to new buffer *1/ */
    unsigned char* out = NULL;
    size_t outsize = 0;
    int		status; 
    size_t	ret_value;              /* Return value */


    /* compress ? */
    if(!(flags & H5Z_FLAG_REVERSE)){
        Options options;
        InitOptions(&options);
        options.numiterations = 15;
        /* options.verbose = 1; */

        unsigned char bp = 0;
        ZlibCompress(&options, *buf, nbytes, &out, &outsize);
        status = outsize;
    }else{
        /* Input; uncompress */
        z_stream	z_strm;                 /* zlib parameters */
        outsize = *buf_size;     /* Number of bytes for output (compressed) buffer */

        /* Allocate space for the compressed buffer */
        out = (unsigned char *)malloc(outsize);

        /* Set the uncompression parameters */
        memset(&z_strm, 0, sizeof(z_strm)); 
        z_strm.next_in = (Bytef *)*buf;
        z_strm.avail_in = nbytes;
        z_strm.next_out = (Bytef *)out;
        z_strm.avail_out = outsize;

        /* Initialize the uncompression routines */
        if (Z_OK != inflateInit(&z_strm)) {
            PUSH_ERR("zopfli_filter", H5E_CANTINIT, "inflateInit() failed");
        }
        /* Loop to uncompress the buffer */
        do {
            /* Uncompress some data */
            status = inflate(&z_strm, Z_SYNC_FLUSH);

            /* Check if we are done uncompressing data */
            if (Z_STREAM_END == status) {
                break;	/*done*/
            }

            /* Check for error */
            if (Z_OK != status) {
                (void)inflateEnd(&z_strm);
                PUSH_ERR("zopfli_filter", H5E_CANTINIT, "inflate() failed");
            } else {
                /* If we're not done and just ran out of buffer space, get more */
                if(0 == z_strm.avail_out) {
                    void	*new_outbuf;         /* Pointer to new output buffer */

                    /* Allocate a buffer twice as big */
                    outsize *= 2;
                    if (NULL == (new_outbuf = realloc(out, outsize))) {
                        (void)inflateEnd(&z_strm);
                        PUSH_ERR("zopfli_filter", H5E_NOSPACE, "memory allocation failed for deflate uncompression");
                    }
                    out = new_outbuf;

                    /* Update pointers to buffer for next set of uncompressed data */
                    z_strm.next_out = (unsigned char*)out + z_strm.total_out;
                    z_strm.avail_out = (uInt)(outsize - z_strm.total_out);
                } /* end if */
            } /* end else */
        } while(status==Z_OK);

        status = z_strm.total_out;

        /* Finish uncompressing the stream */
        (void)inflateEnd(&z_strm);
    }

    if(status != 0){
        free(*buf);
        *buf = out;
        *buf_size = outsize;

        return status;  /* Size of compressed/decompressed data */
    } 
failed:

    free(out);
    return 0;
}
