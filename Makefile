CC=gcc
CFLAGS+=-fPIC -O2
ZOPFLI_SRC=zopfli/blocksplitter.c \
		   zopfli/cache.c \
		   zopfli/deflate.c \
		   zopfli/gzip_container.c \
		   zopfli/hash.c \
		   zopfli/katajainen.c \
		   zopfli/lz77.c \
		   zopfli/squeeze.c \
		   zopfli/tree.c \
		   zopfli/util.c \
		   zopfli/zlib_container.c

ZOPFLI_OBJ=$(patsubst %.c,%.o,$(ZOPFLI_SRC))

all: example libzopfli_filter.so

example: example.c zopfli_filter.c $(ZOPFLI_OBJ)
	$(CC) $(CFLAGS) zopfli_filter.c example.c $(ZOPFLI_SRC) -o example -lhdf5 -lz

libzopfli_filter.so: zopfli_filter.c $(ZOPFLI_OBJ)
	$(CC) $(CFLAGS) $(ZOPFLI_OBJ) zopfli_filter.c -shared -o $@ 

clean:
	rm -f $(ZOPFLI_OBJ)
	rm -f example
	rm -f *.o *.so
	rm -f test_zopfli.hdf5 

.PHONY: clean
