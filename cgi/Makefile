ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftom.cgi
BIN_CFLAGS:=${CFLAGS}\
		-D_GNU_SOURCE

BIN_LDFLAGS:=$(LDFLAGS)\
		-lftomcl\
		-lftomclient\
		-lftom\
		-lftm\
		-lpthread \
		-lcjson \
		-lmxml\
		-lqdecoder\
		-lm\
		-lrt
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

include $(TOPDIR)/IncludePost.mk

