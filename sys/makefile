ROOTDIR=../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc
include $(ROOTDIR)cpu/cpu_flags.inc

CFLAGS += -I$(ROOTDIR)sys

clean:
	$(RM) jicama/*.bak
	$(RM) arch/arm/*.bak
	$(RM) arch/x86/*.bak
	$(RM) arch/amd64/*.bak
	$(RM) *.bak
