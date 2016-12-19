ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/IncludePre.mk

LIB_TARGET=libftomnode.a
LIB_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

include $(TOPDIR)/IncludePost.mk
