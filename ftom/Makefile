include ../Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
CLIENT=ftomc
CLIENT_SRCS=ftom_client_main.c ftom_client_config.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftm -lconfig -lpthread 

FTNM=ftom
FTNM_SRCS=ftom_main.c ftom.c
FTNM_OBJS=$(FTNM_SRCS:.c=.o)
FTNM_LDFLAGS=$(LDFLAGS) -lftom -lftdmc -lftm -lconfig -lpthread  -lnetsnmpmibs -ldl -lnetsnmpagent -lnetsnmp -lmosquitto

LIB_FTNM=libftom
LIB_FTNM_INCS= ftom.h 
LIB_FTNM_SRCS= ftom.c \
			   ftom_msg.c \
			   ftom_server.c\
			   ftom_node.c\
			   ftom_node_management.c\
			   ftom_ep.c\
			   ftom_ep_management.c\
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
			   ftom_shell.c

LIB_FTNM_OBJS=$(LIB_FTNM_SRCS:.c=.o)

LIB_FTNMC=libftomc
LIB_FTNMC_INCS= ftom_client.h 
LIB_FTNMC_SRCS= ftom_client.c 
LIB_FTNMC_OBJS=$(LIB_FTNMC_SRCS:.c=.o)

all: tags $(LIB_FTNM) $(FTNM) $(LIB_FTNMC) $(CLIENT)

tags:
	ctags -R
	
$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(FTNM): $(FTNM_OBJS) $(LIB_FTNM)
	$(CC) -g -o $@ $(FTNM_OBJS) $(FTNM_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS) $(LIB_FTNMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTNM): $(LIB_FTNM_OBJS)
	$(AR) rcs $(LIB_FTNM).a $(LIB_FTNM_OBJS)
	$(CC) -shared -o $(LIB_FTNM).so.1.0.1 $(LIB_FTNM_OBJS) $(LDFLAGS)

$(LIB_FTNMC): $(LIB_FTNMC_OBJS)
	$(AR) rcs $(LIB_FTNMC).a $(LIB_FTNMC_OBJS)
	$(CC) -shared -o $(LIB_FTNMC).so.1.0.1 $(LIB_FTNMC_OBJS) $(LDFLAGS)

.c.o:  $(SERVER_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) $(CLIENT) *.a *.o
