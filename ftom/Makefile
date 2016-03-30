include ../Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
CLIENT=ftm_om_client
CLIENT_SRCS=ftm_om_client_main.c ftm_om_client_config.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftm -lconfig -lpthread 

FTNM=ftom
FTNM_SRCS=ftm_om_main.c ftm_om.c
FTNM_OBJS=$(FTNM_SRCS:.c=.o)
FTNM_LDFLAGS=$(LDFLAGS) -lftom -lftdmc -lftm -lconfig -lpthread  -lnetsnmpmibs -ldl -lnetsnmpagent -lnetsnmp -lmosquitto

LIB_FTNM=libftom
LIB_FTNM_INCS= ftm_om.h 
LIB_FTNM_SRCS= ftm_om.c \
			   ftm_om_msg.c \
			   ftm_om_server.c\
			   ftm_om_node.c\
			   ftm_om_ep.c\
			   ftm_om_ep_class.c\
			   ftm_om_node_snmpc.c\
			   ftm_om_snmpc.c\
			   ftm_om_snmptrapd.c\
			   ftm_om_shell_cmds.c\
			   ftm_om_dmc.c\
			   ftm_om_trigger.c\
			   ftm_om_action.c\
			   ftm_om_rule.c\
			   ftm_mqtt_client.c\
			   ftm_mqtt_client_ft.c\
			   ftm_mqtt_client_tpgw.c\
			   ftm_om_utils.c\
			   ftm_om_service.c

LIB_FTNM_OBJS=$(LIB_FTNM_SRCS:.c=.o)

LIB_FTNMC=libftomc
LIB_FTNMC_INCS= ftm_om_client.h 
LIB_FTNMC_SRCS= ftm_om_client.c 
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
