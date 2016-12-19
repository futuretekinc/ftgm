ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/IncludePre.mk

MODBUS_TEST=ftom_node_modbus_test
MODBUS_TEST_SRCS=ftom_node_modbus_test.c
MODBUS_TEST_OBJS=$(MODBUS_TEST_SRCS:.c=.o)

MODBUS_TEST_LDFLAGS=$(LDFLAGS) -lftomc -lftm 

TEST_MAIN=test

all: $(MODBUS_TEST) $(TEST_MAIN)

$(MODBUS_TEST) : $(MODBUS_TEST_OBJS)
	$(AR) rcs $(MODBUS_TEST).a $(MODBUS_TEST_OBJS)
	$(CC) -shared -o $(MODBUS_TEST).so $(MODBUS_TEST_OBJS) $(MODBUS_TEST_LDFLAGS)

$(TEST_MAIN) : main.o
	gcc -o $@ main.o -ldl

include $(TOPDIR)/IncludePost.mk
