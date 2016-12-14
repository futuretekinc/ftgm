ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libnetclient
LIB_SRCS= ftom_net_client.c


LIB_OBJS=$(LIB_SRCS:.c=.o)
LIB_CFLAGS=$(CFLAGS) -std=gnu99

LIB_LDFLAGS=$(LDFLAGS) -L./ \
				-lftomcl -lftomc -lftom -lftm \
				-liothub_client\
				-liothub_client_mqtt_transport\
				-lumqtt\
				-laziotsharedutil\
				-lpthread\
				-lcurl\
				-lssl\
				-lcrypto\
				-luuid\
				-lcjson\
				-lm\
				-lrt 

all: $(LIB_OBJS) 
	rm -f $(TARGET).a $(TARGET).so.1.0.1
	$(AR) rcs $(TARGET).a $(LIB_OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(LIB_OBJS) $(LIB_LDFLAGS)

.c.o:  $(LIB_SRCS) *.h
	$(CC) $(LIB_CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o *.so.1.0.1
