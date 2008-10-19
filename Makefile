#all: build/Makefile
#	cd build && make -s
#build/Makefile:

all:
	@mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build && make

install:
	cd build && make install

uninstall:
	cd build && make uninstall

test: all
	cd build && make test

test-future: all
	cd build && make test-future

clean:
	-cd build && make clean
	rm -rf build
