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

; BAH: MAKE NORMAL NAMES

call main
HLT

main:
;   Init VRAM
    push 60
    pop [1]
    push 30
    pop [2]
    push 200
    pop [3]

    call fill_vram_with_poison
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
            push [102]; Radius
            push 0.5
            add
            jbe below_0

            push rcx
            push [102]; Radius
            push 0.7
            add
            jbe below_1

            push rcx
            push [102]; Radius
            push 1.05
            add
            jbe below_2

            jmp above_radius

            below_1:
                call calculate_point_quarter
                push rcx
                ; out
                push 1
                je aa_1

                push rcx
                push 2
                je aa_2

                push rcx
                push 3
                je aa_3

                jmp aa_4

                aa_1:
                    push 10479
                    ; push 49
                    pop rcx
                    call paint_cell
                    jmp above_radius
                aa_2:
                    push 10493
                    ; push 50
                    pop rcx
                    call paint_cell
                    jmp above_radius
                aa_3:
                    push 10491
                    ; push 10479
                    ; push 51
                    pop rcx
                    call paint_cell
                    jmp above_radius

                aa_4:
                    push 10463
                    ; push 52
                    pop rcx
                    call paint_cell
                    jmp above_radius

            below_2:
                call calculate_point_quarter
                push rcx
                out
                push 1
                je ab_2_1

                push rcx
                push 2
                je ab_2_2

                push rcx
                push 3
                je ab_2_3

                jmp ab_2_4

                ab_2_1:
                    push 10477
                    ; push 49
                    pop rcx
                    call paint_cell
                    jmp above_radius
                ab_2_2:
                    push 10477
                    ; push 50
                    pop rcx
                    call paint_cell
                    jmp above_radius
                ab_2_3:
                    ; push 51
                    push 10459
                    ; push 51
                    pop rcx
                    call paint_cell
                    jmp above_radius

                ab_2_4:
                    ; push 52
                    push 10459
                    pop rcx
                    call paint_cell
                    jmp above_radius

            below_0:
                push 10495
                pop rcx
                call paint_cell
                jmp above_radius

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

    push 10; Radius
    pop [102]

    push 3;  X multiplier
    pop [103]

    push 1;  Y multiplier
    pop [104]

    ret

calculate_point_quarter:
    push rax
    push [100]; X center
    sub
    push 0
    ja quarters_1_or_4
    jmp quarters_2_or_3
    quarters_1_or_4:
        push rbx
        push [101]; Y center
        sub
        push 0
        jb quarter_1
        push 4
        pop rcx
        ret
        quarter_1:
            push 1
            pop rcx
            ret

    quarters_2_or_3:
        push rbx
        push [101]; Y center
        sub
        push 0
        jb quarter_2
        push 3
        pop rcx
        ret
        quarter_2:
            push 2
            pop rcx
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

get_current_position:
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
    ret

paint_cell:
    call get_current_position
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
