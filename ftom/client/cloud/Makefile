ifeq ($(TOPDIR),)
TOPDIR=$(shell PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libcloudclient
SRCS= ftom_cloud_client.c
OBJS=$(SRCS:.c=.o)

CFLAGS:=$(CFLAGS) -std=gnu99 


SUBDIR=azure thingplus

all: $(TARGET) 
	for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

$(TARGET): $(OBJS) 
	rm -f $(TARGET).a $(TARGET).so.1.0.1
	$(AR) rcs $(TARGET).a $(OBJS)
	$(CC) -shared -o $(TARGET).so.1.0.1 $(OBJS) 

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

install:
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

clean:
	rm -f *.a *.o *.so.1.0.1
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

