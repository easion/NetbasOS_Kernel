
include $(SDKDIR)/bin/compile.mk
CFLAGS += -O1  -I ./include -D__BSD_VISIBLE -DHAVE_NAT  -I../../include -include ../glue.h


#LDFLAGS += -g -L -lgi
PROGS = ipfw




all: 	$(PROGS)



install:	$(PROGS)
	cp $(PROGS) $(BINDIR)
	cd $(BINDIR); strip $(PROGS); chmod 755 $(PROGS)

clean:
	rm -fr *.o *.bak $(PROGS)

# 
OBJS= ipfw2.o main.o ipv6.o nat.o altq.o dummynet.o glue.o



ipfw: $(OBJS)
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)   $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/
#	cp -fr  add_rules  $(INSTALL_DIR)/system/bin/


giwm_static: $(OBJS) 
	$(LD)    $(LCFLAG_DY)    -o $@ $(CRT0)         $(OBJS) $(SO_DIR)libgi.so  -lpng -lz -lfreetype     $(LINKLIB) 
	@$(OBJDUMP) -afx $@ > $@.lst
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/
