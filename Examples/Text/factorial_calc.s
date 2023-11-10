call main
HLT

main:
    in
    pop rax

    call calc_factorial
    push rax
    out
    pop rax

    ret

;╔══════════════╗
;║ INPUT: rax   ║
;║ OUTPUT: rax  ║
;╚══════════════╝
calc_factorial:
    push rax
    push 1
    jbe recursion_base

    push rax

    push rax
    push 1
    sub
    pop rax

    call calc_factorial
    push rax
    mul
    pop rax

    ret

    ; if (rax <= 1) return 1
    recursion_base:
    ; remove rax value and replace with 1
        push 1
        pop rax

        ret
