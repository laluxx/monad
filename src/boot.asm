[BITS 16]
[ORG 0x7C00]

start:
    ; Setup segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load kernel to 0x1000 (50 sectors starting at sector 2)
    mov ah, 0x02            ; BIOS read
    mov al, 50              ; Sector count (INCREASED)
    mov ch, 0               ; Cylinder 0
    mov cl, 2               ; Start sector 2
    mov dh, 0               ; Head 0
    mov bx, 0x1000          ; Load address
    int 0x13

    ; Enter protected mode
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:pm

[BITS 32]
pm:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Copy kernel from 0x1000 to 0x10000
    mov esi, 0x1000
    mov edi, 0x10000
    mov ecx, 6400           ; 50*512/4 dwords (INCREASED)
    rep movsd

    jmp 0x10000

[BITS 16]
; GDT
align 4
gdt:
    dq 0                    ; Null
    dq 0x00CF9A000000FFFF   ; Code
    dq 0x00CF92000000FFFF   ; Data
gdt_desc:
    dw $ - gdt - 1
    dd gdt

times 510-($-$$) db 0
dw 0xAA55

;; [BITS 16]
;; [ORG 0x7C00]

;; start:
;;     ; Setup segments and stack
;;     xor ax, ax
;;     mov ds, ax
;;     mov es, ax
;;     mov ss, ax
;;     mov sp, 0x7C00

;;     ; Load kernel to 0x1000 (50 sectors starting at sector 2)
;;     mov ah, 0x02            ; BIOS read
;;     mov al, 50              ; Sector count
;;     mov ch, 0               ; Cylinder 0
;;     mov cl, 2               ; Start sector 2
;;     mov dh, 0               ; Head 0
;;     mov bx, 0x1000          ; Load address
;;     int 0x13

;;     ; Enter protected mode
;;     cli
;;     lgdt [gdt_desc]
;;     mov eax, cr0
;;     or al, 1
;;     mov cr0, eax
;;     jmp 0x08:pm

;; [BITS 32]
;; pm:
;;     ; Setup segments
;;     mov ax, 0x10
;;     mov ds, ax
;;     mov es, ax
;;     mov ss, ax
;;     mov esp, 0x90000

;;     ; Copy kernel from 0x1000 to 0x10000
;;     mov esi, 0x1000
;;     mov edi, 0x10000
;;     mov ecx, 6400           ; 50*512/4 dwords
;;     rep movsd

;;     ; Zero out VBE mode info at 0x5000 to signal "no VESA available"
;;     ; The kernel will decide whether to use VGA text or try VESA
;;     xor eax, eax
;;     mov edi, 0x5000
;;     mov ecx, 64
;;     rep stosd

;;     jmp 0x10000

;; [BITS 16]
;; ; GDT
;; align 4
;; gdt:
;;     dq 0                    ; Null
;;     dq 0x00CF9A000000FFFF   ; Code
;;     dq 0x00CF92000000FFFF   ; Data

;; gdt_desc:
;;     dw $ - gdt - 1
;;     dd gdt

;; times 510-($-$$) db 0
;; dw 0xAA55
