/* put number in base 2<=base<=10 on screen example */

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

main() {
    extern int printn(), putchar();
    printn(123, 10);
    putchar('\n');
    putchar('\r');
    printn(0123, 8);
    putchar('\n');
    putchar('\r');

    int x, y, *arr;
    char *c_arr;
    x = 0x1234, y = 0x5678;
    c_arr = arr = &y;
    printn(arr[0], 16); // y
    putchar(' ');
    printn(arr[1], 16); // x
    putchar(' ');
    printn(arr[-1], 10); // arr
    putchar(' ');
    printn(c_arr[0], 16); // y & 0xff
    putchar(' ');
    printn(c_arr[1], 16); // y >> 16
    putchar(' ');
    return 0;
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
