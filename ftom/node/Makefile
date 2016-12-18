ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/Makefile.in

TARGET=libftomnode
SRCS= 	ftom_node.c\
		ftom_node_class.c\
		ftom_node_snmpc.c\
		ftom_node_fte.c\
		ftom_node_ftm.c\
		ftom_node_finsc.c\
		ftom_node_hhtw.c\
		ftom_node_virtual.c\
		ftom_node_fte_es7v.c
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
