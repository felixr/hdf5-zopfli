/*
    Copyright (C) 2009 Andrew Collette
    http://h5py.alfven.org
    License: BSD (see LICENSE.txt)

    Example program demonstrating use of the zopfli filter from C code.

    To compile this program:

    h5cc -DH5_USE_16_API zopfli/*.c zopfli_filter.c example.c -o example

    To run:

    $ ./example
    Success!
    $ h5ls -v test_zopfli.hdf5 
    Opened "test_zopfli.hdf5" with sec2 driver.
    dset                     Dataset {100/100, 100/100, 100/100}
        Location:  0:1:0:976
        Links:     1
        Modified:  2009-02-15 16:35:11 PST
        Chunks:    {1, 100, 100} 40000 bytes
        Storage:   4000000 logical bytes, 174288 allocated bytes, 2295.05% utilization
        Filter-0:  shuffle-2 OPT {4}
        Filter-1:  zopfli-32000 OPT {1, 261, 40000}
        Type:      native float
*/

#include "hdf5.h"
#include <stdio.h>
#include "zopfli_filter.h"

#define SIZE 10*100*100
#define SHAPE {10,100,100}
#define CHUNKSHAPE {1,100,100}


#define FILENAME        "test_zopfli.hdf5"
#define DATASET         "dset"


int main(){

    static float data[SIZE];
    static float data_out[SIZE];
    const hsize_t shape[] = SHAPE;
    const hsize_t chunkshape[] = CHUNKSHAPE;
    int r, i;
    int return_code = 1;

    hid_t fid, sid, dset, plist = 0;

    for(i=0; i<SIZE; i++){
        data[i] = i;
    }

    /* Register the filter with the library */
    r = register_zopfli();
    if(r<0) goto failed;

#if 1
    sid = H5Screate_simple(3, shape, NULL);
    if(sid<0) goto failed;

    fid = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if(fid<0) goto failed;

    plist = H5Pcreate(H5P_DATASET_CREATE);
    if(plist<0) goto failed;

    /* Chunked layout required for filters */
    r = H5Pset_chunk(plist, 3, chunkshape);
    if(r<0) goto failed;

    /* Use of the shuffle filter VASTLY improves performance of this
       and other block-oriented compression filters.  Be sure to add
       this before the compression filter!
    */
    r = H5Pset_shuffle(plist);
    if(r<0) goto failed;

    /* Note the "optional" flag is necessary, as with the DEFLATE filter */
    r = H5Pset_filter(plist, H5_FILTER_ZOPFLI, H5Z_FLAG_OPTIONAL, 0, NULL);
    /* r = H5Pset_filter(plist,H5Z_FILTER_DEFLATE, H5Z_FLAG_OPTIONAL, 0, NULL); */
    if(r<0) goto failed;

    dset = H5Dcreate(fid, DATASET, H5T_NATIVE_FLOAT, sid, plist);
    if(dset<0) goto failed;
    
    r = H5Dwrite(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data);
    if(r<0) goto failed;

   /*
     * Close and release resources.
     */
    H5Pclose (plist);
    H5Dclose (dset);
    H5Sclose (sid);
    sid =  0;
    H5Fclose (fid);
#endif


    /*
     * Now we begin the read section of this example.
     */

    /*
     * Open file and dataset using the default properties.
     */
    fid = H5Fopen (FILENAME, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset = H5Dopen (fid, DATASET);

    /*
     * Retrieve dataset creation property list.
     */
     plist = H5Dget_create_plist (dset);

    /*
     * Retrieve and print the filter type.  Here we only retrieve the
     * first filter because we know that we only added one filter.
     */
    H5Z_filter_t    filter_type;
    unsigned int filter_info, flags ;
    size_t   nelmts = 0;

// H5Z_filter_t H5Pget_filter( hid_t plist_id, unsigned idx, unsigned int *flags, size_t *cd_nelmts, unsigned cd_values[], size_t namelen, char name[], unsigned *filter_config )
    filter_type = H5Pget_filter2(plist, 1, &flags, &nelmts, NULL, 0, NULL, &filter_info);
    printf ("Filter type is: %d\n ", filter_type);
    r = H5Dread(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data_out);
    if(r<0) goto failed;

    for(i=0;i<SIZE;i++){
        if(data[i] != data_out[i]) goto failed;
    }


    fprintf(stdout, "Success!\n");

    return_code = 0;

    failed:

    if (dset>0)  H5Dclose(dset);
    if (sid>0)   H5Sclose(sid);
    if (plist>0) H5Pclose(plist);
    if (fid>0)   H5Fclose(fid);

    return return_code;
}

