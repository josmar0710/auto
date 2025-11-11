.PHONY: default build configure run clean

default: build

build: configure
	ninja -C build -j $(shell nproc) --verbose

configure:
	cmake -S . -B build \
	-G "Ninja" \
	-D CMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-D CMAKE_CXX_COMPILER=clang++ \
	-D CMAKE_BUILD_TYPE=Debug 

run: build
	./build/auto

clean:
	rm -rf build
