
#include <filesystem>
#include <bvhhelp.h>
#include <parsercontext.h>

using namespace std;
using fspath=filesystem::path;

void dumphumanoid_bb(const Hierarchy&);
void dumphumanoid_txt(const Hierarchy&);

// https://github.com/g-truc/glm.git
// https://en.wikipedia.org/wiki/Biovision_Hierarchy
// http://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/Example1.bvh

static enum class OutputType {ffregular, ffmix, fflexeroutput, fftext, ffboundingbox} func=OutputType::ffregular;
static OutputOptions Opts {0, SegmentForms::cylinder, true, true};

int yyparse();
extern FILE*xxin;

static BVHScene MyScene;
parsercontext PCX {&MyScene};

int xxlex();

// Flex will not create yylex() directly, because we used --prefix=xx to set the name to xxlex
// and provide this wrapper for the parser. 
int yylex()
{
    int u=xxlex();
    if (func==OutputType::ffmix) dumptoken(u);
    return u;
}

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

    // Lexer and parser have to handle linefeeds of both descriptions
    // anyway to work on Linux and Windows, so open the file explicitly in binary mode.
    if (!fninput.empty())
    {
        xxin=fopen(fninput.string().c_str(), "rb");
        if (xxin==nullptr)
        {
            fprintf(stderr, "Cannot open/read '%s'\n", fninput.string().c_str());
            exit(1);
        }
    }

    int rc=1;

    switch (func)
    {
        case OutputType::ffregular:
        case OutputType::ffmix:
        case OutputType::fftext:
        {
            rc=yyparse();
            if (rc==0)
            {
                Opts.totaltime=PCX.framesep*(PCX.framenum+1);
                switch (func)
                {
                    case OutputType::fftext: dumphumanoid_txt(PCX.Scene->H); break;
                    case OutputType::ffboundingbox: dumphumanoid_bb(PCX.Scene->H); break;
                    default: output_x3d(PCX.Scene->H, PCX.Scene->M, Opts); break;
                }
            }
            break;
        }

        case OutputType::fflexeroutput:
        {
            int u=1;
            while (u!=0)
            {
                u=xxlex();
                dumptoken(u);
            }
            rc=0;
            break;
        }
    }

    if (!fninput.empty() && xxin!=nullptr) fclose(xxin);

    return rc;
}

extern "C" int xxwrap(){ return 1; }

void yyerror(char const*s)
{
    fprintf(stderr, "\n[Line linenum %d] >>>>>> %s\n", PCX.linenum, s);
}

void ateof(){}
