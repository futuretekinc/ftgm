TOPDIR=..
include ${TOPDIR}/Makefile.in

SERVER=ftdm
SERVER_INCS=ftdm.h ftdm_server.h ftdm_server_cmds.h
SERVER_SRCS=ftdm.c ftdm_server.c ftdm_server_cmds.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_LDFLAGS=$(LDFLAGS) -lftdm -lftm -lconfig -lpthread -lsqlite3 -ldl 

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
				ftdm_rule.c 
LIB_FTDM_OBJS=$(LIB_FTDM_SRCS:.c=.o)

all: $(LIB_FTDM) $(SERVER)

$(SERVER): $(SERVER_OBJS) $(LIB_FTDM)
	$(CC) -o $@ $(SERVER_OBJS) $(SERVER_LDFLAGS)

$(LIB_FTDM): $(LIB_FTDM_OBJS)
	$(AR) rcs $(LIB_FTDM).a $(LIB_FTDM_OBJS)
	$(CC) -shared -o $(LIB_FTDM).so.1.0.1 $(LIB_FTDM_OBJS) 

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(SERVER) *.a *.o $(LIB_FTDM)
