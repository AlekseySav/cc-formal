; source of tools/boot

program_segment equ 0x1000

    mov ax, program_segment
    mov ds, ax
    mov es, ax
    cli
    mov ss, ax
    xor sp, sp
    sti
    xor bx, bx
    mov ax, 0x0220 ; 32 sectors = 16 KiB, ok?
    mov cx, 2
    xor dh, dh
    int 0x13
    jc  $
    mov ax, 0x0e00|'O'
    int 0x10
    mov ax, 0x0e00|'K'
    int 0x10
    mov ax, 0x0e0a
    int 0x10
    mov ax, 0x0e0d
    int 0x10
    jmp program_segment:0

    times 510-($-$$) db 0
    dw 0xaa55
