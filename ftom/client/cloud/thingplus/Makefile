ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libtpclient
SRCS=ftom_tp_client.c\
		ftom_tp_restapi.c\
		ftom_mqtt_client_tpgw.c\
		ftom_tp_object.c

OBJS=$(SRCS:.c=.o)

all: $(OBJS)
	$(AR) rcs $(TARGET).a $(OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(OBJS) $(LDFLAGS)

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o *.so.1.0.1

