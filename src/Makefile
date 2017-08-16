
all: libOTIO.a OTIO_test

clean:
	rm libOTIO.a OTIO_io.o OTIO_main.o assemble_json.o OTIO_test

libOTIO.a: OTIO_io.c OTIO_main.c OTIO_internal.h assemble_json.h assemble_json.c
	cc -c OTIO_io.c OTIO_main.c assemble_json.c
	ar rcs libOTIO.a OTIO_io.o OTIO_main.o assemble_json.o

OTIO_test: OTIO_test.c libOTIO.a
	cc -o OTIO_test OTIO_test.c -L. -lOTIO

test: OTIO_test
	./OTIO_test
