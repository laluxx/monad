[BITS 32]

; Declare external C function
extern kernel_main

; Entry point
global _start

section .text
_start:
    ; We're already in protected mode with a stack
    ; Just call the kernel
    call kernel_main

    ; If it returns, hang
    cli
.hang:
    hlt
    jmp .hang
