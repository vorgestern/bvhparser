
CPPFLAGS := -I include
CXXFLAGS := -std=c++20 -Wall -Werror -Wno-unused-function -Wno-unused-but-set-variable

.PHONY: all clean dirs

all: dirs xxbvhtox3d xxbvhshow
clean:; rm -rf b ./bvhparser.a
dirs:; mkdir -p b/bvhparser b/bvhtox3d b/bvhshow

# ======================================================================

PFILES := parser parsercontext utilities hanimjoint
bvhparser.a: $(PFILES:%=b/bvhparser/%.o) b/bvhparser/bison-bvhconv.o b/bvhparser/lex-bvhconv.o
	@echo $@
	@ar -crs $@ $^
b/bvhparser/bison-bvhconv.cpp b/bvhparser/bvhconv.h: src/parser/bvhconv.y
	@echo $<
	@bison --defines=b/bvhparser/bvhconv.h --output=b/bvhparser/bison-bvhconv.cpp -Wcounterexamples $<
b/bvhparser/lex-bvhconv.cpp: src/parser/bvhconv.l b/bvhparser/bvhconv.h
	@echo $<
	@flex -8 --nounistd --prefix=xx -t > $@ $<
b/bvhparser/%.o: src/parser/%.cpp src/parser/parsercontext.h b/bvhparser/bvhconv.h
	@echo $<
	@g++ $(CXXFLAGS) $(CPPFLAGS) -I src/parser -I b/bvhparser -c $< -o $@
b/bvhparser/%.o: b/bvhparser/%.cpp src/parser/parsercontext.h b/bvhparser/bvhconv.h
	@echo $<
	@g++ $(CXXFLAGS) $(CPPFLAGS) -I src/parser -I b/bvhparser -c $< -o $@

# ======================================================================

AFILES := main bbox output x3d
xxbvhtox3d: $(AFILES:%=b/bvhtox3d/%.o) bvhparser.a
	g++ -o $@ $^
b/bvhtox3d/%.o: bvhtox3d/%.cpp src/parser/parsercontext.h b/bvhparser/bvhconv.h
	@echo $<
	@g++ $(CXXFLAGS) $(CPPFLAGS) -I b/bvhparser -I bvhtox3d -c $< -o $@

# ======================================================================

SFILES   := bvhshow glhelp modelwindow
MYLDFLAGS := -L/home/josef/source/fltk-1.4.1/bb/lib -L/usr/lib/x86_64-linux-gnu -Wl,-rpath,/home/josef/source/fltk-1.4.1/bb/lib
MYCPPFLAGS := -I/usr/include/cairo -I/home/josef/source/fltk-1.4.1/bb -I/home/josef/source/fltk-1.4.1
b/bvhshow/%.o: bvhshow/%.cpp bvhshow/bvhshow.h include/parser.h
	@echo $<
	@g++ $(CXXFLAGS) $(CPPFLAGS) $(MYCPPFLAGS) -I b/bvhparser -I bvhshow  -c $< -o $@
xxbvhshow: $(SFILES:%=b/bvhshow/%.o) bvhparser.a
	g++ -o $@ $^ $(MYLDFLAGS) -lfltk -lfltk_forms -lfltk_gl -lfltk_images -lGL -lGLU -lGLEW -lX11 -lcairo

#  -lfltk_jpeg -lfltk_png -lfltk_z
#  -lfltk_gl -lfltk_forms

# fltk-config
