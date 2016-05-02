include ${TOPDIR}/Makefile.in

APP=openssl
PROGRAM=${APP}
VERSION=-1.0.2d
PACKAGE=${PROGRAM}${VERSION}

CONFIG_OPTIONS=--prefix=${PREFIX} shared linux-cortina
BUILD_OPTIONS=AR="${AR} r"
INSTALL_OPTIONS=INSTALL_PREFIX=${DESTDIR}


all: build

init:
	$(call init_package,${PACKAGE})

reinit:
	$(call reinit_package,${PACKAGE})

config: init
	$(call config_package,${PACKAGE},${CONFIG_OPTIONS})

reconfig:
	$(call reconfig_package,${PACKAGE},${CONFIG_OPTIONS})

build: config
	$(call build_package,${PACKAGE})

install:
	$(call install_package,${PACKAGE})

clean:
	$(call clean_package,${PACKAGE})

distclean: clean
	$(call distclean_package,${PACKAGE})
