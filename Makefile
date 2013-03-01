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

example: example.c zopfli_filter.c $(ZOPFLI_OBJ)
	gcc -g zopfli_filter.c example.c $(ZOPFLI_OBJ) -o example -lhdf5 -lz

zopfli_filter.o: zopfli_filter.c  zopfli_filter.h 
	gcc -c $< 


libzopfli_filter.so: zopfli_filter.c 
	# gcc -I. -O2 -lhdf5 -fPIC -shared ../zopfli-git/*.o zopfli_filter.c -o $@ 
	# example: example.c zopfli_filter.c zopfli_filter.h 

