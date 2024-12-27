
#include <bvhhelp.h>
#include <filesystem>

using namespace std;
using fspath=filesystem::path;

// https://github.com/g-truc/glm.git
// https://en.wikipedia.org/wiki/Biovision_Hierarchy
// http://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/Example1.bvh

static enum class OutputType {ffregular, ffmix, fflexeroutput, fftext, ffboundingbox} func=OutputType::ffregular;
static OutputOptions Opts {SegmentForms::cylinder, true, true};

int main(int argc, const char*argv[])
{
    fspath fninput {};

    for (int a=1; a<argc; a++)
    {
        const string_view arg=argv[a];
        if (arg=="-lex") func=OutputType::fflexeroutput;   // Output lexer results
        else if (arg=="-mix") func=OutputType::ffmix;      // Mix output from lexer and parser.
        else if (arg=="-t") func=OutputType::fftext;
        else if (arg=="-b") func=OutputType::ffboundingbox;
        else if (arg=="-f" && a<argc-1) fninput=argv[++a];
        else if (arg=="-s0") Opts.segmentshape=SegmentForms::none;
        else if (arg=="-s1") Opts.segmentshape=SegmentForms::line;
        else if (arg=="-s2") Opts.segmentshape=SegmentForms::cylinder;
        else if (arg[0]!='-') fninput=arg;
    }

    if (func==OutputType::fflexeroutput)
    {
        lexdump(fninput.string());
    }
    else
    {
        BVHScene*Scene=parse(fninput.string(), func==OutputType::ffmix);
        if (Scene!=nullptr)
        {
            switch (func)
            {
                case OutputType::fftext: dumphumanoid_txt(Scene->H); break;
                case OutputType::ffboundingbox: dumphumanoid_bb(Scene->H); break;
                default:
                case OutputType::ffregular: output_x3d(Scene->H, Scene->M, Scene->totaltime, Opts); break;
            }
            delete Scene;
        }
    }

    return 0;
}

extern "C" int xxwrap(){ return 1; }

void ateof(){}
