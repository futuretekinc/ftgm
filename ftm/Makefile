include ../Makefile.in

LIB_FTM=libftm
LIB_FTM_INCS= ftm_types.h ftm_node.h ftm_ep.h ftm_object.h ftm_debug.h ftm_list.h ftm_shell.h 
LIB_FTM_SRCS= ftm_trace.c \
			  ftm_log.c \
			  convert.c \
			  ftm_node.c \
			  ftm_ep.c \
			  ftm_list.c \
			  ftm_queue.c \
			  ftm_msg_queue.c \
			  ftm_shell.c \
			  ftm_mem.c \
			  ftm_value.c \
			  ftm_timer.c \
			  nxjson.c \
			  ftm_config.c \
			  ftm_time.c \
			  ftm_trigger.c \
			  ftm_action.c \
			  ftm_rule.c \
			  ftm_lock.c \
			  ftm_snmp.c

LIB_FTM_OBJS=$(LIB_FTM_SRCS:.c=.o)

all: $(LIB_FTM)

$(LIB_FTM): $(LIB_FTM_OBJS)
	$(AR) rcs $(LIB_FTM).a $(LIB_FTM_OBJS)
	$(CC) -shared -o $(LIB_FTM).so.1.0.1 $(LIB_FTM_OBJS) $(LDFLAGS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(LIB_FTM).so.1.0.1 *.a *.o
