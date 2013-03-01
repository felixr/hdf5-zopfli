HDF5 Zopfli filter 
===================

This an HDF5 compression filter based on Google's Zopfli compression library.

Zopfli is a zlib compatible compression algorithm. It is much slower than zlib, but yields better (=smaller) results.

In the included example the Zopfli filter yields a 9% smaller HDF5 file than the GZIP filter. 

TODO
-----
 * allow setting options (numiterations)

Links
------
 * Zopfli (https://code.google.com/p/zopfli/)
 * HDF5 (http://www.hdfgroup.org/HDF5/)
