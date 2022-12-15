/* echoing input */

main() {
    extern int getchar(), putchar();
    while (1) {
        putchar(getchar());
    }
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

getchar() {
    asm ("xor ah, ah");
    asm ("int 0x16"); /* use bios call (?) to get character input */
    asm ("xor ah, ah"); /* ax contains character now */
}
