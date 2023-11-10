# Processor
The project contains an Assembler and a Software Processing Unit (SPU)

The Assembler compiles a program written by the user, and if the program has errors, it informs the user of their location. Upon successful compilation, two files will be produced: a listing file and a bytecode file.

The SPU will execute commands from the file generated by the assembler. The data is stored in the stack and RAM.

Read in other languages: [English](https://github.com/IBIBENDUM/Processor/blob/main/README.md), [Русский](https://github.com/IBIBENDUM/Processor/blob/main/README_rus.md)

## Assembler
Syntax example
~~~
    push 5
    push 1
    sub
    out; print 5-1 = 4
    HLT
~~~

The Processor supports 19 commands

In the table below ---, --I, and so on are possible combinations of arguments.

| --- | --I | -RI | -R- | M-I | MR- | MRI |
| --- | --- | --- | --- | --- | --- | --- |

 So `push` command, which supports the `MRI` combination will accept all three arguments, for example `push [rax + 5]`

### Basic commands
| NAME | CODE | --- | --I | -RI | -R- | M-I | MR- | MRI |           DESCRIPTION            |
|:----:|:----:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:--------------------------------:|
| push |  1   |  -  |  +  |  +  |  +  |  +  |  +  |  +  |       push value to stack        |
| pop  |  2   |  +  |  -  |  -  |  +  |  +  |  +  |  +  |       pop value from stack       |
| out  |  14  |  +  |  -  |  -  |  -  |  -  |  -  |  -  | print the last stack value to stdout |
|  in  |  13  |  +  |  -  |  -  |  -  |  -  |  -  |  -  |  push value to stack from stdin  |
| HLT  |  12  |  +  |  -  |  -  |  -  |  -  |  -  |  -  |     terminate the program (halt)     |
| dump |  15  |  +  |  -  |  -  |  -  |  -  |  -  |  -  |       print VRAM to stdout       |

### Jumps
`jmp` command jumps by the command number, for instance
```
jmp 16; jump on the team with the number 16
jmp label; jump to the command that label points to
```

Conditional jumps are available, they will be executed under a certain condition, for example:
```
push 4
push 4
je mark; jump will be executed since the last two values in the stack are equal
~~~
push 4
push 3
jb mark; jump will not be executed since 4 > 3
```

>[!IMPORTANT]
>Conditional jumps take the last two values from the stack and do not return them back

`call` work on the same principle, except that the current position is saved in stack before jumping. Upon completion of the called method, the program can return to the original position using the `ret` command.

>[!IMPORTANT]
>`ret` will return us to the right place, but the last value of the stack should be the same number that the `call` command put

All jumps support these combinations of arguments: --I, -R-, M-I, MR-, MRI

| NAME | CODE |     DESCRIPTION      |
|:----:|:----:|:--------------------:|
| jmp  |  4   |     default jump     |
| call |  5   |  jump to the method  |
| ret  |  6   | jump from the method |
|  ja  |  7   |      jump if >       |
| jae  |  8   |      jump if >=      |
|  jb  |  9   |      jump if <       |
| jbe  |  10  |      jump if <=      |
|  je  |  11  |      jump if ==      |

You can pass values to the method using registers or using RAM
```
push 4
pop rax
call method; if you write `push rax` in the method, 4 will be put on the stack
~~~
push 4
pop [5]
call method; if you write `push [5]` in the method, 4 will be put on the
```

### Math
| NAME | CODE | DESCRIPTION |
|:----:|:----:|:-----------:|
| sqrt |  3   | square root |
| add  |  16  |     add     |
| sub  |  17  |  subtract   |
| mul  |  18  |  multiply   |
| div  |  19  |   divide    |

## How to use
### Assembler
- `-h` Print help information in console
- `-i` Select input file
- `-o` Select output file
-  `-l` Select a file for listing
- `-m` Select the level of log messages

To compile the quadratic_solver example, enter the following command in the console:
```
asm -i ../Examples/Text/quadratic_solver.s  -o ../Examples/Binaries/quadratic_solver.asm -m ERROR
```
### SPU
- `-h` Print help information in console
- `-i` Select input file
- `-o` Select output file
- `-m` Select the level of log messages

To compile the quadratic_solver example, enter the following command in the console:
```
spu -i ../Examples/Binaries/quadratic_solver.asm -m ERROR
```

## Examples
- quadratic_solver quadratic equation solver
- factorial_calc factorial calculator
- draw_circle draw a circle

## Build
To build the project, type `make` in the console within the Assembler folder, then again in the CPU folder.