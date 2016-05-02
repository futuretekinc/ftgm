TOPDIR=../..
include ${TOPDIR}/Makefile.in

CLIENT=ftm_test
CLIENT_SRCS=main.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS)

LIBS= -lftm -lconfig -lpthread
all: $(LIB_FTDMC) $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) 
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS) $(LIBS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(SERVER) $(CLIENT) *.a *.o $(LIB_FTDM) $(LIB_FTDMC) 
