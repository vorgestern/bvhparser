
#include <string_view>
#include <vector>
#include <filesystem>
#include <bvhhelp.h>
#include <parsercontext.h>
#include "bvhconv.h"

using namespace std;
using fspath=filesystem::path;

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

void parsercontext::pushjoint(const char name[])
{
    if (mylog) printf("\n%*sjoint %s {", 2*jointlevel, "", name);
    Scene->H.emplace_back(jointlevel);
    if (name!=nullptr && name[0]!=0) Scene->H.back().setname(name);
    jointlevel++;
}

void parsercontext::popjoint()
{
    jointlevel--;
    if (mylog) printf("\n%*s}", 2*jointlevel, "");
}

void parsercontext::endsite()
{
    if (mylog) printf("\n%*s endsite", 2*jointlevel, "");
}

static unsigned char channelcode(unsigned c)
{
    return c-XPOSITION<3?'X'+c-XPOSITION : c-XROTATION<3?'x'+c-XROTATION : '?';
}

void parsercontext::setcurrentchannels(unsigned c0, unsigned c1, unsigned c2)
{
    if (Scene->H.size()>0) Scene->H.back().setchannels(channelcode(c0),channelcode(c1),channelcode(c2));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void parsercontext::setcurrentchannels(unsigned c0, unsigned c1, unsigned c2, unsigned c3, unsigned c4, unsigned c5)
{
    if (Scene->H.size()>0) Scene->H.back().setchannels(channelcode(c0),channelcode(c1),channelcode(c2),channelcode(c3),channelcode(c4),channelcode(c5));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void parsercontext::setcurrentoffset(double x, double y, double z)
{
    if (Scene->H.size()>0) Scene->H.back().offset={x,y,z};
    else fprintf(stderr, "\nFehler: offsetspec ohne offenen joint");
}

void dumphumanoid_bb(const Hierarchy&);
void dumphumanoid_txt(const Hierarchy&);

void parsercontext::parserfinished()
{
    Opts.totaltime=PCX.framesep*(PCX.framenum+1);
    switch (func)
    {
        case OutputType::fftext: dumphumanoid_txt(Scene->H); break;
        case OutputType::ffboundingbox: dumphumanoid_bb(Scene->H); break;
        default: output_x3d(Scene->H, Scene->M, Opts); break;
    }
}
