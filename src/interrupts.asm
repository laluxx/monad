[BITS 32]

; Export symbols
global idt_load
global irq0_handler
global irq1_handler

; Import C functions
extern timer_handler
extern keyboard_handler

; Load IDT
idt_load:
    mov eax, [esp + 4]  ; Get IDT pointer from argument
    lidt [eax]
    ret

; Timer interrupt handler (IRQ0)
irq0_handler:
    pusha               ; Save all registers

    call timer_handler

    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al

    popa                ; Restore registers
    iret                ; Return from interrupt

; Keyboard interrupt handler (IRQ1)
irq1_handler:
    pusha               ; Save all registers

    call keyboard_handler

    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al

    popa                ; Restore registers
    iret                ; Return from interrupt
