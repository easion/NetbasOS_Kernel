#
# $Id: Makefile 2848 2009-06-22 11:03:33Z luigi $
#
# GNUMakefile to build the userland part of ipfw on Linux
#
# enable extra debugging information
# Do not set with = or := so we can inherit from the caller
$(warning Building userland ipfw for $(VER))
EXTRA_CFLAGS += -g
EXTRA_CFLAGS += -O1 -Wall -Werror
EXTRA_CFLAGS += -include ../glue.h

LDFLAGS=

EXTRA_CFLAGS += -I ./include

ifneq ($(VER),openwrt)
OSARCH := $(shell uname)
ifeq ($(OSARCH),Linux)
    EXTRA_CFLAGS += -D__BSD_VISIBLE
else
    HAVE_NAT := $(shell grep O_NAT /usr/include/netinet/ip_fw.h)
    # EXTRA_CFLAGS += ...
endif
endif # !openwrt

CFLAGS += $(EXTRA_CFLAGS)

OBJS = ipfw2.o dummynet.o main.o ipv6.o altq.o
ifneq ($(HAVE_NAT),)
    OBJS += nat.o
    EXTRA_CFLAGS += -DHAVE_NAT
endif
OBJS += glue.o

all: ipfw
	echo "VER is $(VER)"

ipfw: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJS) : ipfw2.h ../glue.h

clean distclean:
	-rm -f $(OBJS) ipfw
