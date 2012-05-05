CC			= gcc
CFLAGS		= -Iinclude -Wall --std=c99 -fnested-functions
LDFLAGS		=

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

BUILD_DIR	=	build
SCANNER		=	src/scanner.c

OBJECTS		=	src/ast.o \
				src/debug.o \
				src/global.o \
				src/scanner.o \
				src/token_names.o

default: build/menace

obj: $(OBJECTS)

scanner: src/scanner.c

src/scanner.c: src/scanner.leg
	leg $< > $@

build:
	mkdir -p $@

all: obj

# Binaries

build/menace: all build
	$(CC) -o build/main $(LDFLAGS) $(OBJECTS)

# Cleanup

clean:
	find . -name '*.o' -delete
	rm -f $(SCANNER)
	rm -rf $(BUILD_DIR)

# Tests

build/test_parser: all build src/tests/test_parser.o
	gcc -o $@ $(LDFLAGS) src/tests/test_parser.o $(OBJECTS)

test_parser: build/test_parser
	@./test/parsing/test_all

build/test_scanner_repl: all build src/tests/test_scanner_repl.o
	gcc -o $@ $(LDFLAGS) src/tests/test_scanner_repl.o $(OBJECTS)
