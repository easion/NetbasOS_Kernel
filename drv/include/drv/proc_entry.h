
#ifndef __KPROC_ENTRY_H_
#define __KPROC_ENTRY_H_

typedef int (read_proc_fun)(char *buf, int len); 

struct proc_entry
{
	struct proc_entry *next;
	time_t time;
	char *name;
	read_proc_fun *read_func;
	read_proc_fun *write_func;

	int read_size;
	int read_pos;
	char* read_buffer;
};

int pprintf(struct proc_entry *pf, const char *fmt, ...);

int register_proc_entry(struct proc_entry *proc);
int unregister_proc_entry(struct proc_entry *proc);
int read_proc(char *name, char *buf, int len);
int write_proc(char *name, char *buf, int len);
int ls_proc(char *buf, int len);


#endif

