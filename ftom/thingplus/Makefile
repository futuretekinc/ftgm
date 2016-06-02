ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=tpclient
CLIENT_SRCS=main.c \
			ftom_tp_client.c\
			ftom_tp_gateway.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -L./ -lftomcl -lftomc -lftm -lconfig -lcurl -lcjson -lpthread  -lm -lrt

all: $(CLIENT) 

$(CLIENT): $(CLIENT_OBJS) 
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(CLIENT_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(CLIENT) *.a *.o
