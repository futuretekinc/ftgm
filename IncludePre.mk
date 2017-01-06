#MODEL=ftm-50s
#MODEL=ftm-100s
MODEL=i686

TARGET=$(TOPDIR)/target/$(MODEL)
EXEC_PREFIX=$(TARGET)/usr/bin
INC_PREFIX=$(TARGET)/usr/include
LIB_PREFIX=$(TARGET)/usr/lib
CONF_PREFIX=$(TARGET)/etc
CGI_PREFIX=$(TARGET)/www/cgi-bin
export MODEL TOPDIR

CFLAGS:= -std=gnu99 -fPIC -c -g -Wall -DDEBUG \
				-I. \
				-I/usr/include/azureiot\
				-I/usr/include/azureiot/inc\
				-I$(TOPDIR)/common\
				-I$(TOPDIR)/ftdm \
				-I$(TOPDIR)/ftdm/lib \
				-I$(TOPDIR)/ftom \
				-I$(TOPDIR)/ftom/lib \
				-I$(TOPDIR)/ftomcl\
				-I$(TOPDIR)/ftdm/client \
				-I$(TOPDIR)/ftom/client \
				-I$(TOPDIR)/ftom/client/ftom \
				-I$(TOPDIR)/ftom/client/ftom/net \
				-I$(TOPDIR)/ftom/client/mqtt \
				-I$(TOPDIR)/ftom/node \
				-I$(TOPDIR)/blocker/cloud\
				-I$(TOPDIR)/blocker/cloud/azure \
				-I$(TOPDIR)/blocker/cloud/thingplus \

LDFLAGS:= -Wl,--no-as-needed\
				-L.\
				-L$(TOPDIR)/common\
				-L$(TOPDIR)/ftdm \
				-L$(TOPDIR)/ftdm/lib \
				-L$(TOPDIR)/ftom \
				-L$(TOPDIR)/ftom/lib \
				-L$(TOPDIR)/ftomcl \
				-L$(TOPDIR)/ftdm/client \
				-L$(TOPDIR)/ftom/client \
				-L$(TOPDIR)/ftom/client/ftom\
				-L$(TOPDIR)/ftom/client/ftom/net\
				-L$(TOPDIR)/ftom/client/mqtt\
				-L$(TOPDIR)/blocker/cloud\
				-L$(TOPDIR)/blocker/cloud/azure \
				-L$(TOPDIR)/blocker/cloud/thingplus \
				-L$(TOPDIR)/ftom/node 

ifeq ("$(MODEL)","ftm-50s")
CC=armv5-linux-gcc
AR=armv5-linux-ar
CFLAGS:=-fPIC \
		$(CFLAGS)\
		-I$(TOPDIR)/devtools/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/local/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/include/cjson
LDFLAGS:=$(LDFLAGS)\
		-L$(TOPDIR)/devtools/lib\
		-L/home/xtra/ftm/build/ftm-50s/_root/lib \
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/lib \
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/local/lib 
else ifeq ("$(MODEL)","ftm-100s")
CC=arm-openwrt-linux-uclibcgnueabi-gcc
AR=arm-openwrt-linux-uclibcgnueabi-ar
CFLAGS:=-fPIC \
		$(CFLAGS) \
		-I$(TOPDIR)/devtools/include \
		-I/home/xtra/ftm/build/ftm-100s/_root/include\
		-I/home/xtra/ftm/build/ftm-100s/_root/usr/include\
		-I/home/xtra/ftm/build/ftm-100s/_root/usr/local/include\
		-I/home/xtra/ftm/build/ftm-100s/_root/usr/include/cjson

LDFLAGS:= $(LDFLAGS)\
		-L/home/xtra/ftm/build/ftm-100s/_root/lib \
		-L/home/xtra/ftm/build/ftm-100s/_root/usr/lib \
		-L/home/xtra/ftm/build/ftm-100s/_root/usr/local/lib \

else
CC=gcc
CFLAGS:= -fPIC \
	$(CFLAGS) \
		-I/usr/local/include \
		-I/usr/local/include/cjson 

LDFLAGS:= $(LDFLAGS)\
		-L/usr/local/lib 

endif

BIN_CFLAGS=$(CFLAGS)
BIN_LDFLAGS=$(LDFLAGS)
LIB_CFLAGS=$(CFLAGS)
LIB_LDFLAGS=$(LDFLAGS)
