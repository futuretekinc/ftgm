include $(TOPDIR)/Makefile.in

CFLAGS:=${CFLAGS}\
 		-std=gnu99\
		-Wall\
		-fPIC\
		-g \
		-D_GNU_SOURCE
LDFLAGS:=$(LDFLAGS)
LIBS:= 	-lftomcl\
		-lftomc\
		-lftm\
		-lconfig\
		-lpthread \
		-lm\
		-lrt

TARGET=	ftom.cgi
SRCS=	main.o\
		ftom_cgi.o\
		cmd_ep.o\
		cmd_node.o\
		cmd_trigger.o\
		cmd_action.o\
		cmd_rule.o\
		cmd_discovery.o

ifeq ($(MODEL),"FTM-50S")
LIBS:=	$(LIBS)\
		-lcjson \
		-lmxml\
		-lqdecoder
else ifeq ($(MODEL),"FTM-100S")
LIBS:=	$(LIBS)\
		-lcjson \
		-lmxml\
		-lqdecoder
else
LIBS:=	$(LIBS)\
		$(TOPDIR)/cjson/libcjson.a\
		$(TOPDIR)/mxml-2.9/libmxml.a\
		$(TOPDIR)/qdecoder-12.0.4/src/libqdecoder.a
endif

OBJS=$(SRCS:.c=.o)

all: $(TARGET) xmltest

$(TARGET): $(OBJS) 
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

xmltest : xmltest.o
	$(CC) $(LDFLAGS) -o $@ xmltest.o $(LIBS)

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET) *.a *.o sxmlc/src/*.o
