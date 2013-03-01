#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "zopfli_filter.h"

#define DATASET_NAME   "DS1"


#define DIM 3
#define DIM0 10 
#define DIM1 100 
#define DIM2 100 


#define SIZE (DIM0 * DIM1 * DIM2)
#define SHAPE {DIM0, DIM1, DIM2}
#define CHUNK_SHAPE {1, DIM1, DIM2}

void print_filter_name(hid_t dcpl);
void create_and_test_file(hid_t dcpl, const char *file_name);
bool check_for_gzip();
int get_file_size(const char *file_name);

int main() {
    hid_t dcpl;
    herr_t status;
    hsize_t chunk_shape[] = CHUNK_SHAPE;

    /* Register the filter with the library */
    status = register_zopfli();

    /* dclp = dataset creation property list */

    /* GZIP COMPRESSION */
    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(dcpl, DIM, chunk_shape);
    status = H5Pset_shuffle(dcpl);
    status = H5Pset_deflate(dcpl, 9);
    create_and_test_file(dcpl, "test_gzip.hdf5");

    /* ZOPFLI COMPRESSION */
    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(dcpl, DIM,  chunk_shape);
    status = H5Pset_shuffle(dcpl);
    status = H5Pset_filter(dcpl, H5_FILTER_ZOPFLI, H5Z_FLAG_OPTIONAL, 0, NULL);
    create_and_test_file(dcpl, "test_zopfli.hdf5");

    int size_gzip = get_file_size("test_gzip.hdf5"); 
    int size_zopfli = get_file_size("test_zopfli.hdf5");

    printf("GZIP file size: %d\n", size_gzip);
    printf("Zopfli file size: %d\n",  size_zopfli);

    printf("Zopfli/GZIP: %0.4f\n",  (float)size_zopfli/(float)size_gzip);
}

void create_and_test_file(hid_t dcpl, const char *file_name) {
    hid_t           file, space, dset;    /* Handles */
    herr_t          status;
    htri_t          avail;
    H5Z_filter_t    filter_type;
    unsigned int    filter_info;
    hsize_t         dset_shape[] = SHAPE;
    float wdata[SIZE],   rdata[SIZE];
    int i;

    /* Initialize data.  */
    for (i=0; i<SIZE; i++) {
        wdata[i] = (float)i ;
    }

    /* Create a new file using the default properties.  */
    file = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /*
     * Create dataspace.  Setting maximum size to NULL sets the maximum
     * size to be the current size.
     */
    space = H5Screate_simple(DIM, dset_shape, NULL); 

    /* Create the dataset. */
    dset = H5Dcreate(file, DATASET_NAME, H5T_NATIVE_FLOAT, space, dcpl);

    /* Write the data to the dataset.  */
    status = H5Dwrite (dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &wdata);

    /* Close and release resources.  */
    status = H5Pclose (dcpl);
    status = H5Dclose (dset);
    status = H5Sclose (space);
    status = H5Fclose (file);


    /* Open file and dataset using the default properties.  */
    file = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset = H5Dopen(file, DATASET_NAME);

    /* Retrieve dataset creation property list.  */
    dcpl = H5Dget_create_plist(dset);

    print_filter_name(dcpl);

    /* Read the data using the default properties. */
    status = H5Dread(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rdata);

    /*
     * Find the maximum value in the dataset, to verify that it was
     * read correctly.
     */
    bool data_correct = true;
    for (i=0; i<SIZE; i++) {
        if ( rdata[i] != wdata[i] ) { 
        /* if ( abs(rdata[i] - wdata[i]) > 0.0001) { */
            printf("data mismatch %d %f %f\n", i,  rdata[i], wdata[i]);
            data_correct = false;
            break;
        }
    }

    printf("Data in file matches written data: %s\n", (data_correct == true ? "YES" : "NO"));


    /* Close and release resources. */
    status = H5Pclose (dcpl);
    status = H5Dclose (dset);
    status = H5Fclose (file);
}

bool check_for_gzip() {
    H5Z_filter_t    filter_type;
    unsigned int    filter_info;
    herr_t status;
    htri_t avail;
    avail = H5Zfilter_avail(H5Z_FILTER_DEFLATE);
    if (!avail) {
        printf ("gzip filter not available.\n");
        return false;
    }
    status = H5Zget_filter_info (H5Z_FILTER_DEFLATE, &filter_info);
    if ( !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
            !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ) {
        printf ("gzip filter not available for encoding and decoding.\n");
        return false;
    }
    return true;
}


/*
 * Retrieve and print the filter type. 
 */
void print_filter_name(hid_t dcpl) {
    size_t          nelmts;
    H5Z_filter_t    filter_type;
    unsigned int    flags, filter_info;

    nelmts = 0;
    /* retrieve second filter; the first one is SHUFFLE the second the compression */
    filter_type = H5Pget_filter2(dcpl, 1, &flags, &nelmts, NULL, 0, NULL, &filter_info);
    printf ("Filter type is: ");
    switch (filter_type) {
    case H5_FILTER_ZOPFLI:
        printf ("H5_FILTER_ZOPFLI\n");
        break;
    case H5Z_FILTER_DEFLATE:
        printf ("H5Z_FILTER_DEFLATE\n");
        break;
    case H5Z_FILTER_SHUFFLE:
        printf ("H5Z_FILTER_SHUFFLE\n");
        break;
    case H5Z_FILTER_FLETCHER32:
        printf ("H5Z_FILTER_FLETCHER32\n");
        break;
    case H5Z_FILTER_SZIP:
        printf ("H5Z_FILTER_SZIP\n");
        break;
    case H5Z_FILTER_NBIT:
        printf ("H5Z_FILTER_NBIT\n");
        break;
    case H5Z_FILTER_SCALEOFFSET:
        printf ("H5Z_FILTER_SCALEOFFSET\n");
    }
}

int get_file_size(const char *file_name) {
    struct stat file_status;
    if (stat(file_name, &file_status) != 0){
        return 0;
    }
    return file_status.st_size;
}
