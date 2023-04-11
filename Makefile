CC       ?= clang
CFLAGS   ?= -O3 -g -flto=thin -std=c2x -Wall -Wextra -Wno-unused-function -Winline
DBGFLAGS ?=

BIN_PATH       := out
OBJ_PATH       := out/obj
SRC_PATH       := src

TARGET         := $(BIN_PATH)/scheme

SRC            := $(foreach x, ${SRC_PATH}, $(wildcard $(addprefix ${x}/*,.c*)))
OBJ            := $(addprefix ${OBJ_PATH}/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
DEP            := $(addprefix ${OBJ_PATH}/, $(addsuffix .d, $(notdir $(basename $(SRC)))))

CLEAN_LIST     := $(OBJ) $(OBJ_PATH)
CLEANALL_LIST  := $(TARGET) $(CLEAN_LIST) $(BIN_PATH)

# depend on the makefile too
.EXTRA_PREREQS:= $(abspath $(lastword $(MAKEFILE_LIST)))

default: makedir all

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS)

-include $(DEP)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -rf $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEANALL_LIST)
	@rm -rf $(CLEANALL_LIST)
