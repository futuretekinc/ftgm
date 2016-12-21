ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftomcl
BIN_SRCS=main.c
BIN_OBJS=$(BIN_SRCS:.c=.o)

BIN_LDFLAGS=$(LDFLAGS) -L./ -lftomcl -lftomclient -lftom -lftm -lcjson -lpthread  -lrt -lm

LIB_TARGET=libftomcl.a
LIB_SRCS= ftom_client_cmdline.c\
		cmd_cgi.c\
		cmd_node.c\
		cmd_ep.c\
		cmd_data.c
LIB_OBJS=$(LIB_SRCS:.c=.o)

LIB_LDFLAGS=$(LDFLAGS) 

include $(TOPDIR)/IncludePost.mk
