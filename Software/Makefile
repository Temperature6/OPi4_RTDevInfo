INCLUDE_PATHS   := ./inc
SOURCE_PATHS    := ./src
OBJECT_DIR      := ./obj
LIBS_NAME       := wiringPi \
					pthread
TARGET_FILE     := ./RTDevInfo
CXX_FLAGS       := -c


CXX_FILES       := $(shell find $(SOURCE_PATHS)/*.c)
OBJ_FILES       := $(patsubst $(SOURCE_PATHS)/%.c,$(OBJECT_DIR)/%.o,$(CXX_FILES))
LIBS            := $(LIBS_NAME:%=-l%)
INCLUDES        := $(INCLUDE_PATHS:%=-I%)

$(OBJECT_DIR)/%.o : $(SOURCE_PATHS)/%.c
	@mkdir -p $(dir $@)
	@gcc $(CXX_FLAGS) $^ $(INCLUDES) -o $@

compile : $(OBJ_FILES)

$(TARGET_FILE) : $(OBJ_FILES)
	@gcc $(OBJ_FILES) $(LIBS) -o $@

build: $(TARGET_FILE)

run : $(TARGET_FILE)
	@$(TARGET_FILE)

clean :
	@rm -rf $(OBJECT_DIR)
	@rm -f $(TARGET_FILE)

.PHONY : build compile run clean