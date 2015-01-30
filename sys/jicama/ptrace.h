
#ifndef _SYS_PTRACE_H
#define _SYS_PTRACE_H	1



/* Type of the REQUEST argument to `ptrace.'  */
enum PTRACE_REQUEST
{
  /* Indicate that the process making this request should be traced.
     All signals received by this process can be intercepted by its
     parent, and its parent can use the other `ptrace' requests.  */
  PTRACE_TRACEME = 0,

  /* Return the word in the process's text space at address ADDR.  */
  PTRACE_PEEKTEXT = 1,

  /* Return the word in the process's data space at address ADDR.  */
  PTRACE_PEEKDATA = 2,

  /* Return the word in the process's user area at offset ADDR.  */
  PTRACE_PEEKUSER = 3,

  /* Write the word DATA into the process's text space at address ADDR.  */
  PTRACE_POKETEXT = 4,

  /* Write the word DATA into the process's data space at address ADDR.  */
  PTRACE_POKEDATA = 5,

  /* Write the word DATA into the process's user area at offset ADDR.  */
  PTRACE_POKEUSER = 6,

  /* Continue the process.  */
  PTRACE_CONT = 7,

  /* Kill the process.  */
  PTRACE_KILL = 8,

  /* Single step the process.
     This is not supported on all machines.  */
  PTRACE_SINGLESTEP = 9,

  /* Get all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETREGS = 12,

  /* Set all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETREGS = 13,

  /* Get all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPREGS = 14,

  /* Set all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPREGS = 15,

  /* Attach to a process that is already running. */
  PTRACE_ATTACH = 16,

  /* Detach from a process attached to with PTRACE_ATTACH.  */
  PTRACE_DETACH = 17,

  /* Get all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPXREGS = 18,

  /* Set all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPXREGS = 19,

  /* Continue and stop at the next (return from) syscall.  */
  PTRACE_SYSCALL = 24
};



#endif /* _SYS_PTRACE_H */
