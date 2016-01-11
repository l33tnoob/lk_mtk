# ----------------------------------------------
# commands
# ----------------------------------------------
C++     := g++
CFLAGS  := -m64 -c
LINK    := g++
LFLAGS  := -m64 -pthread -o
LIBFLAGS := -Wl,-rpath,./

# ----------------------------------------------
# directories
# ----------------------------------------------
OBJ_DIR := Obj
TARGET_DIR := Out
INC_DIR := \
	CheckSum_Generate/Inc \
	MTK/Brom
SRC_DIR := \
	CheckSum_Generate/Src
# ----------------------------------------------
# files
# ----------------------------------------------
TARGET  := $(TARGET_DIR)/CheckSum_Gen
SRCS	:= $(wildcard $(SRC_DIR)/*.cpp)
INCS	:= $(addprefix -I , $(INC_DIR))
OBJS	:= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
LIBS	:= \
	MTK/Lib/libflashtool.so

# ----------------------------------------------
# make
# ----------------------------------------------

.PHONY: new r clean copy prepare ok

new: clean r copy	

r: prepare $(TARGET) ok

$(TARGET): $(OBJS) $(LIBS)
	$(LINK) $(LFLAGS) $(TARGET) $(OBJS) $(LIBS) $(LIBFLAGS)
$(OBJS): $(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(C++) $(CFLAGS) $(INCS) $< -o $@
		
prepare:
	@-mkdir $(OBJ_DIR)
	@-mkdir $(TARGET_DIR)

clean:
	@-rm -rf $(OBJ_DIR)
	@-rm -rf $(TARGET_DIR)

copy:
	@cp $(LIBS) $(TARGET_DIR)
	@chmod +x $(TARGET)
    
ok:
	@echo =======================
	@echo ------ Success --------
	@echo =======================