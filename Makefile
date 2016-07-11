TOPDIR=$(shell pwd)
SUBDIR=libftm ftdm ftom

export TOPDIR

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

