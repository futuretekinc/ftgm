TOPDIR=${CURDIR}

export TOPDIR

include Makefile.in

phase1-y= 
phase1-${CONFIG_GMP} += gmp
phase1-${CONFIG_PCRE} += pcre 
phase1-${CONFIG_LIBUBOX} += libubox 
phase1-${CONFIG_ZLIB} += zlib 
phase1-${CONFIG_QDECODER} += qdecoder 
phase1-${CONFIG_LIBPCAP} += libpcap
phase1-${CONFIG_OPENSSL} += openssl 

phase2-y= utils
phase2-${CONFIG_BASE} += base 
phase2-${CONFIG_BUSYBOX} += busybox 

# Network Applications
phase2-${CONFIG_NETWORK} += network 
phase2-${CONFIG_IPTABLES} += iptables 
phase2-${CONFIG_NET_SNMP} += net-snmp
phase2-${CONFIG_OPENSSH} += openssh 
phase2-${CONFIG_DROPBEAR} += dropbear 
phase2-${CONFIG_STRONGSWAN} += strongswan 
phase2-${CONFIG_HOTPLUG2} += hotplug2 
phase2-${CONFIG_NTPCLIENT} += ntpclient 
phase2-${CONFIG_LIGHTTPD} += lighttpd 
phase2-${CONFIG_UDHCPD} += udhcpd
phase2-${CONFIG_WEBADMIN} += webadmin 
phase2-${CONFIG_MOSQUITTO} += mosquitto 
phase2-${CONFIG_BIND} += bind

# Wireless applications
phase2-${CONFIG_WIRELESS_TOOLS} += wireless_tools 
phase2-${CONFIG_WIFI} += wifi 
phase2-${CONFIG_AP} += ap

# Configuration Utilities
phase2-${CONFIG_LUA} += lua 
phase2-${CONFIG_UCI} += uci 

# Debugging Utilities
phase2-${CONFIG_TCPDUMP} += tcpdump 

LIBS=${phase1-y}
APPS=${phase2-y}

all: install_phase2
	
phase1: build_phase1

config_phase1: 
	[ -d ${BUILDDIR} ] || mkdir -p ${BUILDDIR}; 
	( \
		cd ${BUILDDIR}; \
		for lib in $(LIBS); do \
			[ -d $$lib ] || tar xvfz ${PKGDIR}/$$lib-pkg.tar.gz ;\
			make -C $$lib config DESTDIR=${DESTDIR} ; \
		done ; \
	) 

config_phase2: install_phase1
	[ -d ${BUILDDIR} ] || mkdir -p ${BUILDDIR}; 
	( \
		cd ${BUILDDIR}; \
		for app in $(APPS); do \
			[ -d $$app ] || tar xvfz ${PKGDIR}/$$app-pkg.tar.gz ; \
			make -C $$app config DESTDIR=${DESTDIR} ; \
		done; \
	)

build_phase1:  config_phase1
	( \
		cd ${BUILDDIR}; \
		for app in $(LIBS); do \
			make -C $$app build ; \
		done; \
	)

build_phase2:  config_phase2
	( \
		cd ${BUILDDIR}; \
		for app in $(APPS); do \
			make -C $$app build ; \
		done; \
	)

build: build_phase2

install_phase1: build_phase1
	if [ -d ${DESTDIR} ]; then \
		rm -rf ${DESTDIR}\* ;\
	else \
		mkdir -p ${DESTDIR} ;\
	fi
	( \
		cd ${BUILDDIR}; \
		for app in $(LIBS); do \
			make -C $$app install DESTDIR=${DESTDIR} ; \
		done; \
	)

install_phase2: build_phase2
	if [ -d ${DESTDIR} ]; then \
		rm -rf ${DESTDIR}\* ;\
	else \
		mkdir -p ${DESTDIR} ;\
	fi
	( \
		cd ${BUILDDIR}; \
		for app in $(APPS); do \
			make -C $$app install DESTDIR=${DESTDIR} ; \
		done; \
	)
	${TOPDIR}/tools/make_image ${DESTDIR} ${BUILDDIR}/rootfs.img

#	if [ -d ${DESTDIR} ]; then \
		rm -rf ${DESTDIR} ;\
	fi

install: install_phase2

clean:
	( \
		cd ${BUILDDIR}; \
		for app in $(LIBS) $(APPS); do \
			make -C $$app clean ; \
		done; \
	)

distclean:
	rm -rf ${BUILDDIR}

