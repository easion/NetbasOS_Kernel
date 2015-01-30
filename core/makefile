ROOTDIR=../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys
#elf_so/elf32.o\

OBJS =  main.o gunzip.o  error.o  daemon.o  global.o lfb.o\
ipc/msg.o ipc/uname.o ipc/ipc.o\
ipc/shm.o ipc/iomux.o\
ipc/mutex.o ipc/cond.o ipc/proc.o\
ipc/signal.o ipc/semaphore.o ipc/msgport.o\
ipc/ptrace.o ipc/pkt.o  ipc/thread_msg.o\
kernel/system.o  kernel/process.o\
kernel/schedule.o  kernel/syscall.o kernel/thread.o \
kernel/wait.o \
kernel/linux.o \
kernel/user_thread.o \
kernel/linux_syscall.o \
elf_so/rldelf.o\
elf_so/rldmisc.o\
mm/alloc.o mm/mem.o mm/vm.o\
fs/rawfs.o fs/server_api.o  fs/fifo.o  fs/kfs.o\
fs/dll_manage.o fs/module.o fs/elf.o\
fs/core_sym.o\
debug/debug.o\
debug/gdb.o\
debug/stacktraverse.o



LIBS=driver/driver.o lib/lib.o
all:	kernel.o 

driver/driver.o:
	$(MAKE) -C ./driver

lib/lib.o:
	$(MAKE) -C ./lib

kernel.o:	$(OBJS) $(LIBS)
	$(LD) -r -o $@ $(OBJS) $(LIBS)


clean:
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./driver clean
	$(RM) *.bak 
	$(RM) fs/*.bak fs/*.o
	$(RM) ipc/*.bak ipc/*.o
	$(RM) kernel/*.bak kernel/*.o
	$(RM) gfx/*.bak gfx/*.o
	$(RM)  $(OBJS)$(LIBS)

