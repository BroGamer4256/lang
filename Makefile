OUT = lang
CC := clang
CXX := clang++
TARGET := x86_64-pc-windows-gnu
SRC = src/dllmain.cpp src/helpers.c src/sigscan.c tomlc99/toml.c minhook/src/buffer.c minhook/src/hook.c minhook/src/trampoline.c minhook/src/hde/hde32.c minhook/src/hde/hde64.c
OBJ = $(addprefix $(TARGET)/,$(subst .cpp,.o,$(SRC:.c=.o)))
CFLAGS = -Iminhook/include -Itomlc99 -Wall -Ofast -target $(TARGET) -D_WIN32_WINNT=_WIN32_WINNT_WIN7
LDFLAGS := -shared -static -static-libgcc -s
LIBS := -lmingw32 -luuid -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lsetupapi -lversion -lshlwapi -pthread

all: options $(OUT)

.PHONY: options
options:
	@mkdir -p $(TARGET)/src
	@mkdir -p $(TARGET)/minhook/src/hde
	@mkdir -p $(TARGET)/tomlc99
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "LDFLAGS	= $(LDFLAGS)"
	@echo "CC	= $(CC)"

$(TARGET)/%.o: %.c
	@echo BUILD $@
	@bear -- $(CC) -c $(CFLAGS) $< -o $@

$(TARGET)/%.o: %.cpp
	@echo BUILD $@
	@bear -- $(CXX) -std=c++20 -c $(CFLAGS) $< -o $@

.PHONY: $(OUT)
$(OUT): $(OBJ)
	@echo LINK $@
	@$(CXX) $(CFLAGS) -std=c++20 -o $(TARGET)/$@.dll $(OBJ) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(TARGET)
