ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/IncludePre.mk

CLIENT=ftm_test
CLIENT_SRCS=main.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS)

LIBS= -lftm -lconfig -lpthread
all: $(LIB_FTDMC) $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) 
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS) $(LIBS)

include $(TOPDIR)/IncludePost.mk
