include ../colors.mk

CXX := clang
CFLAGS := -Wall -Iinclude/ -g
LDFLAGS := -lm -ldl -lreadline

SOURCES := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPS := $(patsubst src/%.c,deps/%.d,$(SOURCES))
HEADERS := $(shell find include -name '*.h') $(shell find src -name '*.h')

# Assume all sources are in a "flat" directory
LIB_SOURCES := $(wildcard lib/*.c)
LIBFLAGS := -fpic -shared
LIB_TARGETS := $(patsubst lib/%.c,extern/%.so,$(LIB_SOURCES))


TARGET := crane

all:
	@printf '$(RED)>$(BLK) Building target $(TARGET)$(RST)\n'
	@mkdir -p build/deps/lib
	@mkdir -p build/lib
	@mkdir -p extern
	@make $(TARGET) --no-print-directory -s
	@printf '$(RED)>$(BLK) Compiling shared libraries: $(patsubst extern/%.so,%,$(LIB_TARGETS))$(RST)\n'
	@make $(LIB_TARGETS)  --no-print-directory -s

$(TARGET): $(OBJ) ${HEADERS}
	@printf '$(YLW)==>$(BLK) Linking target $(TARGET)$(RST)\n'
	@$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(LIB_TARGETS): extern/%.so : lib/%.c ${HEADERS}
	@printf '$(GRN)==>$(BLK) Linking shared library $(subst lib/%.c,%,$<)$(RST)\n'
	@$(CXX) $< -o $@ $(CFLAGS) $(LIBFLAGS) -MD -MF $(patsubst extern/%.so,build/deps/lib/%.d,$@)

$(OBJ): build/%.o : src/%.c
	@printf '$(GRN)==>$(BLK) Compiling $<$(RST)\n'
	@$(CXX) -c $< $(CFLAGS) -o $@ -MD -MF $(patsubst build/%.o,build/deps/%.d,$@)

clean:
	@printf '$(RED)>$(BLK) Cleaning target $(TARGET)$(RST)\n'
	@find . -name '*.o' -exec rm -rf {} \;
	@rm -rf build/deps
	@rm -rf build
	@rm -rf extern
	@rm -rf $(TARGET)