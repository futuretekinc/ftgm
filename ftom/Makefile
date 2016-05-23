ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/..
endif
include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

FTOM=ftom
FTOM_SRCS=ftom_main.c ftom.c
FTOM_OBJS=$(FTOM_SRCS:.c=.o)
FTOM_LDFLAGS=$(LDFLAGS) -lftom -lftdmc -lftm -lconfig -lpthread  -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lmodbus -lmosquitto -lrt -ldl

LIB_FTOM=libftom
LIB_FTOM_INCS= ftom.h 
LIB_FTOM_SRCS= ftom.c \
			   ftom_msg.c \
			   ftom_server.c\
			   ftom_node.c\
			   ftom_ep.c\
			   ftom_node_snmpc.c\
			   ftom_snmpc.c\
			   ftom_snmptrapd.c\
			   ftom_shell_cmds.c\
			   ftom_dmc.c\
			   ftom_trigger.c\
			   ftom_action.c\
			   ftom_rule.c\
			   ftom_mqtt_client.c\
			   ftom_mqtt_client_ft.c\
			   ftom_mqtt_client_tpgw.c\
			   ftom_utils.c\
			   ftom_service.c\
			   ftom_shell.c\
			   ftom_discovery.c\
			   ftom_node_modbus_client.c
LIB_FTOM_OBJS=$(LIB_FTOM_SRCS:.c=.o)

LIB_FTOMC=libftomc
LIB_FTOMC_INCS= ftom_client.h 
LIB_FTOMC_SRCS= ftom_client.c \
				ftom_client_net.c
LIB_FTOMC_OBJS=$(LIB_FTOMC_SRCS:.c=.o)

SUBDIR=client cmdline cgi

all: $(LIB_FTOM) $(FTOM) $(LIB_FTOMC) 
	for dir in $(SUBDIR); do\
		make -C $$dir TOPDIR=$(TOPDIR);\
	done
	
$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(FTOM): $(FTOM_OBJS) $(LIB_FTOM)
	$(CC) -g -o $@ $(FTOM_OBJS) $(FTOM_LDFLAGS)

$(LIB_FTOM): $(LIB_FTOM_OBJS)
	$(AR) rcs $(LIB_FTOM).a $(LIB_FTOM_OBJS)
	$(CC) -shared -o $(LIB_FTOM).so.1.0.1 $(LIB_FTOM_OBJS) $(LDFLAGS)

$(LIB_FTOMC): $(LIB_FTOMC_OBJS)
	$(AR) rcs $(LIB_FTOMC).a $(LIB_FTOMC_OBJS)
	$(CC) -shared -o $(LIB_FTOMC).so.1.0.1 $(LIB_FTOMC_OBJS) $(LDFLAGS)

.c.o:  $(CLIENT_SRC) $(LIB_FTOMC_OBJS) *.h $(SERVER_SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) *.a *.o
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done
