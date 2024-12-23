
XFILES := hanimjoint main tools x3d euler_to_axisangle
CPPFLAGS := -I src
CXXFLAGS := -std=c++20 -Wall -Werror

.PHONY: all clean

all: dirs bvhparser
clean:
	rm -rf b ./bvhparser
dirs:
	mkdir -p b

b/bison-bvhconv.cpp: src/bvhconv.y
	bison --defines=b/bvhconv.h --output=$@ -Wcounterexamples $<

b/lex-bvhconv.cpp: src/bvhconv.lex
	flex -8 --nounistd --prefix=xx -t > $@ $<

b/%.o: src/%.cpp src/bvhhelp.h
	g++ $(CXXFLAGS) -Werror $(CPPFLAGS) -c $< -o $@

b/%.o: b/%.cpp src/bvhhelp.h
	g++ $(CXXFLAGS) -Wno-unused-function $(CPPFLAGS) -c $< -o $@

bvhparser: $(XFILES:%=b/%.o) b/bison-bvhconv.o b/lex-bvhconv.o
	g++ -o $@ $^
