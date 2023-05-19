# General config

C        := gcc
CFLAGS   := -Wall -Wextra -std=c11
LDFLAGS  := -Llib

R_CFLAGS := -O2 -flto -DNDEBUG
D_CFLAGS := -O0 -g -fstack-protector-strong -D_DEBUG -DDEBUG

GDB      := gdb
GDBFLAGS :=

VALGRIND      := valgrind
VALGRINDFLAGS := --leak-check=full --track-origins=yes -s --error-exitcode=1

RUNFLAGS :=

SOURCE_DIR  := src
INCLUDE_DIR := include
COMPILE_DIR := build

TARGET             := remote-wrapper

SRC             := $(shell find $(SOURCE_DIR) $(INCLUDE_DIR) -type f -iname "*.c")

D_BUILD_DIR   := $(COMPILE_DIR)/debug
D_OBJECT_DIR  := $(D_BUILD_DIR)
D_BINARY_DIR  := $(D_BUILD_DIR)/bin
D_OBJECTS     := $(SRC:%.c=$(D_OBJECT_DIR)/%.o)

R_BUILD_DIR   := $(COMPILE_DIR)/release
R_OBJECT_DIR  := $(R_BUILD_DIR)
R_BINARY_DIR  := $(R_BUILD_DIR)/bin
R_OBJECTS     := $(SRC:%.c=$(R_OBJECT_DIR)/%.o)

.PHONY: all clean debug release debug_run release_run gdb valgrind compile_commands
all: release

# Compilation

$(D_OBJECT_DIR)/%.o: %.c
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@) \
	  && printf "[\033[34mMKDIR\033[0m] $(dir $@)\n"; fi
	@$(C) $(CFLAGS) $(D_CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@ \
	  && printf "[\033[32mC\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m\n"

$(R_OBJECT_DIR)/%.o: %.c
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@) \
	  && printf "[\033[34mMKDIR\033[0m] $(dir $@)\n"; fi
	@$(C) $(CFLAGS) $(R_CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@ \
	  && printf "[\033[32mC\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m\n"

# Linking targets

$(D_BINARY_DIR)/$(TARGET): $(D_OBJECTS)
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@) \
	  && printf "[\033[34mMKDIR\033[0m] $(dir $@)\n"; fi
	@$(C) $(CFLAGS) $(D_CFLAGS) -o $(D_BINARY_DIR)/$(TARGET) $^ $(LDFLAGS) \
	  && printf "[\033[32mLD\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m\n"

$(R_BINARY_DIR)/$(TARGET): $(R_OBJECTS)
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@) \
	  && printf "[\033[34mMKDIR\033[0m] $(dir $@)\n"; fi
	@$(C) $(CFLAGS) $(R_CFLAGS) -o $(R_BINARY_DIR)/$(TARGET) $^ $(LDFLAGS) \
	  && printf "[\033[32mLD\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m\n"

# Running targets

release: $(R_BINARY_DIR)/$(TARGET)
debug: $(D_BINARY_DIR)/$(TARGET)

test: $(T_BINARY_DIR)/$(TEST_TARGET)
	@printf "[\033[34mRUN\033[0m] $(T_BINARY_DIR)/$(TEST_TARGET)\n"
	@cd $(T_BINARY_DIR); ./$(TEST_TARGET) $(TEST_FLAGS)

debug_run: debug
	@printf "[\033[34mRUN\033[0m] $(D_BINARY_DIR)/$(TARGET)\n"
	@cd $(D_BINARY_DIR); ./$(TARGET) $(RUNFLAGS)

release_run: release
	@printf "[\033[34mRUN\033[0m] $(R_BINARY_DIR)/$(TARGET)\n"
	@cd $(R_BINARY_DIR); ./$(TARGET) $(RUNFLAGS)

run: release_run

# Miscellaneous tasks

gdb: debug
	@printf "[\033[34mGDB\033[0m] $(D_BINARY_DIR)/$(TARGET)\n"
	@cd $(D_BINARY_DIR); $(GDB) $(GDBFLAGS) --args ./$(TARGET) $(RUNFLAGS)

valgrind: debug
	@printf "[\033[34mVALGRIND\033[0m] $(D_BINARY_DIR)/$(TARGET)\n"
	@cd $(D_BINARY_DIR); $(VALGRIND) $(VALGRINDFLAGS) ./$(TARGET) $(RUNFLAGS)

compile_commands:
	-@if [ -f compile_commands.json ]; then rm compile_commands.json > /dev/null \
	  && printf "[\033[34mRM\033[0m] compile_commands.json\n"; fi
	@printf "[" >> compile_commands.json
	@printf "%s\0" $(SRC) $(TEST_SRC) | xargs -0 -I "{}" printf "\n  {\n    \"directory\": \"$(shell pwd)\",\n    \"file\": \"{}\",\n    \"command\": \"$(C) $(CFLAGS) -I$(INCLUDE_DIR) -c {} $(LDFLAGS)\"\n  }," >> compile_commands.json
	@sed -i "$$ s/.$$//" compile_commands.json
	@printf "\n]" >> compile_commands.json
	@printf "[\033[34mOUT\033[0m] compile_commands.json\n"

clean:
	-@if [ -d "$(COMPILE_DIR)" ]; then rm -rfv $(COMPILE_DIR) > /dev/null \
	  && printf "[\033[34mRM\033[0m] $(COMPILE_DIR)/\n"; fi
	-@if [ -d "doc" ]; then rm -rfv doc > /dev/null \
	  && printf "[\033[34mRM\033[0m] doc/\n"; fi
