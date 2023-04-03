CC       ?=
CFLAGS   ?=
DBGFLAGS ?=

BIN_PATH       := out
OBJ_PATH       := out/obj
SRC_PATH       := src

TARGET         := $(BIN_PATH)/scheme

SRC            := $(foreach x, ${SRC_PATH}, $(wildcard $(addprefix ${x}/*,.c*)))
OBJ            := $(addprefix ${OBJ_PATH}/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

CLEAN_LIST     := $(OBJ) $(OBJ_PATH)
CLEANALL_LIST  := $(TARGET) $(CLEAN_LIST) $(BIN_PATH)

default: makedir all

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CFLAGS) -c -o $@ $<

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
