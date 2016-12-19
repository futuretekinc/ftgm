ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftomcl
BIN_SRCS=main.c\
		ftom_client_cmdline.c\
		cmd_cgi.c\
		cmd_node.c\
		cmd_ep.c\
		cmd_data.c
BIN_OBJS=$(BIN_SRCS:.c=.o)

BIN_LDFLAGS=$(LDFLAGS) -L./ -lftomclient -lftom -lftm -lcjson -lpthread  -lrt -lm

include $(TOPDIR)/IncludePost.mk
