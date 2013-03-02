#ifndef H5PY_ZOPFLI_H
#define H5PY_ZOPFLI_H

/* Filter ID from the testing range (-> not for production use)
 * http://www.hdfgroup.org/HDF5/doc/H5.user/Filters.html
 */ 
#define H5_FILTER_ZOPFLI 256 

#ifdef __cplusplus
extern "C" {
#endif

/* Register the filter with the library. Returns a negative value on failure, 
   and a non-negative value on success.
*/
int register_zopfli(void);

#ifdef __cplusplus
}
#endif

#endif

