ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=tpclient
CLIENT_SRCS=main.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -L./ -lftomcl -lftomc -lftm -lconfig -lcurl -lpthread  -lrt

all: $(CLIENT) 

$(CLIENT): $(CLIENT_OBJS) 
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(CLIENT_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(CLIENT) *.a *.o
