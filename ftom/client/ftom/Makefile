ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/IncludePre.mk


LIB_TARGET=libftomclient.a

LIB_SRCS= $(foreach dir, . net, $(wildcard $(dir)/*.c))
LIB_OBJS=$(LIB_SRCS:.c=.o)

LIB_CFLAGS:=$(CFLAGS) -I$(TOPDIR)/libftm

include $(TOPDIR)/IncludePost.mk

