
#define CR4_VME	0x00000001		/* Enable virtual intrs in v86 mode. */
#define CR4_PVI	0x00000002		/* Enable virtual intrs in pmode. */
#define CR4_TSD	0x00000004		/* Disable RDTSC in user mode. */
#define CR4_DE	0x00000008		/* Debug extensions (I/O breakpoints). */
#define CR4_PSE	0x00000010		/* Page size extensions. */
#define CR4_PGE	0x00000020		/* Page global extensions. */
#define CR4_MCE	0x00000040		/* Machine check exception. */
#define CR4_PCE	0x00000100		/* Enable read perf counter instr. */