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

;~~~~~~~~~~~~~~~~~~~~~~~~~
; input: rax
; output: rax
;~~~~~~~~~~~~~~~~~~~~~~~~~
calc_factorial:
    push rax
    push 1
    jbe ret_jmp

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
    ret_jmp:
    ; remove rax value and replace with 1
        push 1
        pop rax

        ret
