FMDynamic_Path = ./FMDynamic/
LibFlowMatrix_Path = ./libFlowMatrix/
FMCuSparseLinAlg_Path = ./FMCuSparseLinAlg/
FMTrace_path = ./trace/FMtrace/

all: FMDynamic
.PHONY: clean FMDynamic  LibFlowMatrix

LibFlowMatrix:

FMDynamic:
	mkdir -p ./bin
	(cd $(LibFlowMatrix_Path) && make -j)
	(cd $(FMDynamic_Path) && make -j)
	sync -f $(FMDynamic_Path)/bin/QueryCLI
	cp $(FMDynamic_Path)/bin/QueryCLI ./bin
	sync -f $(FMDynamic_Path)/bin/TraceLoader	
	cp $(FMDynamic_Path)/bin/TraceLoader ./bin


clean:
	rm -rf ./bin
	(cd $(FMDynamic_Path) && make clean)
	(cd $(LibFlowMatrix_Path) && make clean)
	(cd $(FMCuSparseLinAlg_Path) && make clean)
	(cd $(FMTrace_path) && make clean)

