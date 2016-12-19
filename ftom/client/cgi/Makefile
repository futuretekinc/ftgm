ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif
include $(TOPDIR)/Makefile.in

CFLAGS:=${CFLAGS}\
		-D_GNU_SOURCE

LDFLAGS:=$(LDFLAGS)
LIBS:= 	-lftomcl\
		-lftomclient\
		-lftom\
		-lftm\
		-lpthread \
		-lcjson \
		-lmxml\
		-lqdecoder\
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
		cmd_log.o\
		cmd_discovery.o

OBJS=$(SRCS:.c=.o)

all: $(TARGET) xmltest

$(TARGET): $(OBJS) 
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

xmltest : xmltest.o
	$(CC) $(LDFLAGS) -o $@ xmltest.o $(LIBS)

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

install: 	$(TARGET)
	install -d $(CGI_PREFIX)
	install -t $(CGI_PREFIX) ftom.cgi
	ln -s -f ftom.cgi $(CGI_PREFIX)/node
	ln -s -f ftom.cgi $(CGI_PREFIX)/ep
	ln -s -f ftom.cgi $(CGI_PREFIX)/data
	ln -s -f ftom.cgi $(CGI_PREFIX)/trigger
	ln -s -f ftom.cgi $(CGI_PREFIX)/action
	ln -s -f ftom.cgi $(CGI_PREFIX)/rule
	ln -s -f ftom.cgi $(CGI_PREFIX)/discovery

clean:
	rm -f $(CGI) *.a *.o sxmlc/src/*.o
