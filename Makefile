 CC=g++ -g3 -std=c++17 -Wall -pedantic -Wconversion -Wextra -Wreorder -fno-builtin

#LDFLAGS:=-I/usr/local/opt/openssl/include/ -L/usr/local/opt/openssl/lib -lssl -lcrypto

## Define certain variables based on system
#ifeq ($(shell uname -s | tr A-Z a-z), darwin)
#	# -I likely means (Include) -L likely means (Library)
#endif
#
#ifeq ($(shello uname -s | tr A-Z a-z), linux)
#	LDFLAGS
#endif

FrameWorkSRC=libraries/unit_test_framework/src
FrameWorkSources=$(wildcard ${FrameWorkSRC}/*.cpp)
FrameWorkOBJS=$(FrameWorkSources:.cpp=.o)

SRC=src
SOURCES=$(wildcard ${SRC}/*.cpp)
OBJS=${SOURCES:.cpp=.o}

all: test

TESTDIR=tests
EXECDIR=tests/bin
OUTPUT=tests/output

TEST_SRC:=$(basename $(wildcard ${TESTDIR}/*.cpp))
$(TEST_SRC): %: %.cpp ${OBJS} ${FrameWorkOBJS}
	@mkdir -p ${EXECDIR}
	@mkdir -p ${OUTPUT}
	${CC} -Dtesting -o  ${EXECDIR}/$(notdir $@) $^ -pthread

test: ${TEST_SRC}

run_test: test
	@echo
	@./bin/run_tests.sh

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $< -o $@
%.o: %.cc
	${CC} -c $<

clean:
	rm -rf ${OBJS} ${EXECDIR}/* ${OUTPUT}/* 