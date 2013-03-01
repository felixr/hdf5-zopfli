#ifndef H5PY_ZOPFLI_H
#define H5PY_ZOPFLI_H

/* Filter revision number, starting at 1 */
#define H5_FILTER_ZOPFLI_VERSION 1

/* Filter ID registered with the HDF Group as of 2/6/09.  For maintenance
   requests, contact the filter author directly. */
#define H5_FILTER_ZOPFLI 2600 

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

