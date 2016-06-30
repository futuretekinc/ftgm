ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/..
endif
include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

FTOM=ftom
FTOM_SRCS=ftom_main.c \
		ftom.c\
		ftom_server.c\
		ftom_service.c\
		ftom_snmpc.c\
		ftom_snmptrapd.c\
		ftom_node_snmp_client.c\
		ftom_node_snmp_client_gen.c\
		ftom_node_fins_client.c\
		ftom_node_fins_client_hhtw.c\
		ftom_node_virtual.c\
		ftom_node_virtual_fte_es7.c\
		ftom_discovery.c

FTOM_OBJS=$(FTOM_SRCS:.c=.o)
FTOM_LDFLAGS=$(LDFLAGS) -lftom -lftdmc -lftm -lcjson -lpthread  -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lmosquitto -lrt -ldl -lm

LIB_FTOM=libftom
LIB_FTOM_INCS= ftom.h 
LIB_FTOM_SRCS= ftom_msg.c \
			   ftom_message_queue.c \
			   ftom_node.c\
			   ftom_node_class.c\
			   ftom_ep.c\
			   ftom_shell_cmds.c\
			   ftom_dmc.c\
			   ftom_trigger.c\
			   ftom_action.c\
			   ftom_rule.c\
			   ftom_mqtt_client.c\
			   ftom_mqtt_client_ft.c\
			   ftom_utils.c\
			   ftom_logger.c\
			   ftom_session.c\
			   ftom_shell.c
LIB_FTOM_OBJS=$(LIB_FTOM_SRCS:.c=.o)

LIB_FTOMC=libftomc
LIB_FTOMC_INCS= ftom_client.h 
LIB_FTOMC_SRCS= ftom_client.c \
				ftom_client_net.c
LIB_FTOMC_OBJS=$(LIB_FTOMC_SRCS:.c=.o)

LIB_HHTW=libhhtw
LIB_HHTW_SRCS= ftom_node_fins_client_hhtw.c
LIB_HHTW_OBJS=$(LIB_HHTW_SRCS:.c=.o)

SUBDIR=client cmdline cgi

all: $(LIB_FTOM) $(FTOM) $(LIB_FTOMC) $(LIB_HHTW)
	for dir in $(SUBDIR); do\
		make -C $$dir TOPDIR=$(TOPDIR);\
	done
	
$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(FTOM): $(FTOM_OBJS) $(LIB_FTOM)
	$(CC) -g -o $@ $(FTOM_OBJS) $(FTOM_LDFLAGS)

$(LIB_FTOM): $(LIB_FTOM_OBJS)
	rm -f $(LIB_FTOM).a $(LIB_FTOM).so.1.0.1
	$(AR) rcs $(LIB_FTOM).a $(LIB_FTOM_OBJS)
	$(CC) -shared -o $(LIB_FTOM).so.1.0.1 $(LIB_FTOM_OBJS) $(LDFLAGS)

$(LIB_FTOMC): $(LIB_FTOMC_OBJS)
	rm -f $(LIB_FTOMC).a $(LIB_FTOMC).so.1.0.1
	$(AR) rcs $(LIB_FTOMC).a $(LIB_FTOMC_OBJS)
	$(CC) -shared -o $(LIB_FTOMC).so.1.0.1 $(LIB_FTOMC_OBJS) $(LDFLAGS)

$(LIB_HHTW): $(LIB_HHTW_OBJS)
	rm -f $(LIB_HHTW).a $(LIB_HHTW).so
	$(AR) rcs $(LIB_HHTW).a $(LIB_HHTW_OBJS)
	$(CC) -shared -o $(LIB_HHTW).so $(LIB_HHTW_OBJS) $(LDFLAGS)

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

install: $(FTOM) $(LIB_FTOM) $(LIB_FTOMC)
	install -d $(EXEC_PREFIX)	
	install -t $(EXEC_PREFIX) $(FTOM)
	install -d $(LIB_PREFIX)
	install -t $(LIB_PREFIX) $(LIB_FTOM).a
	install -t $(LIB_PREFIX) $(LIB_FTOM).so.1.0.1
	install -t $(LIB_PREFIX) $(LIB_FTOMC).a
	install -t $(LIB_PREFIX) $(LIB_FTOMC).so.1.0.1
	install -d $(CONF_PREFIX)
	install -t $(CONF_PREFIX) $(FTOM).conf
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

clean:
	rm $(SERVER) *.a *.o
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done
