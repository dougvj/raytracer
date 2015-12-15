
WARNINGS= -Wall -Werror -Wfatal-errors
CWARNINGS=
CXXWARNINGS=
INCLUDE=
CFLAGS= -MMD -std=gnu99 $(WARNINGS) $(CWARNINGS) $(INCLUDE) -g
CXXFLAGS = -MMD $(WARNINGS) $(CXXWARNINGS) $(INCLUDE)

LDFLAGS= -lpthread -lm -L./libdatastruct -ldatastruct

ifdef MSYSTEM
LDFLAGS+= -mwindows
else
LDFLAGS+= 
endif

EXECUTABLE=raytrace

#compilers

CC=gcc
CXX=g++



SRC=$(wildcard *.c)
OBJECTS = $(SRC:%.c=%.o)

CPPSRC=$(wildcard *.cpp)
CPPOBJECTS = $(CPPSRC:%.cpp=%.o)

all: $(OBJECTS) $(CPPOBJECTS)
	$(CXX) $(OBJECTS) $(CPPOBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

cleanDebug: clean Debug

linecount:
	cat *.cpp *.c *.h | wc -l

clean:
	rm *.o
	rm *.d
	rm $(EXECUTABLE)
	rm test/unit-test

set_debug:
	$(eval DEBUG=1)

set_clang:
	$(eval CLANG=1)

Debug: set_debug all

clang: set_clang all

Release: all

$(EXECUTABLE): $(OBJECTS) $(CPPOBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@


set_test:
	$(eval CFLAGS+=-DUNIT_TESTS)



ifdef DEBUG
CFLAGS+= -g
CXXFLAGS+= -g
else
CFLAGS+= -O3
CXXFLAGS+= -O3
endif

ifdef CLANG
CC=clang
CXX=clang
endif

Test: set_test $(OBJECTS) unit-test.o test.o
	$(CC) $(OBJECTS) unit-test.o test.o $(LDFLAGS) -o ./test/unit-test

test.o: ./test/test.c
	$(CC) -c -I./ $(CFLAGS) $< -o $@

unit-test.o: ./test/unit-test.c
	$(CC) -c -I./ $(CFLAGS) $< -o $@



-include $(SRC:%.c=%.d) 
-include $(CPPSRC:%.cpp=%.d)

