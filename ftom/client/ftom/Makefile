ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

export TOPDIR

SUBDIR=net 

TARGET=ftomclient

INCS= $(foreach dir, . $(SUBDIR), $(wildcard $(dir)/*.h))
SRCS= $(foreach dir, . $(SUBDIR), $(wildcard $(dir)/*.c))

OBJS=$(SRCS:.c=.o)

CFLAGS:=$(CFLAGS) -I$(TOPDIR)/libftm


all: lib$(TARGET).a lib$(TARGET).so

lib$(TARGET).a: $(OBJS)
	$(AR) rcs $(TARGET).a $(OBJS)

lib$(TARGET).so: $(OBJS)
	$(CC) -shared -o $(TARGET).so $(OBJS) 

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@


install:
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

clean:
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

