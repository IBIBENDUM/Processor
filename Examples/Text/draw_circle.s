;              RAM DOCS
;      ╔═══════════════════════╗
;      ║ [100] - X CENTER      ║
;      ║ [101] - Y CENTER      ║
;      ║ [102] - RADIUS        ║
;      ║ [103] - X MULTUPLIER  ║
;      ║ [104] - Y MULTIPLIER  ║
;      ║ [0]   - RECYCLE BIN   ║
;      ║ [1]   - VRAM WIDTH    ║
;      ║ [2]   - VRAM HEIGHT   ║
;      ║ [3]   - VRAM OFFSET   ║
;      ╚═══════════════════════╝
; FIRST 100 CELLS RESERVED FOR SYSTEM NEEDS

call main
HLT

main:
;   Init VRAM
    push 80
    pop [1]
    push 40
    pop [2]
    push 200
    pop [3]

    call fill_vram_with_poison
    call draw_circle
    ; call draw_anti_aliasing


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
            push [102]; Radius
            jbe below_radius
            jmp above_radius

            below_radius:
            push 10495
            pop rcx
            call paint_cell

            above_radius:
            push rax
            push 1
            add
            pop rax

            push rax
            push [1]; X size
            push 1
            add
            jb x_cycle

        push rbx
        push 1
        add
        pop rbx

        push rbx
        push [2]; Y size
        push 1
        add
        jb y_cycle

    ret

init_circle:
    ; push 40; X center
    push [1]
    push 2
    div
    pop [100]

    ; push 20; Y center
    push [2]
    push 2
    div
    pop [101]

    push 15; Radius
    pop [102]

    push 3;  X multiplier
    pop [103]

    push 1;  Y multiplier
    pop [104]

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
    push [100]
    pop rdx
    call calculate_line_length
    push rdx

    push [103]; X multiplier
    div

    push rbx
    pop rcx
    push [101]
    pop rdx
    call calculate_line_length
    push rdx

    push [104]; Y multiplier
    div

    add

    sqrt
    pop rcx

    ret

paint_cell:
    push rbx
    push [1]; X size
    push 2
    add
    mul
    push rax
    add
    push [3]; VRAM OFFSET
    add
    pop rdx
    push rcx
    pop [rdx]

    ret

fill_vram_with_poison:
    push 0
    pop rbx; Y = 0

    init_vram_y_cycle:
        push 0
        pop rax; X = 0

        init_vram_x_cycle:
            push 10441
            pop rcx
            call paint_cell

            push rax
            push 1
            add
            pop rax

            push rax
            push [1]; X size
            push 1
            add
            jb init_vram_x_cycle

        push rbx
        push 1
        add
        pop rbx

        push rbx
        push [2]; Y size
        push 1
        add
        jb init_vram_y_cycle

    ret
