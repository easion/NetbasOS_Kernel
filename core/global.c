
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#define _JEB_GLOBAL


//tty devices
console_t *current_tty;
console_t *tty[NR_TTY];

volatile struct bparam_s boot_parameters={0,};

thread_t* bill_proc;

proc_t *proc[NR_PROC];
 int errno;

//memory


