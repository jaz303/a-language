CC			= gcc
CFLAGS		= -Iinclude -Wall --std=c99 
LDFLAGS		=

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

BUILD_DIR	=	build
SCANNER		=	src/scanner.c
PARSER		=	src/parser.c

OBJECTS		=	src/ast.o \
				src/debug.o \
				src/global.o \
				src/hash.o \
				src/parser.o \
				src/scanner.o \
				src/token_names.o

default: build/menace

obj: $(OBJECTS)

scanner: src/scanner.c

src/scanner.c: src/scanner.leg
	leg $< > $@

src/parser.c: src/parser.leg
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
	rm -f $(PARSER)
	rm -rf $(BUILD_DIR)

# Tests

test/hash_test: src/token_names.o src/ast.o test/hash_test.o src/hash.o src/global.o
	gcc -o $@ $(LDFLAGS) src/token_names.o src/ast.o test/hash_test.o src/hash.o src/global.o

test: test/hash_test
	@./test/hash_test



# 
# build/test_parser: all build src/tests/test_parser.o
# 	gcc -o $@ $(LDFLAGS) src/tests/test_parser.o $(OBJECTS)
# 
# build/test_scanner_repl: all build src/tests/test_scanner_repl.o
# 	gcc -o $@ $(LDFLAGS) src/tests/test_scanner_repl.o $(OBJECTS)
# 
# test_parser: build/test_parser
# 	@./test/parsing/test_all