.PHONY: all clean run clean_log

all: exe/main.exe

# docs:
# 	@doxygen Doxyfile

run: all
	@exe/main.exe

clean:
	@rmdir d /s /q
	@rmdir o /s /q
	@rmdir exe /s /q
	@mkdir d
	@mkdir o
	@mkdir exe

clean_log:
	@rmdir log /s /q
	@mkdir log
	@mkdir log\dot-src
	@mkdir log\dot-img
	@mkdir log\dump
	@mkdir log\tex

CC:= gcc

O_FILES:= $(patsubst %.cpp,o/%.o,$(notdir $(wildcard src/*.cpp)))

DED_FLAGS := -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wpointer-arith -Wstack-usage=8192 -Wstrict-aliasing -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

CFLAGS:= -I ./h -Wno-unused-parameter -Wno-unused-function # $(DED_FLAGS)

exe/main.exe: $(O_FILES)
	@$(CC) $(O_FILES) -o exe/main.exe

include $(wildcard d/*.d)

o/%.o: src/%.cpp
	@$(CC) $< $(CFLAGS) -c -o $@
	@$(CC) -MM -MT $@ -I ./h $< -o d/$(patsubst %.o,%.d,$(notdir $@))