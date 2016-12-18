ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libazclient
SRCS= ftom_azure_client.c
OBJS=$(SRCS:.c=.o)

CFLAGS:=$(CFLAGS) -std=gnu99 \
				-I/usr/include/azureiot \
				-I/usr/include/azureiot/inc\
				-DARCHITECTURE_x86=1

all: $(OBJS) 
	rm -f $(TARGET).a $(TARGET).so.1.0.1
	$(AR) rcs $(TARGET).a $(OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(OBJS) 

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o *.so.1.0.1
