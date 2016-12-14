ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libazclient
LIB_SRCS= ftom_azure_client.c


LIB_OBJS=$(LIB_SRCS:.c=.o)
LIB_CFLAGS=$(CFLAGS) -std=gnu99 \
				-I/usr/include/azureiot \
				-I/usr/include/azureiot/inc\
				-DARCHITECTURE_x86=1

LIB_LDFLAGS=$(LDFLAGS) -L./ \
				-L/usr/lib/azureiot \
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
