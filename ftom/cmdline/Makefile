ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
TARGET=ftomcl
TARGET_SRCS=main.c

TARGET_OBJS=$(TARGET_SRCS:.c=.o)
TARGET_LDFLAGS=$(LDFLAGS) -L./ -lftomcl -lftomc -lftom -lftm -lcjson -lpthread  -lrt -lm

TARGET_LIB=libftomcl
TARGET_LIB_SRCS= ftom_client_cmdline.c\
			cmd_cgi.c\
			cmd_node.c\
			cmd_ep.c\
			cmd_data.c

TARGET_LIB_OBJS=$(TARGET_LIB_SRCS:.c=.o)

all: tags $(TARGET_LIB) $(TARGET) 

tags:
	ctags -R
	
$(TARGET): $(TARGET_OBJS) $(TARGET_LIB)
	$(CC) -o $@ $(TARGET_OBJS) $(TARGET_LDFLAGS)

$(TARGET_LIB): $(TARGET_LIB_OBJS)
	$(AR) rcs $(TARGET_LIB).a $(TARGET_LIB_OBJS)
	$(CC) -shared -o $(TARGET_LIB).so.1.0.1 $(TARGET_LIB_OBJS) $(TARGET_LDFLAGS)

.c.o:  $(TARGET_SRC) $(TARGET_LIB_OBJS) *.h
	$(CC) $(CFLAGS) $< -o $@

install: $(TARGET) $(TARGET_LIB)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(TARGET)
	install -d $(LIB_PREFIX)
	install -t $(LIB_PREFIX) $(TARGET_LIB).a
	install -t $(LIB_PREFIX) $(TARGET_LIB).so.1.0.1

clean:
	rm $(TARGET) *.a *.o
