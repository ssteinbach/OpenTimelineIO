# Makefile for opentime
CC = clang

all: test_ot opentime

opentime: opentime.c
	$(CC) -shared -o libopentime.so -fPIC opentime.c -lm

test_ot: opentime test_opentime.c
	$(CC) -o test_ot -L. -lopentime test_opentime.c 

python: opentime test_opentime.py setup.py
	python2 setup.py build && env PYTHONPATH=build/lib.linux-x86_64-2.7 LD_LIBRARY_PATH=. python2 test_opentime.py --failfast

clean:
	rm libopentime.so test_ot
	rm -rf build
