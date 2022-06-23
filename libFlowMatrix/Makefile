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
PROJ_HOME := .
LIB_PATH := $(PROJ_HOME)/lib
SRC_PATH := $(PROJ_HOME)/src
INC_PATH := $(PROJ_HOME)/include
TEST_SRC_PATH := $(PROJ_HOME)/testsuite
PROTOBUF_PATH := $(PROJ_HOME)/src/
FMTRACE_PATH := $(PROJ_HOME)/../trace/FMtrace/
FMTRACE_INC_PATH := $(FMTRACE_PATH)/include

# Compilation variables
OPT ?= -O3 -g #-O0 
WARNINGS = -Wall -Wextra
CPP_STD = -std=c++17
PIC_FLAG ?= -fPIC
CFLAGS ?= $(OPT) $(WARNINGS) $(CPP_STD) $(PIC_FLAG)
INC += -I$(INC_PATH) -I$(PROJ_HOME) -I$(FMTRACE_INC_PATH)

# Linking variables
LDLIBS += -lcapstone -lprotobuf

# Lib flags
# SOLIB_FLAGS ?= -Wl,-soname,libfmtrace.so

# Target
OBJ := $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRC_PATH)/*.cpp)))
test_bin := $(TEST_SRC_PATH)/vstate_init $(TEST_SRC_PATH)/vstate_fmtrace 
libfm_static_lib :=  $(LIB_PATH)/libfm.a
libfmtrace_static_lib := $(FMTRACE_PATH)/lib/libfmtrace.a

test: acorns_obj.pb.o $(test_bin) $(libfm_static_lib)

all : test 

acorns_obj.pb.o: $(PROTOBUF_PATH)/acorns_obj.pb.cc
	$(CPPC) -c $(strip $(CFLAGS) $(INC) $^ $(LDLIBS))

$(PROTOBUF_PATH)/acorns_obj.pb.cc: $(PROTOBUF_PATH)/acorns_obj.proto
	@-$(CP) $^ ./
	$(PROTOC) *.proto --cpp_out=$(SRC_PATH)
	@-$(MV) $(SRC_PATH)/acorns_obj.pb.h $(INC_PATH)
	@-$(RM) *.proto

$(libfm_static_lib): $(OBJ) acorns_obj.pb.o
	@-$(MKDIR) -p $(LIB_PATH)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

$(TEST_SRC_PATH)/vstate_init: $(OBJ) 
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $(TEST_SRC_PATH)/vstate_init.cpp
	$(CPPC) -o $@ $(strip $(CFLAGS) $^ vstate_init.o $(LDLIBS))

$(TEST_SRC_PATH)/vstate_fmtrace: $(OBJ) $(libfmtrace_static_lib)
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $(TEST_SRC_PATH)/vstate_fmtrace.cpp
	$(CPPC) -o $@ $(strip $(CFLAGS) $(OBJ) vstate_fmtrace.o $(libfmtrace_static_lib) $(LDLIBS))

%.o: $(SRC_PATH)/%.cpp $(PROTOBUF_PATH)/acorns_obj.pb.cc
	$(CPPC) $(strip $(CFLAGS) $(INC) -c) $<

$(libfmtrace_static_lib):
	(cd $(FMTRACE_PATH) && make)


.PHONY: clean
clean:
	$(RM) *.o *.gch
	$(RM) -rf ./bin
	$(RM) -rf ./lib
	$(RM) -rf $(test_bin)
	$(RM) -rf $(PROTOBUF_PATH)/acorns_obj.pb.cc
	$(RM) -rf $(INC_PATH)/acorns_obj.pb.h

