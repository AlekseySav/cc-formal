/* printf example (supports only %% %d %s %c) */

main() {
    extern int printf();
    printf("hello, %s %d! %c\n\r", "world!", 123, 'q');
    return 0;
}

printf(fmt, _)
char* fmt;
int _;
{
    extern int printn(), putchar();

    char c, *str;
    int* ap;
    ap = &_; /* argument pointer */

    while (c = *fmt++) {
        if (c != '%') {
            putchar(c);
            continue;
        }
        c = *fmt++;
        if (c == '%') {
            putchar(c);
            continue;
        }
        if (c == 'd') {
            printn(*ap++, 10);
            continue;
        }
        if (c == 's') {
            str = *ap++;
            while (c = *str++)
                putchar(c);
        }
        if (c == 'c') {
            putchar(*ap++);
        }
    }
}

/*
 * printn implementation borrowed from https://en.wikipedia.org/wiki/B_(programming_language)#Examples
 */
printn(n, base)
int n, base; /* specifying type for arguments 'n' and 'base' */
{
    /* declare putchar as extern inside for this function */
    extern int putchar(), printn();
    int a; /* variable on stack */
    if (a = n / base)
        printn(a, base);
    putchar(n % base + '0');
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
