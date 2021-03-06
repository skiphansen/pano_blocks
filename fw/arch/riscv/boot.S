
.global init
.global main
.global irq_handler

#####################################################
# Entry point
#####################################################
.org    0x0
boot_vector:
	j start

#####################################################
# IPC Vector
#####################################################
.org   0x8
ipc_vector:
	.zero

#####################################################
# ISR Vector
#####################################################
.org    0x10
    j int_vector

#####################################################
# Arg handling
#####################################################
.org 0x20
arg_argc:
.space 4, 0
arg_argv:
.space 256, 0

#####################################################
# Actual ISR vector
#####################################################
int_vector:
#include "exception_asm.h"

#####################################################
# Start
#####################################################
start:
    # Setup stack pointer
    lui sp, %hi(_sp)
    add sp, sp, %lo(_sp)

    # Setup IRQ vector
    lui t0, %hi(isr_vector)
    add t0, t0, %lo(isr_vector)
    csrw mtvec, t0

    # t0 = _bss_start
    lui t0,%hi(_bss_start)
    add t0,t0,%lo(_bss_start)
    
    # t1 = _end
    lui t1,%hi(_end)
    add t1,t1,%lo(_end)

bss_clear:
#ifndef SKIP_BSS_INIT
    sw x0,  (0)(t0)        # Write 0x00 to mem[t0]
    add t0, t0, 4          # t0 += 4
    blt t0, t1, bss_clear  # Branch back to bss_clear if (t0 < t1)
#endif

    # Jump to init
    jal init

    # Run main
    # a0 = argc
    la a0, arg_argc
    lw a0, 0(a0)
    # a1 = argv
    la a1, arg_argv
    jal main

_die:
    j _die

    # Exit    
    jal _exit

#####################################################
# Exit
#####################################################
.global _exit
_exit:
    csrw dscratch, x0
_exit_loop:
    j _exit_loop
