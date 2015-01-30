#
#Makefile for Jicama OS
#

include ./config.inc

include cpu/$(ARCH)_platform.inc

OBJS =  $(CPU_OBJ)
OBJS +=  core/kernel.o 
OBJS +=  net/net.o 


SUBDIRS := core net 

ARCHIVES := $(patsubst %, %/.tmp.o, $(SUBDIRS))


all:    dbg

$(CPU_OBJ):
	$(MAKE) -C $(CPU_DIR)


core/kernel.o:
	$(MAKE) -C ./core

net/net.o:
	$(MAKE) -C ./net


cleanall: 
	$(MAKE) -C ./cpu/i386 clean
	$(MAKE) -C ./cpu/amd64 clean
	$(MAKE) -C ./cpu/arm  clean
	$(MAKE) -C ./net  clean
	$(MAKE) -C ./cpu/xen32 clean
	$(MAKE) -C ./core clean
	$(MAKE) -C ./sys clean
	$(RM) kernel.gz  *.lst *.dbg *.map
	$(RM) *.bak
	$(RM)  $(LIBS)
	$(RM)  $(OBJS)

kernel: $(OBJS)  $(LIBS)
	$(LD) -g $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(STRIP) $@
	$(ACTION)
	cp $@.gz $(SDKDIR)/livecd/iso/boot/ -fr

SYM_ADDR=0

dbg: $(OBJS)  $(LIBS)
	$(LD) -g $(MYLDFLAGS) $(OBJS) $(LIBS) -o kernel.dbg
	$(NM) kernel.dbg | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map
	$(OBJDUMP) --source -d  kernel.dbg > kernel.lst
#	$(OBJDUMP) --source -d --adjust-vma=$(SYM_ADDR) kernel > result.lst
	$(STRIP) kernel.dbg
	cp kernel.dbg $(SDKDIR)/livecd/iso/boot/kernel.gz -fr


clean:
	$(RM)  $(OBJS)
	$(RM) *.gz *.lst *.dbg *.map
	$(RM) kernel
	$(MAKE) -C net clean

	
