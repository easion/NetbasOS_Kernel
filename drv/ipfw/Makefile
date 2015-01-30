# $Id: Makefile 3070 2009-07-24 16:59:59Z marta $
# Top level makefile for building ipfw kernel and userspace.
# You can run it manually or also under the Planetlab build.
# Planetlab wants also the 'install' target.

DATE ?= $(shell date +%Y%m%d)
SNAPSHOT_NAME=ipfw_linux-$(DATE)

_all: all

all clean distclean:
	echo target is $(@)
	(cd ipfw && $(MAKE) $(@) )
	(cd dummynet && $(MAKE) $(@) )

snapshot:
	-ln -s `pwd` /tmp/$(SNAPSHOT_NAME)
	(cd /tmp; tar cvzhf $(SNAPSHOT_NAME).tgz --exclude .svn \
		--exclude README.openwrt --exclude tags --exclude NOTES \
		$(SNAPSHOT_NAME) )
	-rm  /tmp/$(SNAPSHOT_NAME)

install:
