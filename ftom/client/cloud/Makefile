ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/IncludePre.mk

LIB_TARGET=libcloudclient.a
LIB_SRCS= $(foreach dir, . azure thingplus, $(wildcard $(dir)/*.c))
LIB_OBJS=$(LIB_SRCS:.c=.o)

include $(TOPDIR)/IncludePost.mk
