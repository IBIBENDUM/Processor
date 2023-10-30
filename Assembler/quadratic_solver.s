call main
HLT

main:
    call get_coeffs
    call solve_qe

    ret

get_coeffs:
    in
    pop rax

    in
    pop rbx

    in
    pop rcx

    ret

solve_qe:
    push rax
    push 0
    je zero_a

    push rcx
    push 0
    je quadratic_zero_c

    call calculate_discr

    push rdx
    push 0
    je zero_discr

    push rdx
    push 0
    jb negative_discr

    pop rdx
    call two_roots
    ret

    zero_a:
        call solve_linear_equation
        ret

    quadratic_zero_c:
        call solve_incomplete_qe
        ret

    negative_discr:
        call complex_roots
        ret

    zero_discr:
        call one_root
        ret

calculate_discr:
    push rbx
    push rbx
    mul
    push rax
    push rcx
    mul
    push 4
    mul
    sub
    pop rdx

    ret

;ax + b = 0
solve_incomplete_qe:
    pop rcx

    push 0
    out

    push -1
    push rbx
    mul
    push rax
    div
    out

    ret

two_roots:
    push rdx
    sqrt
    pop rdx

    call d_sqrt_half
    push rdx

    call calculate_root_const_part

    push rax
    push rdx
    add
    out

    push rax
    push rdx
    sub
    out

    ret


calculate_root_const_part:
    push rbx
    push -1
    mul
    push rax
    push 2
    mul
    div
    pop rax
    ret


d_sqrt_half:
    push rdx
    push 2
    push rax
    mul
    div
    pop rdx
    ret


one_root:
    push 0
    push rbx
    sub
    push 2
    div
    push rax
    div
    out

    ret

complex_roots:
    push 404
    out

    ret

solve_linear_equation:
    ; a = 0
    push rbx
    push 0
    je zero_b

    ; a = 0, b != 0 c = 0
   push rcx
   push 0
   je root_zero

    ; a = 0, b != 0, c != 0,
    push rcx
    push -1
    mul

    push rbx
    div

    ret

    zero_b:
        call free_member_equation
        ret

    root_zero:
        pop rcx
        push 0
        out
        ret

free_member_equation:
    push rcx
    push 0
    je root_is_any_number
    jmp no_roots

root_is_any_number:
    push 888
    out
    ret

no_roots:
    push -111
    out
    ret
