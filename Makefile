CXXFLAGS=-std=c++17 -g -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

9cc: $(OBJS)
	$(CXX) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

fmt:
	find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o tmp*

.PHONY: test clean fmt
