# Command ard arguments
CC ?= gcc
CPPC ?= g++
RM ?= rm
MKDIR ?= mkdir
AR ?= ar
ARFLAGS ?= -rcs # ar needs the dash on OpenBSD
RANLIB ?= ranlib


# Path
PROJ_HOME := .
BIN_PATH := $(PROJ_HOME)/bin
LIB_PATH := $(PROJ_HOME)/lib
SRC_PATH := $(PROJ_HOME)/src
INC_PATH := $(PROJ_HOME)/include


# Compilation variables
OPT ?= -O3
WARNINGS = -Wall -Wextra
CPP_STD = -std=c++17
PIC_FLAG ?= -fPIC
CFLAGS ?= $(OPT) $(WARNINGS) $(CPP_STD) $(PIC_FLAG)
INC += -I$(PROJ_HOME)/include 

# Linking variables
LDLIBS += -lcapstone

# Lib flags
SOLIB_FLAGS ?= -Wl,-soname,libfmtrace.so

# Target
PROG := $(BIN_PATH)/FMTraceReader

OBJ := $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRC_PATH)/*.cpp)))

all : libfmtrace.so libfmtrace.a $(BIN_PATH)/FMTraceReader clean_tmp

libfmtrace.so: $(filter-out FMTraceReader.o,$(OBJ))
	@-$(MKDIR) -p $(LIB_PATH)
	$(CPPC) -shared $(SOLIB_FLAGS) -o $(LIB_PATH)/$@ $(strip $(CFLAGS) $(LDLIBS) $^)

libfmtrace.a: $(filter-out FMTraceReader.o,$(OBJ))
	@-$(MKDIR) -p $(LIB_PATH)
	$(AR) $(ARFLAGS) $(LIB_PATH)/$@ $^
	$(RANLIB) $(LIB_PATH)/$@

$(BIN_PATH)/FMTraceReader: $(OBJ)
	@-$(MKDIR) -p $(BIN_PATH)
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDLIBS))

%.o: $(SRC_PATH)/%.cpp
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $<


.PHONY: clean create_folder clean_tmp
create_folder:
	@-$(MKDIR) -p $(BIN_PATH)
	@-$(MKDIR) -p $(LIB_PATH)
clean:
	$(RM) *.o *.gch
	$(RM) -rf ./bin
	$(RM) -rf ./lib
clean_tmp:
	@-$(RM) *.o *.gch

