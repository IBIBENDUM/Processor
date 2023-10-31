.PHONY: clean run

CFLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

LIB_DIR = ../Libs
BUILD_DIR = build
EXECUTABLE = asm

SRCS = $(wildcard *.cpp)
LIB_OBJS = $(wildcard $(LIB_DIR)/*.o)
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS)) $(LIB_OBJS)

compile: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	g++ $^ $(CFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	g++ $^ $(CFLAGS) -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	del /Q $(BUILD_DIR)

run:
	.\$(EXECUTABLE) -i ../Examples/input.s
