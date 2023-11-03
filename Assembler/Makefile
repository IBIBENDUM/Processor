.PHONY: clean run

ifdef OS
   RM = del /Q
   SLASH = \\
else
   ifeq ($(shell uname), Linux)
      RM = rm -f
      SLASH = /
   endif
endif

CFLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

LIBS_DIR = ..$(SLASH)Libs
BUILD_DIR = build
EXECUTABLE = asm

PROGRAM_SRCS = $(wildcard *.cpp)
PROGRAM_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(PROGRAM_SRCS))
LIBS_SRCS = $(wildcard $(LIBS_DIR)/*.cpp)
LIBS_OBJS = $(patsubst %.cpp, $(LIBS_DIR)/%.o, $(LIBS_SRCS))
OBJS = $(PROGRAM_OBJS) $(LIBS_OBJS)

compile: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	g++ $^ $(CFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	g++ $^ $(CFLAGS) -c -o $@

$(LIBS_DIR)/%.o: %.cpp
	g++ $^ $(CFLAGS) -c -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	del /Q $(BUILD_DIR)\*.o
	del /Q $(LIBS_DIR)\*.o

run:
	.\$(EXECUTABLE) -i ../Examples/Text/input.s
