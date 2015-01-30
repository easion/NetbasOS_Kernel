

$(warning including dummynet/Makefile)

# lets default for 2.6 for planetlab builds
VER ?= 2.6

# General values
obj-m := ipfw_mod.o

# generic cflags used on all systems
ipfw-cflags += -DIPFIREWALL_DEFAULT_TO_ACCEPT
# _BSD_SOURCE enables __FAVOR_BSD (udp/tcp bsd structs instead of posix)
ipfw-cflags += -D_BSD_SOURCE
ipfw-cflags += -DKERNEL_MODULE	# build linux kernel module
# the two header trees for empty and override files
ipfw-cflags += -I $(M)/include_e -I $(M)/include
ipfw-cflags += -include $(M)/../glue.h	# headers

$(warning "---- Building dummynet kernel module for Version $(VER)")
# We have three sections for OpenWrt, Linux 2.4 and Linux 2.6
#
ifeq ($(VER),openwrt)
  M=.
  obj-y := ipfw2_mod.o bsd_compat.o \
	in_cksum.o ip_dummynet.o ip_fw2.o ip_fw_pfil.o
  O_TARGET := ipfw_mod.o

  # xcflags-y is a temporary variable where we store build options
  xcflags-y += -O1 -DLINUX_24
  xcflags-y += -g

  EXTRA_CFLAGS := -O1 -g -DLINUX_24 $(ipfw-cflags)

  # we should not export anything
  #export-objs := ipfw2_mod.o
-include $(TOPDIR)/Rules.make

else	# !openwrt, below we do linux builds for 2.4 and 2.6

  # KERNELPATH is where the kernel headers reside. On PlanetLab
  # it is set already by the build system.
  # We can override it from the command line, or let the system guess.

ifneq ($(shell echo $(VER)|grep '2.4'),)
  # The linux 2.4 version
  # guess the kernel path -- or is it under /lib/modules ?
  KERNELPATH ?= /usr/src/`uname -r`/build

  # Guess the gcc include directory
  # The gcc version is in the last line returned by gcc -v
  # gcc version 4.3.2 (Debian 4.3.2-1.1)
  MYGCC_VER ?= $(shell gcc -v 2>&1 |tail -n 1 | cut -d " " -f 3)
  # We don't know the exact directory unde /usr/lib/gcc so we guess
  MYGCC_INCLUDE ?= $(shell echo /usr/lib/gcc/*/$(MYGCC_VER) | cut -d " " -f 1)/include
  $(warning "---- gcc includes guessed to $(MYGCC_INCLUDE)")

  # additional warning
  #WARN = -Wp,-MD,/home/luigi/ports-luigi/dummynet-branches/ipfw_mod/dummynet/.ipfw2_mod.o.d
  #WARN += -Iinclude  -include include/linux/autoconf.h

  WARN += -Wall -Wundef
  WARN += -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing
  WARN += -fno-common -Werror-implicit-function-declaration
  # WARN += -O2  -fno-stack-protector -m32 -msoft-float -mregparm=3
  # -mregparm=3 gives a printk error
  WARN += -m32 -msoft-float # -mregparm=3
  #WARN += -freg-struct-return -mpreferred-stack-boundary=2
  WARN += -Wno-sign-compare
  WARN += -Wdeclaration-after-statement -Wno-pointer-sign 

  ccflags-y += -O1 -DLINUX_24
  CFLAGS = -DMODULE -D__KERNEL__ -nostdinc \
	-isystem ${KERNELPATH}/include -isystem $(MYGCC_INCLUDE) ${ccflags-y}
  # The Main target
all: mod24

else
  # if not set, use the version from the installed system
  KERNELPATH ?= /lib/modules/`uname -r`/build
  $(warning "---- Building Version 2.6 $(VER) in $(KERNELPATH)")
  WARN := -O1 -Wall -Werror -DDEBUG_SPINLOCK -DDEBUG_MUTEXES
  # The main target

  # Required by kernel <= 2.6.22, ccflags-y is used on newer version
  LINUX_VERSION_CODE := $(shell grep LINUX_VERSION_CODE $(KERNELPATH)/include/linux/version.h|cut -d " " -f3)
  ifeq ($(LINUX_VERSION_CODE),132630)
    EXTRA_CFLAGS += $(ccflags-y)
  endif

all: include_e
	$(MAKE) -C $(KERNELPATH) V=1 M=`pwd` modules
endif

#-- back to the common section of code

# the list of objects used to build the module
ipfw_mod-y = $(IPFW_SRCS:%.c=%.o)

# Original ipfw and dummynet sources + FreeBSD stuff,
IPFW_SRCS = ip_fw2.c ip_dummynet.c ip_fw_pfil.c in_cksum.c

# Module glue and functions missing in linux
IPFW_SRCS += ipfw2_mod.c bsd_compat.c

# additional $(CC) flags
ccflags-y += $(WARN)
ccflags-y += $(ipfw-cflags)
ccflags-y += -g

mod24: include_e $(obj-m)

$(obj-m): $(ipfw_mod-y)
	$(LD) $(LDFLAGS) -m elf_i386 -r -o $@ $^
clean:
	-rm -f *.o *.ko Module.symvers *.mod.c

distclean: clean
	-rm -f .*cmd modules.order opt_*
	-rm -rf .tmp_versions include_e

# support to create empty dirs and files in include_e/
# EDIRS is the list of directories, EFILES is the list of files.

EDIRS= altq arpa machine net netinet netinet6 sys

EFILES += opt_inet6.h opt_ipfw.h opt_ipsec.h opt_mac.h
EFILES += opt_mbuf_stress_test.h opt_param.h

EFILES += altq/if_altq.h
EFILES += arpa/inet.h
EFILES += machine/in_cksum.h
EFILES += net/ethernet.h net/netisr.h net/pf_mtag.h net/radix.h

EFILES += netinet/ether.h netinet/icmp6.h netinet/if_ether.h
EFILES += netinet/in.h netinet/in_pcb.h netinet/in_var.h
EFILES +=  netinet/ip_carp.h netinet/ip_var.h netinet/pim.h
EFILES += netinet/sctp.h netinet/tcp_timer.h netinet/tcpip.h
EFILES += netinet/udp_var.h

EFILES += netinet6/ip6_var.h

EFILES += sys/_lock.h sys/_mutex.h sys/jail.h
EFILES += sys/limits.h sys/lock.h sys/mutex.h sys/priv.h
EFILES += sys/proc.h sys/rwlock.h sys/socket.h sys/socketvar.h
EFILES += sys/sysctl.h sys/time.h sys/ucred.h

M ?= $(shell pwd)
include_e:
	echo "running in $M"
	-@rm -rf $(M)/include_e opt_*
	-@mkdir -p $(M)/include_e
	-@(cd $(M)/include_e; mkdir -p $(EDIRS); touch $(EFILES) )

endif # !openwrt
