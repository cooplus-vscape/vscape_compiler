current_dir = $(shell pwd)

LLVM_HOME = $(current_dir)/../
LLVM_HOME_OBJ=$(current_dir)/build-coop

BUILD_TYPE=Release
CFLAGS=-v -g
CXXFLAGS=-v -g

build: $(LLVM_HOME_OBJ)/Makefile
	make  -j20 -C $(LLVM_HOME_OBJ) verbose=1

$(LLVM_HOME_OBJ)/Makefile:
	mkdir -p $(LLVM_HOME_OBJ) && \
	cd $(LLVM_HOME_OBJ) && \
	cmake  -DCMAKE_BUILD_TYPE:STRING="$(BUILD_TYPE)" -DCMAKE_CXX_FLAGS="$(CXXFLAGS)" -DCMAKE_C_FLAGS="$(CFLAGS)" $(LLVM_HOME) && \
	cd -
