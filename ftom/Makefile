ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

BIN_TARGET=ftom
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

BIN_LDFLAGS=$(LDFLAGS) -lftomnode -lftom -lftdm -lftm -lcjson -lpthread  -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lmosquitto -lrt -ldl -lm

SUBDIR=lib client node

include $(TOPDIR)/IncludePost.mk
