ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

TPRESTAPI_LIB=libtprestapi
TPRESTAPI_LIB_SRCS= ftom_tp_restapi.c\
					ftom_tp_restapi_gateway.c

TPRESTAPI_LIB_OBJS=$(TPRESTAPI_LIB_SRCS:.c=.o)
TPRESTAPI_LIB_LDFLAGS=$(LDFLAGS)

CLIENT=tprestapi
CLIENT_SRCS=main.c 

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -L./ -ltprestapi -lftomcl -lftomc -lftm -lconfig -lcurl -lcjson -lpthread  -lm -lrt

all: $(TPRESTAPI_LIB) $(CLIENT)

$(TPRESTAPI_LIB): $(TPRESTAPI_LIB_OBJS) 
	$(AR) rcs $(TPRESTAPI_LIB).a $(TPRESTAPI_LIB_OBJS)
	$(CC) -shared -o $(TPRESTAPI_LIB).so.1.0.1 $(TPRESTAPI_LIB_OBJS) $(TPRESTAPI_LIB_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS) 
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(TPRESTAPI_LIB_SRCS) $(CLIENT_SRCS) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(CLIENT) *.a *.o
