ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libtpclient
LIB_SRCS=ftom_tp_client.c\
				ftom_tp_restapi.c\
				ftom_mqtt_client_tpgw.c\
				ftom_tp_object.c

LIB_OBJS=$(LIB_SRCS:.c=.o)

all: $(LIB_OBJS)
	$(AR) rcs $(TARGET).a $(LIB_OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(LIB_OBJS) $(LDFLAGS)

.c.o:  $(LIB_SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o *.so.1.0.1

