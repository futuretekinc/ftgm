ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/IncludePre.mk

LIB_TARGET=libftdm.a
LIB_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

include $(TOPDIR)/IncludePost.mk
