TOPDIR=$(shell pwd)
SUBDIR=common ftdm ftdmconsole ftom blocker ftomconsole ftomcl

export TOPDIR

all: subdirs

subdirs:
	@for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

install:
	@for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

dev_install:
	@for dir in $(SUBDIR); do\
		make -C $$dir dev_install;\
	done

clean:
	@for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

