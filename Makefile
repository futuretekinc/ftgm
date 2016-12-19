TOPDIR=$(shell pwd)
SUBDIR=common ftdm ftom

export TOPDIR

all: subdirs

subdirs:
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

