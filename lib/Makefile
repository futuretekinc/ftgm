#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include 
LDFLAGS= -L/usr/local/lib -L. 

LIB_FTM=libftm
LIB_FTM_INCS= ftm_types.h ftm_object.h simclist.h ftm_debug.h ftm_list.h ftm_console.h 
LIB_FTM_SRCS= simclist.c  ftm_debug.c convert.c ftm_object.c ftm_list.c ftm_console.c ftm_mem.c ftm_value.c
LIB_FTM_OBJS=$(LIB_FTM_SRCS:.c=.o)

all: $(LIB_FTM) $(SERVER)

$(LIB_FTM): $(LIB_FTM_OBJS)
	$(AR) rcs $(LIB_FTM).a $(LIB_FTM_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTM).so.1.0.1 $(LIB_FTM_OBJS) $(LDFLAGS)

.c.o:  $(SERVER_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) $(CLIENT) *.a *.o
