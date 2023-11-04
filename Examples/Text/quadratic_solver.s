;             RAM DOCS
; ╔══════════════════════════════════╗
; ║ [0]  - FIRST  ROOT               ║
; ║	[1]  - SECOND ROOT               ║
; ║ [2]  - NUMBMER OF ROOTS          ║
; ║ [3]  - UNIT TEST FIRST  ROOT     ║
; ║ [4]  - UNIT TEST SECOND ROOT     ║
; ║ [5]  - UNIT TEST NUMBER OF ROOTS ║
; ║ [10] - RECYCLE BIN               ║
; ╚══════════════════════════════════╝

call main
HLT

main:
    ; call get_coeffs
    ; call solve_qe
    call unit_test_1

    ret

unit_test_1:
    push 1; Push test number
    out; Print test number
    pop [10]; Remove value from stack

    push 1 ; Init 'a' coefficient
    pop rax

    push 0 ; Init 'b' coefficient
    pop rbx

    push 0 ; Init 'c' coefficient
    pop rcx

    push 0 ; Init first root
    pop [3]

    push 0 ; Init second root
    pop [3]

    call solve_qe
    call print_roots

    ; Check first root
    push [0]
    pop rax
    push [3]
    pop rbx
    call check_root
    push rcx

    ; Check second root
    push [1]
    pop rax
    push [4]
    pop rbx
    call check_root
    push rcx

    ; Because check_root return 1 if the root is true
    ; If the roots passed the test (1 + 1 = 2) test succeed

    add
    push 2
    je test_succeed

    push 0
    out
    pop [10]
    ret

    test_succeed:
        push 1
        out; OK
        pop [10]
        ret

; ╔══════════════════════════════════╗
; ║              INPUT               ║
; ║   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ║
; ║ rax  - ROOT                      ║
; ║	rbx  - UNIT TEST ROOT            ║
; ║~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~║
; ║              OUTPUT              ║
; ║   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ║
; ║ rcx  - RETURN VALUE              ║
; ║        1 IF EQUALS               ║
; ║        0 IF NOT                  ║
; ╚══════════════════════════════════╝
check_root:
    push rax
    push rbx
    je equal_roots
    push 0
    pop rcx
    ret

    equal_roots:
        push 1
        pop rcx
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

;ax^2 + bx = 0
solve_incomplete_qe:
    push 0
    ; out
    pop  [0]

    push rbx
    push 0
    je incomplete_qe_zero_b

    push -1
    push rbx
    mul
    push rax
    div
    ; out
    pop [1]

    ret

    incomplete_qe_zero_b:
        ret


two_roots:
    call d_sqrt_half

    call calculate_root_const_part

    push rax
    push rdx
    add
    ; out
    pop [0]

    push rax
    push rdx
    sub
    ; out
    pop [1]

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
    sqrt
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
    ; out
    pop [0]

    ret

complex_roots:
    push 404
    ; out
    pop [0]

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
        ; out
        pop [0]

        ret

free_member_equation:
    push rcx
    push 0
    je root_is_any_number
    jmp no_roots

root_is_any_number:
    push 888
    pop [0]
    ; out
    ret

no_roots:
    push -111
    pop [0]
    ; out
    ret

print_roots:
    push [0]
    out
    pop [0]

    push [5]
    push 2
    je print_second_root

    ret

    print_second_root:
        push [1]
        out
        pop [1]

        ret

