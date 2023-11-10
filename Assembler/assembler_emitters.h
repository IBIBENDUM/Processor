#ifndef ASM_EMITTERS_H
#define ASM_EMITTERS_H

#include <wchar.h>

#include "assembler.h"
#include "assembler_errors.h"

cmd_error emit_command(const Command* const cmd, Command_error* const cmd_err, Bytecode* bytecode);

cmd_error emit_bytecode(const void* value, const size_t type_size, Bytecode* bytecode);

cmd_error emit_label_arg(Command* cmd, Labels* labels, const wchar_t* arg_ptr);

cmd_error emit_imm_arg(Command* cmd, const wchar_t* arg_ptr);

cmd_error emit_reg_arg(Command* cmd, const wchar_t* arg_ptr);

cmd_error emit_label(const line* label_name, const size_t position, Labels* labels);

bool emit_ram_arg(Command* cmd, wchar_t* arg_ptr, wchar_t**  left_br_ptr, wchar_t** right_br_ptr);

#endif
