# Command ard arguments
CC ?= gcc
CPPC ?= g++
PROTOC ?= protoc
RM ?= rm
CP ?= cp
MV ?= mv
MKDIR ?= mkdir
AR ?= ar
ARFLAGS ?= -rcs # ar needs the dash on OpenBSD
RANLIB ?= ranlib

# Path
PROJ_HOME := ../
LIBFM_HOME := $(PROJ_HOME)/libFlowMatrix
LIB_PATH := $(LIBFM_HOME)/lib
INC_PATH := $(LIBFM_HOME)/include
FMTRACE_PATH := $(PROJ_HOME)/trace/FMtrace/
FMTRACE_INC_PATH := $(FMTRACE_PATH)/include/
FMTRACE_STATIC_LIB_PATH := $(FMTRACE_PATH)/lib/
FMCUSPARSELINALG_PATH := $(PROJ_HOME)/FMCuSparseLinAlg/
FMCUSPARSELINALG_INC_PATH := $(FMCUSPARSELINALG_PATH)/include/
FMCUSPARSELINALG_STATIC_LIB_PATH := $(FMCUSPARSELINALG_PATH)/lib/
TEST_SRC_PATH := ./testsuite/
SRC_PATH := ./src
CLI_SRC_PATH := $(SRC_PATH)/QueryCLI/
TRACE_LOADER_SRC_PATH := $(SRC_PATH)/TraceLoader/
BIN_PATH := ./bin

# CUDA Related
CUDA_TOOLKIT := $(shell dirname $$(command -v nvcc))/..
INC          += -I$(CUDA_TOOLKIT)/include
LDPATH		 += -L$(CUDA_TOOLKIT)/lib64

# Compilation variables
OPT ?= -O3 -g
WARNINGS = -Wall -Wextra
CPP_STD = -std=c++17
PIC_FLAG ?= #-fPIC
CFLAGS ?= $(OPT) $(WARNINGS) $(CPP_STD) $(PIC_FLAG)
INC += -I$(INC_PATH) -I$(FMTRACE_INC_PATH) -I$(FMCUSPARSELINALG_INC_PATH) -I./ -I./include

# Linking variables
LDLIBS += -lcudart -lcusparse -lcapstone -lprotobuf -lsqlite3 -pthread 
#LDLIBS += $(wildcard $(PROJ_HOME)/libs/*) -pthread -ldl -lrt 

# Target
OBJ := $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRC_PATH)/*.cpp)))
cli_bin := $(patsubst %.cpp,%,$(wildcard $(CLI_SRC_PATH)/*.cpp))
traceload_bin := $(patsubst %.cpp,%,$(wildcard $(TRACE_LOADER_SRC_PATH)/*.cpp))
libfmtrace_static_lib :=  $(FMTRACE_STATIC_LIB_PATH)/libfmtrace.a
libfm_static_lib := $(LIB_PATH)/libfm.a
libfmlinalg_static_lib := $(FMCUSPARSELINALG_STATIC_LIB_PATH)/libfmlinalg.a
PROG := $(BIN_PATH)/QueryCLI $(BIN_PATH)/TraceLoader $(BIN_PATH)/TraceGenerator $(BIN_PATH)/Worker


all : $(PROG)

$(BIN_PATH)/QueryCLI: $(CLI_SRC_PATH)/QueryCLI.o $(OBJ) $(libfmtrace_static_lib) $(libfm_static_lib) $(libfmlinalg_static_lib) 
	@-$(MKDIR) -p $(BIN_PATH)
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDPATH) $(LDLIBS))

$(BIN_PATH)/TraceLoader: $(TRACE_LOADER_SRC_PATH)/TraceLoader.o $(OBJ) $(libfmtrace_static_lib) $(libfm_static_lib) $(libfmlinalg_static_lib) 
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDPATH) $(LDLIBS))

$(BIN_PATH)/TraceGenerator: $(TRACE_LOADER_SRC_PATH)/TraceGenerator.o $(OBJ) $(libfmtrace_static_lib) $(libfm_static_lib) $(libfmlinalg_static_lib) 
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDPATH) $(LDLIBS))

$(BIN_PATH)/Worker: $(CLI_SRC_PATH)/Worker.o $(OBJ) $(libfmtrace_static_lib) $(libfm_static_lib) $(libfmlinalg_static_lib) 
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDPATH) $(LDLIBS))

$(TRACE_LOADER_SRC_PATH)/%: $(TRACE_LOADER_SRC_PATH)/%.o $(OBJ) $(libfmtrace_static_lib) $(libfm_static_lib) $(libfmlinalg_static_lib) 
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ $(LDPATH) $(LDLIBS))

%.o: $(SRC_PATH)/%.cpp 
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $< -o $@

$(CLI_SRC_PATH)/%.o: $(CLI_SRC_PATH)/%.cpp 
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $< -o $@

$(TRACE_LOADER_SRC_PATH)/%.o: $(TRACE_LOADER_SRC_PATH)/%.cpp 
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $< -o $@

$(libfmtrace_static_lib):
	(cd $(FMTRACE_PATH) && make -j)

$(libfm_static_lib):
	(cd $(LIBFM_HOME) && make -j)

$(libfmlinalg_static_lib):
	(cd $(FMCUSPARSELINALG_PATH) && make -j)

clean:
	$(RM) *.o *.gch
	$(RM) -rf ./bin
	(cd $(TRACE_LOADER_SRC_PATH) && $(RM) *.o)
	(cd $(CLI_SRC_PATH) && $(RM) *.o)
	(cd $(FMCUSPARSELINALG_PATH) && make clean)

clean_tmp:
	$(RM) *.o *.gch

.PHONY: clean clean_tmp
