ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

TARGET=cloudclient
SUBDIR=azure thingplus
SRCS= $(foreach dir, . $(SUBDIR),$(wildcard $(dir)/*.c))

OBJS=$(SRCS:.c=.o)

all: lib$(TARGET).a

lib$(TARGET).a: $(OBJS) 
	$(AR) rcs lib$(TARGET).a $(OBJS)

lib$(TARGET).so: $(OBJS) 
	$(CC) -shared -o lib$(TARGET).so $(OBJS) 

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

