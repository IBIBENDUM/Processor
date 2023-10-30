;~~~~~~~~~~RAM~~~~~~~~~~~~
; [0] X center
;
; [1] Y center
;
; [2] Radius
;
; [3] Y size
;
; [4] X size
;
; [5] X multiplier
;
; [6] Y multiplier
;
; [7] VRAM OFFSET
;~~~~~~~~~~~~~~~~~~~~~~~~~

call main
HLT

main:
    call draw_circle
    dump
    ret

draw_circle:
    call init_circle

    push 0
    pop rbx; Y = 0

    y_cycle:
        push 0
        pop rax; X = 0

        x_cycle:
            call calculate_distance

            push rcx
            push [2]; Radius
            jbe below_radius
            jmp above_radius

            below_radius:
            call paint_cell

            above_radius:
            push rax
            push 1
            add
            pop rax

            push rax
            push [4]; X size
            jb x_cycle

        push rbx
        push 1
        add
        pop rbx

        push rbx
        push [3]; Y size
        jb y_cycle

    ret

init_circle:
    push 40; X center
    pop [0]

    push 20; Y center
    pop [1]

    push 15; Radius
    pop [2]

    push 40; Y size
    pop [3]

    push 80; X size
    pop [4]

    push 3;  X multiplier
    pop [5]

    push 1;  Y multiplier
    pop [6]

    push 20; VRAM OFFSET
    pop [7]

    ret

calculate_line_length:
    push rcx
    push rdx; X center
    sub

    push rcx
    push rdx
    sub

    mul

    pop rdx

    ret

calculate_distance:
    push rax
    pop rcx
    push [0]
    pop rdx
    call calculate_line_length
    push rdx

    push [5]; X multiplier
    div

    push rbx
    pop rcx
    push [1]
    pop rdx
    call calculate_line_length
    push rdx

    push [6]; Y multiplier
    div

    add

    sqrt
    pop rcx

    ret

paint_cell:
    push rbx
    push [4]; X size
    mul
    push rax
    add
    push [7]; VRAM OFFSET
    add
    pop rdx

    push 1
    pop [rdx]

    ret
