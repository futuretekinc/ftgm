include ../Makefile.in

LIB_FTM=libftm
LIB_FTM_INCS= ftm_types.h ftm_node.h ftm_endpoint.h ftm_object.h ftm_debug.h ftm_list.h ftm_console.h 
LIB_FTM_SRCS= ftm_debug.c convert.c ftm_node.c ftm_endpoint.c ftm_list.c ftm_console.c ftm_mem.c ftm_value.c ftm_timer.c
LIB_FTM_OBJS=$(LIB_FTM_SRCS:.c=.o)

all: $(LIB_FTM)

$(LIB_FTM): $(LIB_FTM_OBJS)
	$(AR) rcs $(LIB_FTM).a $(LIB_FTM_OBJS)
	$(CC) -shared -o $(LIB_FTM).so.1.0.1 $(LIB_FTM_OBJS) $(LDFLAGS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(LIB_FTM).so.1.0.1 *.a *.o
