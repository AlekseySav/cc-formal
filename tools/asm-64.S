============= NOT IMPLEMENTED ======================
asm.S -- assembler instructions for each operator (x86_64 platform !!not finished!!)
Line index corresponds to token number
first 8 lines are unused (correspond bad tokens)
format: XY command
where X,Y are argument types: L=lvalue(%rbx), R=rvalue(%rax), N=name(%rax)
    @ is replaced with any optional parameter
assembler instructions are very inefficient, as far as i know
L-  mov (%rbx), %rax; $@, add (%rbx)                            /* b++   */
L-  mov (%rbx), %rax; $@, sub (%rbx)                            /* b--   */
R-  call %rax; add $@*8, %rsp                                   /* a()   */
L_index,
LN  add %rax, %rbx                                              /* b.a   */
LN  mov (%rbx), %rbx; add %rax, %rbx                            /* b->a  */
L-  add $@, (%rbx); mov (%rbx), %rax                            /* ++b   */
L-  sub $@, (%rbx); mov (%rbx), %rax                            /* --b   */
/* noting ... */                                                /* +a    */
R-  neg %rax                                                    /* -a    */
R-  test %rax, %rax; sete %al; cbw; cwde; cdqe                  /* !a    */
R-  not %rax                                                    /* ~a    */
L_cast,
L_star,
L_addr,
RR  mul %rbx                                                    /* b*a   */
RR  xor %rdx, %rdx; xchg %rax, %rbx; div %rbx                   /* b/a   */
RR  xor %rdx, %rdx; xchg %rax, %rbx; div %rbx; mov %rdx, %rax   /* b%a   */
RR  add %rbx, %rax                                              /* b+a   */
RR  sub %rax, %rbx; mov %rbx, %rax                              /* b-a   */
RR  mov %al, %cl; mov %rbx, %rax; shl %cl, %rax                 /* b<<a  */
RR  mov %al, %cl; mov %rbx, %rax; shr %cl, %rax                 /* b>>a  */
RR  and %rbx, %rax                                              /* b&a   */
RR  xor %rbx, %rax                                              /* b^a   */
RR  or  %rbx, %rax                                              /* b|a   */
RR  cmp %rbx, %rax; setl %al; cbw; cwde; cdqe                   /* b<a   */
RR  cmp %rbx, %rax; setle %al; cbw; cwde; cdqe                  /* b<=a  */
RR  cmp %rbx, %rax; setg %al; cbw; cwde; cdqe                   /* b>a   */
RR  cmp %rbx, %rax; setge %al; cbw; cwde; cdqe                  /* b>=a  */
RR  cmp %rbx, %rax; sete %al; cbw; cwde; cdqe                   /* b==a  */
RR  cmp %rbx, %rax; setne %al; cbw; cwde; cdqe                  /* b!=a  */
L_andl,             /* && */
L_orl,              /* || */
L_question,
L_colon,            /* : */
L_assign,
L_imul, L_idiv, L_imod,
L_iadd, L_isub,
L_ishl, L_ishr,
L_iand, L_ixor, L_ior,
L_comma,
L_n_operators,
L_open_curly,       /* { */
L_close_curly,      /* } */
L_semicolon,        /* ; */
L_empty_expr,
L_bad,
L_space,
L_newline,
L_quote,
