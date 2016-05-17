TOPDIR=$(shell pwd)
SUBDIR=ftm ftdm ftom

export TOPDIR

all:
	for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

clean:
	for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

