	.h8300s
	
	.global _gdb_register_file
	.extern _h8300_handle_exception
	.extern _gdb_monitor_onexit
	
	.global _h8300_trapa_handler
_h8300_trapa_handler:
	mov.l er1, @-sp
	mov.l er0, @-sp

	/* find end of register file */
	mov #(_gdb_register_file + 552), er0

	/* save pre-exception stack pointer */
	mov er7, er1
	add #12, er1
	mov.l er1, @-er0

	/* pop and save er0 and er1 */
	mov.l @sp+, er1
	mov.l er1, @-er0
	mov.l @sp+, er1
	mov.l er1, @-er0

	/* save everything else */
	mov.l er2, @-er0
	mov.l er3, @-er0
	mov.l er4, @-er0
	mov.l er5, @-er0
	mov.l er6, @-er0

	/* save ccr */
	mov.b @sp, r1l
	and #0xff, er1
	mov.l er1, @-er0

	/* save original pc */
	mov.l @sp, er1
	and #0xffffff, er1
	sub #2, er1
	mov.l er1, @-er0

	/* switch stacks */
	mov er0, er7

	/* send signal 5 to gdb */
	mov #5, er0
	jsr @_h8300_handle_exception


	.global _gdb_return_from_exception
_gdb_return_from_exception:

	jsr @_gdb_monitor_onexit
	
	mov #(_gdb_register_file + 512), er0
	
	mov.l @(36,er0), er7
	mov.l @er0+, er1
	mov.l er1, @-sp
	mov.l @er0+, er1
	mov.b r1l, @sp	
	
	mov.l @er0+, er6
	mov.l @er0+, er5
	mov.l @er0+, er4
	mov.l @er0+, er3
	mov.l @er0+, er2
	mov.l @er0+, er1
	mov.l @er0, er0
	rte


#if defined(CRT0)
#if defined(__H8300S__)
	.global _start
	.extern _main
	.h8300s


	.section .vect
_vector_table:	
        .long     _start               ; Reset     Vec 0       External Pin
        .long     0                    ; Reserved  Vec 1
        .long     0                    ; Reserved  Vec 2
        .long     0                    ; Reserved  Vec 3
        .long     0                    ; Reserved  Vec 4
        .long     0                    ; Reserved  Vec 5
        .long     0                    ; Reserved  Vec 6
        .long     0                    ; NMI       Vec 7       External pin
        .long     _h8300_trapa_handler ; TRAPA 0   Vec 8       Software
        .long     _h8300_trapa_handler ; TRAPA 1   Vec 9
        .long     _h8300_trapa_handler ; TRAPA 2   Vec 10
        .long     _h8300_trapa_handler ; TRAPA 3   Vec 11
	

	.section .text
_start:	
	mov #_stack, er7
	jsr @_main
	bra _start
#endif
#endif
