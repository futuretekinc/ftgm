ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libmqttclient
SRCS= ftom_mqtt_client.c\
	ftom_mqtt_client_ft.c
OBJS=$(SRCS:.c=.o)

CFLAGS:=$(CFLAGS) -std=gnu99 

all: $(OBJS) 
	rm -f $(TARGET).a $(TARGET).so.1.0.1
	$(AR) rcs $(TARGET).a $(OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(OBJS) 

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o *.so.1.0.1
