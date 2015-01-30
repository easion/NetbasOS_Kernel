
include $(SDKDIR)/bin/compile.mk

CFLAGS += -g -I../include 
LDFLAGS += -g 
BINDIR = .
PROGS =  pipetest1 pipetest2 pipetest3 pipetest4 pipe popen minicron


all: 	$(PROGS)



install:	$(PROGS)
	cp $(PROGS) $(BINDIR)
	cd $(BINDIR); strip $(PROGS); chmod 755 $(PROGS)

clean:
	rm -f $(PROGS) *.o core *.a *.bak

minicron:	minicron.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/


pipetest1:	pipetest1.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/


pipetest2:	pipetest2.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/

pipetest3:	pipetest3.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/

pipetest4:	pipetest4.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/

pipe:	pipe.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/

popen:	popen.o
	$(LD)  $(LCFLAG) -o $@ $(CRT1) $^  -L$(UCLIBBINDIR)  $(LINKLIB) 
	$(STRIP)  $@ 
	cp -fr  $@  $(INSTALL_DIR)/system/bin/





