DEF_CMD_ERR(CMD_NO_ERR, "no error%.*ls")
DEF_CMD_ERR(CMD_REPEATED_LABEL_ERR, "previous declaration of label '%.*ls'")
DEF_CMD_ERR(CMD_WRONG_NAME_ERR, "command '%.*ls' wasn't declared")
DEF_CMD_ERR(CMD_WRONG_ARG_ERR, "invalid argument for '%.*ls'")
DEF_CMD_ERR(CMD_WRONG_LABEL_NAME, "no previous '%.*ls' declaration")
DEF_CMD_ERR(CMD_TOO_LONG_LABEL_ERR, "too long label name (edit constant in 'assembler.h')%.*ls")
DEF_CMD_ERR(CMD_TOO_MANY_LABEL_ERR, "too much labels (edit constant in 'assembler.h')%.*ls")
DEF_CMD_ERR(CMD_TOO_MANY_ARGS, "too many arguments to command '%.*ls'")
DEF_CMD_ERR(TOO_FEW_ARGS_ERR, "too few arguments to command '%.*ls'")

#undef DEF_CMD_ERR
