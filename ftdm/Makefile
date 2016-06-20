ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/..
endif

include $(TOPDIR)/Makefile.in

SERVER=ftdm
SERVER_INCS=ftdm.h ftdm_server.h ftdm_server_cmds.h
SERVER_SRCS=ftdm.c ftdm_server.c ftdm_server_cmds.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_LDFLAGS=$(LDFLAGS) -lftdm -lftm -lconfig -lcjson -lpthread -lsqlite3 -ldl -lrt -lm

SUBDIR=client

LIB_FTDM=libftdm
LIB_FTDM_INCS= 	ftdm_config.h\
				ftdm_server.h\
				ftdm_sqlite.h\
				ftdm_node.h\
				ftdm_node_management.h\
				ftdm_ep.h\
				ftdm_ep_class.h 
LIB_FTDM_SRCS= 	ftdm_config.c\
				ftdm_server.c\
				ftdm_sqlite.c\
				ftdm_node.c\
				ftdm_node_management.c\
				ftdm_ep.c\
				ftdm_ep_management.c\
				ftdm_ep_class.c\
				ftdm_shell_cmds.c\
				ftdm_trigger.c\
				ftdm_action.c\
				ftdm_rule.c \
				ftdm_logger.c\
				ftdm_log.c
LIB_FTDM_OBJS=$(LIB_FTDM_SRCS:.c=.o)

LIB_FTDMC=libftdmc
LIB_FTDMC_INCS=ftdm_client.h
LIB_FTDMC_SRCS=ftdm_client.c 
LIB_FTDMC_OBJS=$(LIB_FTDMC_SRCS:.c=.o)


all: $(LIB_FTDMC) $(LIB_FTDM) $(SERVER)
	for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

$(SERVER): $(SERVER_OBJS) $(LIB_FTDM)
	$(CC) -o $@ $(SERVER_OBJS) $(SERVER_LDFLAGS)

$(LIB_FTDM): $(LIB_FTDM_OBJS)
	$(AR) rcs $(LIB_FTDM).a $(LIB_FTDM_OBJS)
	$(CC) -shared -o $(LIB_FTDM).so.1.0.1 $(LIB_FTDM_OBJS) 

$(LIB_FTDMC): $(LIB_FTDMC_OBJS)
	$(AR) rcs $(LIB_FTDMC).a $(LIB_FTDMC_OBJS)
	$(CC) -shared -o $(LIB_FTDMC).so.1.0.1 $(LIB_FTDMC_OBJS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

install: $(SERVER) $(LIB_FTDM) $(LIB_FTDMC)
	install -d $(EXEC_PREFIX)	
	install -t $(EXEC_PREFIX) $(SERVER)
	install -d $(LIB_PREFIX)
	install -t $(LIB_PREFIX)	$(LIB_FTDM).a
	install -t $(LIB_PREFIX)	$(LIB_FTDM).so.1.0.1
	install -t $(LIB_PREFIX)	$(LIB_FTDMC).a
	install -t $(LIB_PREFIX)	$(LIB_FTDMC).so.1.0.1
	install -d $(CONF_PREFIX)
	install -t $(CONF_PREFIX) $(SERVER).conf
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

clean:
	rm -f $(SERVER) *.a *.o $(LIB_FTDM) $(LIB_FTDMC)
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done
