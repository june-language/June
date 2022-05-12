include ../colors.mk

OS := $(shell uname -s)

CXX := clang++
CFLAGS := -Wall -Iinclude/ -g
LDFLAGS := -lm -ldl -lreadline

SOURCES := $(shell find src -name '*.cpp')
OBJ := $(patsubst src/%.cpp,build/%.o,$(SOURCES))
DEPS := $(patsubst src/%.cpp,deps/%.d,$(SOURCES))
HEADERS := $(shell find include -name '*.hpp') $(shell find src -name '*.hpp')

# Assume all sources are in a "flat" directory
LIB_SOURCES := $(wildcard lib/*.cpp)
LIBFLAGS := -fpic -shared

TARGET := crane

ifeq ($(OS), Darwin)
LIB_EXT := .dylib
LIB_TARGETS := $(patsubst lib/%.cpp,extern/%.dylib,$(LIB_SOURCES))
CFLAGS += -DCRANE_DARWIN
else
LIB_EXT := .so
LIB_TARGETS := $(patsubst lib/%.cpp,extern/%.so,$(LIB_SOURCES))
endif

all:
	@#printf '$(RED)>$(BLK) Building target $(TARGET)$(RST)\n'
	@mkdir -p build/deps/lib
	@mkdir -p build/lib
	@mkdir -p extern
	@make $(TARGET) --no-print-directory
	@#printf '$(RED)>$(BLK) Compiling shared libraries: $(patsubst extern/%.so,%,$(LIB_TARGETS))$(RST)\n'
	@make $(LIB_TARGETS) --no-print-directory

$(TARGET): $(OBJ) ${HEADERS}
	@#printf '$(YLW)==>$(BLK) Linking target $(TARGET)$(RST)\n'
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(LIB_TARGETS): extern/%$(LIB_EXT) : lib/%.cpp ${HEADERS}
	@#printf '$(GRN)==>$(BLK) Linking shared library $(subst lib/%.cpp,%,$<)$(RST)\n'
	$(CXX) $< -o $@ $(CFLAGS) $(LIBFLAGS) -MD -MF $(patsubst extern/%.so,build/deps/lib/%.d,$@)

$(OBJ): build/%.o : src/%.cpp
	@#printf '$(GRN)==>$(BLK) Compiling $<$(RST)\n'
	$(CXX) -c $< $(CFLAGS) -o $@ -MD -MF $(patsubst build/%.o,build/deps/%.d,$@)

clean:
	@#printf '$(RED)>$(BLK) Cleaning target $(TARGET)$(RST)\n'
	@find . -name '*.o' -exec rm -rf {} \;
	@rm -rf build/deps
	@rm -rf build
	@rm -rf extern
	@rm -rf $(TARGET)
