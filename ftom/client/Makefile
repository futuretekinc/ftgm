ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/Makefile.in

export TOPDIR

SUBDIR=ftom mqtt cloud cmdline cgi console

all: 
	for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

install:
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

clean:
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

