AR ?= ar
ARFLAGS ?= -rcs
RANLIB ?= ranlib

CUDA_TOOLKIT := $(shell dirname $$(command -v nvcc))/..
INC          += -I$(CUDA_TOOLKIT)/include -I./include
LIBS         += -lcudart -lcusparse
CFLAGS		 ?= -O3 -g -G -Wno-deprecated-declarations

SRC_PATH = ./src/

OBJ := $(notdir $(patsubst %.cu,%.o,$(wildcard $(SRC_PATH)*.cu)))
TARGET := $(notdir $(patsubst %.cu,%,$(wildcard testsuite/*.cu)))
TESTS := testsuite/spgemm_test testsuite/spgeam_test testsuite/coo_csr_convert_test testsuite/main_diag_test
STATIC_LIB := libfmlinalg.a
all: $(TESTS) $(STATIC_LIB)

%.o: $(SRC_PATH)/%.cu
	nvcc $(INC) $(CFLAGS) -c $<

testsuite/%.o: testsuite/%.cu
	nvcc $(INC) $(CFLAGS) -c $< -o $@

testsuite/%: testsuite/%.o $(OBJ)
	nvcc $(INC) $^ -o $@ $(LIBS)

$(STATIC_LIB): $(OBJ)
	mkdir -p ./lib
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@
	mv $@ ./lib

clean:
	rm -f $(TESTS) 
	$(RM) -rf ./lib
	rm -f *.o
	rm -f testsuite/*.o

test:
	@echo "\n==== 1. SpGEMM Test ====\n"
	@./testsuite/spgemm_test
	@echo "\n==== 2. SpGEAM Test ====\n"
	@./testsuite/spgeam_test
	@echo "\n==== 3. COO&CSR Conversion Test ====\n"
	@./testsuite/coo_csr_convert_test
	@echo "\n==== 4. Set Main Diag Test ====\n"
	@./testsuite/main_diag_test
	
.PHONY: clean test