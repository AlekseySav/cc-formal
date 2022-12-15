/*
 * tic-tac-toe game
 */

update(board, player, cursor)
char* board, player;
int* cursor;
{
    char c;
    extern int putchar(), getchar(), printf();
    c = getchar();
    if (c == '\r' & board[*cursor] == ' ') {
        board[*cursor] = player;
        return 1;
    }
    else if (c == 'a') (*cursor)--;
    else if (c == 'd') (*cursor)++;
    else if (c == 'w') (*cursor) -= 3;
    else if (c == 's') (*cursor) += 3;
    if ((*cursor) < 0) (*cursor) = 0;
    if ((*cursor) > 8) (*cursor) = 8;
    return 0;
}

render(board, cursor)
char* board;
int cursor;
{
    extern int printf(), putchar(), clear();
    int x, y;
    clear();
    y = 0;
    while (y < 9) {
        x = y;
        while (x < y + 3) {
            if (x != y) putchar('|');
            if (x == cursor) putchar('*');
            else putchar(board[x]);
            x++;
        }
        printf("\n\r");
        y += 3;
        if (y != 9) printf("-----\n\r");
    }
}

eq3(board, a, b, c)
char* board;
int a, b, c;
{
    return board[a] != ' ' & board[a] == board[b] & board[a] == board[c];
}

endgame(board, player)
char* board, player;
{
    extern int eq3();
    int i, j;
    i = 0;
    while (i < 3) {
        j = i * 3;
        if (eq3(board, j, j + 1, j + 2)) return 1; // row
        if (eq3(board, i, i + 3, i + 6)) return 1; // column
        i++;
    }
    if (eq3(board, 0, 4, 8)) return 1; // main diag
    if (eq3(board, 2, 4, 6)) return 1; // second diag
    return 0;
}

main() {
    extern int getchar(), printf(), endgame(), update(), render();
    char *board, player;
    int cursor;
    int x1,x2,x3,x4,x5; /* space for board, arrays not supported */
    x1 = x2 = x3 = x4 = x5 = (' ' << 8 | ' '); /* clear board with spaces */
    board = &x5;
    player = 'x';
    cursor = 4;
    printf("\n\r\n\rTictactoe\n\r'*' is cursor position\n\rmove cursor with WASD, then Enter to put character\n\rHit any key to play\n\r");
    getchar();
    render(board, cursor);
    while (!endgame(board, player)) {
        if (update(board, player, &cursor)) {
            if (player == 'x') player = 'o';
            else player = 'x';
        }
        render(board, cursor);
    }
    printf("player '%c' won", player & ~0x20); /* player & ~0x20 will uppercase letter */
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
    asm ("mov ax, [bp+4]");
}

getchar() {
    asm ("xor ah, ah");
    asm ("int 0x16"); /* use bios call (?) to get character input */
    asm ("xor ah, ah"); /* ax contains character now */
}

clear() {
    asm ("mov ax, 2"); /* clear screen */
    asm ("int 0x10");
}
