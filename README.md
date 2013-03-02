HDF5 Zopfli filter 
===================

This an HDF5 compression filter based on Google's Zopfli compression library.

Zopfli is a zlib compatible compression algorithm. It is much slower than zlib, but yields better (=smaller) results.

Example results
----------------

*Note: the results from this examples are not representative for all possible data sets*

In the included example a 3-dimensional data set with 100,000 floats is written using standard GZIP filter and the Zopfli filter. Below you see the `h5ls -v` output for both files. The zopfli compressed file uses 6872 bytes to store the 100,000 floats while the GZIP compressed file uses 7938 bytes. 
Hence, in this example the zopfli filter saves 13% of storage.

**Standard GZIP filter**

    Opened "test_gzip.hdf5" with sec2 driver.
    DS1                      Dataset {10/10, 100/100, 100/100}
        Location:  1:800
        Links:     1
        Chunks:    {1, 100, 100} 40000 bytes
        Storage:   400000 logical bytes, 7938 allocated bytes, 5039.05% utilization
        Filter-0:  shuffle-2 OPT {4}
        Filter-1:  deflate-1 OPT {9}
        Type:      native float

**Zopfli filter**

    Opened "test_zopfli.hdf5" with sec2 driver.
    DS1                      Dataset {10/10, 100/100, 100/100}
        Location:  1:800
        Links:     1
        Chunks:    {1, 100, 100} 40000 bytes
        Storage:   400000 logical bytes, 6872 allocated bytes, 5820.72% utilization
        Filter-0:  shuffle-2 OPT {4}
        Filter-1:  zopfli-256 OPT {5}
        Type:      native float


Links
------
 * Zopfli (https://code.google.com/p/zopfli/)
 * HDF5 (http://www.hdfgroup.org/HDF5/)
