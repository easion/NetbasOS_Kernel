/*
    mcore.h - prototypes and definitions for mcore.c

    Copyright (c) Motorola, Inc., 2002
    ALL RIGHTS RESERVED

    You are hereby granted a copyright license to use, modify, and
    distribute the SOFTWARE so long as this entire notice is retained
    without alteration in any modified and/or redistributed versions,
    and that such modified versions are clearly identified as such.
    No licenses are granted by implication, estoppel or otherwise under
    any patents or trademarks of Motorola, Inc.

    The SOFTWARE is provided on an "AS IS" basis and without warranty.
    To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
    ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
    WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
    PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
    REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
    THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

    To the maximum extent permitted by applicable law, IN NO EVENT SHALL
    MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
    (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
    INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
    INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
    for the maintenance and support of the SOFTWARE.
*/

#if !defined(MCORE_H)
#define MCORE_H

#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL ((void *) 0)
#endif

/* platform-neutral functions */
void _start ( void );
int main ( void );
void gdb_handle_exception ( int sigval );
void gdb_console_output ( int len, const char *buf );

/* platform-specific functions in mcore.c */
int readvbr ( void );
int gdb_peek_register_file (int id, long *val);
int gdb_poke_register_file ( int id, long val );
int gdb_putc ( char c );
char gdb_getc ( void );
void signal (int vector);
void isr_RDRF1 ( void ) __attribute__ ((naked));
void gdb_step ( long addr );
void gdb_continue ( long addr );
void gdb_kill ( void );
void gdb_detach ( void );
void gdb_flush_cache ( void* cache_start, int cache_len );
void gdb_monitor_onentry ( void );
void gdb_monitor_onexit ( void );
void gdb_return_from_exception ( void );
void gdb_catch_exception ( void ) __attribute__ ((naked));
void _stub_start ( void ) __attribute__ ((naked));
void _stub_exit ( void );

/*      GDB_Signal          Stop Print Pass Description              */
/*      -----------------   ---- ----- ---- ------------------------ */
#define GDB_SIGHUP      1 /* Yes  Yes  Yes  Hangup                   */
#define GDB_SIGINT      2 /* Yes  Yes  No   Interrupt                */
#define GDB_SIGQUIT     3 /* Yes  Yes  Yes  Quit                     */
#define GDB_SIGILL      4 /* Yes  Yes  Yes  Illegal instruction      */
#define GDB_SIGTRAP     5 /* Yes  Yes  No   Trace/breakpoint trap    */
#define GDB_SIGABRT     6 /* Yes  Yes  Yes  Aborted                  */
#define GDB_SIGEMT      7 /* Yes  Yes  Yes  Emulation trap           */
#define GDB_SIGFPE      8 /* Yes  Yes  Yes  Arithmetic exception     */
#define GDB_SIGKILL     9 /* Yes  Yes  Yes  Killed                   */
#define GDB_SIGBUS     10 /* Yes  Yes  Yes  Bus error                */
#define GDB_SIGSEGV    11 /* Yes  Yes  Yes  Segmentation fault       */
#define GDB_SIGSYS     12 /* Yes  Yes  Yes  Bad system call          */
#define GDB_SIGPIPE    13 /* Yes  Yes  Yes  Broken pipe              */
#define GDB_SIGALRM    14 /* No   No   Yes  Alarm clock              */
#define GDB_SIGTERM    15 /* Yes  Yes  Yes  Terminated               */
#define GDB_SIGURG     16 /* No   No   Yes  Urgent I/O condition     */
#define GDB_SIGSTOP    17 /* Yes  Yes  Yes  Stopped (signal)         */
#define GDB_SIGTSTP    18 /* Yes  Yes  Yes  Stopped (user)           */
#define GDB_SIGCONT    19 /* Yes  Yes  Yes  Continued                */
#define GDB_SIGCHLD    20 /* No   No   Yes  Child status changed     */
#define GDB_SIGTTIN    21 /* Yes  Yes  Yes  Stopped (tty input)      */
#define GDB_SIGTTOU    22 /* Yes  Yes  Yes  Stopped (tty output)     */
#define GDB_SIGIO      23 /* No   No   Yes  I/O possible             */
#define GDB_SIGXCPU    24 /* Yes  Yes  Yes  CPU time limit exceeded  */
#define GDB_SIGXFSZ    25 /* Yes  Yes  Yes  File size limit exceeded */
#define GDB_SIGVTALRM  26 /* No   No   Yes  Virtual timer expired    */
#define GDB_SIGPROF    27 /* No   No   Yes  Profiling timer expired  */
#define GDB_SIGWINCH   28 /* No   No   Yes  Window size changed      */
#define GDB_SIGLOST    29 /* Yes  Yes  Yes  Resource lost            */
#define GDB_SIGUSR1    30 /* Yes  Yes  Yes  User defined signal 1    */
#define GDB_SIGUSR2    31 /* Yes  Yes  Yes  User defined signal 2    */
#define GDB_SIGPWR     32 /* Yes  Yes  Yes  Power fail/restart       */
#define GDB_SIGPOLL    33 /* No   No   Yes  Pollable event occurred  */
#define GDB_SIGWIND    34 /* Yes  Yes  Yes  SIGWIND                  */
#define GDB_SIGPHONE   35 /* Yes  Yes  Yes  SIGPHONE                 */
#define GDB_SIGWAITING 36 /* No   No   Yes  Process's LWPs blocked   */
#define GDB_SIGLWP     37 /* No   No   Yes  Signal LWP               */
#define GDB_SIGDANGER  38 /* Yes  Yes  Yes  Swap space low           */
#define GDB_SIGGRANT   39 /* Yes  Yes  Yes  Monitor mode granted     */
#define GDB_SIGRETRACT 40 /* Yes  Yes  Yes  Relinquish monitor mode  */
#define GDB_SIGMSG     41 /* Yes  Yes  Yes  Monitor mode data avail  */
#define GDB_SIGSOUND   42 /* Yes  Yes  Yes  Sound completed          */
#define GDB_SIGSAK     43 /* Yes  Yes  Yes  Secure attention         */
#define GDB_SIGPRIO    44 /* Yes  Yes  Yes  SIGPRIO                  */
#define GDB_SIG32      77 /* Yes  Yes  Yes  Real-time event 32       */
#define GDB_SIG33      45 /* Yes  Yes  Yes  Real-time event 33       */
#define GDB_SIG34      46 /* Yes  Yes  Yes  Real-time event 34       */
#define GDB_SIG35      47 /* Yes  Yes  Yes  Real-time event 35       */
#define GDB_SIG36      48 /* Yes  Yes  Yes  Real-time event 36       */
#define GDB_SIG37      49 /* Yes  Yes  Yes  Real-time event 37       */
#define GDB_SIG38      50 /* Yes  Yes  Yes  Real-time event 38       */
#define GDB_SIG39      51 /* Yes  Yes  Yes  Real-time event 39       */
#define GDB_SIG40      52 /* Yes  Yes  Yes  Real-time event 40       */
#define GDB_SIG41      53 /* Yes  Yes  Yes  Real-time event 41       */
#define GDB_SIG42      54 /* Yes  Yes  Yes  Real-time event 42       */
#define GDB_SIG43      55 /* Yes  Yes  Yes  Real-time event 43       */
#define GDB_SIG44      56 /* Yes  Yes  Yes  Real-time event 44       */
#define GDB_SIG45      57 /* Yes  Yes  Yes  Real-time event 45       */
#define GDB_SIG46      58 /* Yes  Yes  Yes  Real-time event 46       */
#define GDB_SIG47      59 /* Yes  Yes  Yes  Real-time event 47       */
#define GDB_SIG48      60 /* Yes  Yes  Yes  Real-time event 48       */
#define GDB_SIG49      61 /* Yes  Yes  Yes  Real-time event 49       */
#define GDB_SIG50      62 /* Yes  Yes  Yes  Real-time event 50       */
#define GDB_SIG51      63 /* Yes  Yes  Yes  Real-time event 51       */
#define GDB_SIG52      64 /* Yes  Yes  Yes  Real-time event 52       */
#define GDB_SIG53      65 /* Yes  Yes  Yes  Real-time event 53       */
#define GDB_SIG54      66 /* Yes  Yes  Yes  Real-time event 54       */
#define GDB_SIG55      67 /* Yes  Yes  Yes  Real-time event 55       */
#define GDB_SIG56      68 /* Yes  Yes  Yes  Real-time event 56       */
#define GDB_SIG57      69 /* Yes  Yes  Yes  Real-time event 57       */
#define GDB_SIG58      70 /* Yes  Yes  Yes  Real-time event 58       */
#define GDB_SIG59      71 /* Yes  Yes  Yes  Real-time event 59       */
#define GDB_SIG60      72 /* Yes  Yes  Yes  Real-time event 60       */
#define GDB_SIG61      73 /* Yes  Yes  Yes  Real-time event 61       */
#define GDB_SIG62      74 /* Yes  Yes  Yes  Real-time event 62       */
#define GDB_SIG63      75 /* Yes  Yes  Yes  Real-time event 63       */

#endif /* MCORE_H */
