# cc-formal

### dependencies
`qemu-system-x86_64`, `nasm`

### build
- `make` &mdash; make release build
- `make config=debug` &mdash; make debug build
- `make config=test && ./cc` &mdash; run tests

### compile & run C source file with

- build with `tools/ccom.sh examples/printf.c printf`
- run with `qemu-system-x86_64 printf`
- or build & run with `tools/ccom.sh -run examples/printf.c printf`

### limitations

#### keywords
supported only `asm`, `int`, `char`, `return`, `extern`, `if`, `else`, `while`, `break`, `continue`
#### operators
not supported `cast` and `||`, `&&`, `? :`
#### definitions
only typeless function and non-array variables can be defined in global scope (check examples i guess)
### size
only very small code block can be executed, as it should fit in 15KiB.
### pointers
pointer arithmetic (e.g `+` `-`) not implemented for pointers, though `++`, `--`, `*` and `a[]` can be used instead.
