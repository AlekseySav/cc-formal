/* put string on screen example */

main() {
    extern int puts();
    puts("hello, world!\n\r");
    return 0;
}

puts(msg)
char* msg;
{
    extern int putchar();
    char c;
    while (c = *msg++)
        putchar(c);
}

putchar(c) {
    /*
     * [bp] contains old bp
     * [bp+2] contains return address
     * so first argument stored at [bp+4]
     */
    asm ("mov al, [bp+4]");
    asm ("mov ah, 0x0e");
    asm ("int 0x10"); /* use bios call (?) to print character on screen */
}
