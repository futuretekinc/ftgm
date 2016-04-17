TOPDIR=../..
include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
CLIENT=ftomc
CLIENT_SRCS=main.c\
			ftom_client_cmdline.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftm -lconfig -lpthread 

all: tags $(CLIENT) $(LIB_FTOMC) 

tags:
	ctags -R
	
$(CLIENT): $(CLIENT_OBJS) $(LIB_FTOMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(CLIENT_SRC) $(LIB_FTOMC_OBJS) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(CLIENT) *.a *.o
