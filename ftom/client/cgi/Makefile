ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/IncludePre.mk

TARGET=	ftom.cgi
BIN_CFLAGS:=${CFLAGS}\
		-D_GNU_SOURCE

BIN_LDFLAGS:=$(LDFLAGS)

BIN_LIBS:= 	-lftomcl\
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

