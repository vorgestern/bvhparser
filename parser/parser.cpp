
#include <string>
#include <parser.h>
#include <parsercontext.h>
#include "bvhconv.h"

using namespace std;

int xxlex();

parsercontext PCX;
extern FILE*xxin;

void yyerror(char const*s)
{
    fprintf(stderr, "\n[Line linenum %d] >>>>>> %s\n", PCX.linenum, s);
}

BVHScene*parse(string_view filename, bool mix)
{
    // Lexer and parser have to handle linefeeds of both descriptions
    // anyway to work on Linux and Windows, so open the file explicitly in binary mode.
    if (!filename.empty())
    {
        xxin=fopen(filename.data(), "rb");
        if (xxin==nullptr)
        {
            fprintf(stderr, "Cannot open/read '%s'\n", filename.data());
            exit(1);
        }
    }
    PCX.Scene=new BVHScene;
    PCX.framenum=0;
    PCX.framesep=0;
    PCX.jointlevel=0;
    PCX.linenum=0;
    PCX.mylog=false;
    PCX.nextchannel=0;
    PCX.mixlexeroutput=mix;
    const int rc=yyparse();
    if (!filename.empty() && xxin!=nullptr)
    {
        fclose(xxin);
        xxin=nullptr;
    }
    if (rc==0)
    {
        auto X=PCX.Scene;
        PCX.Scene=nullptr;
        return X;
    }
    else
    {
        auto X=PCX.Scene;
        PCX.Scene=nullptr;
        delete X;
        return nullptr;
    }
}

static void dumptoken(int u)
{
    switch (u)
    {
#define I(s) case s: { printf("\n%d %s %d", s, #s, yylval.I); break; }
#define II(s) case s: { printf("\n%d %s", s, #s); break; }
#define D(s) case s: { printf("\n%d %s %g", s, #s, yylval.D); break; }
#define S(s) case s: { printf("\n%d %s %s", s, #s, yylval.S); break; }
        II(ASTERISK)
            II(OPENBRACE) II(CLOSEBRACE) II(OPENPAREN) II(CLOSEPAREN)
            I(INT)
            S(STRING)
            D(FLOAT) D(FRAMETIME)

            II(HIERARCHY) II(ENDSITE) II(OFFSET) II(MOTION)
            II(XPOSITION) II(YPOSITION) II(ZPOSITION)
            II(XROTATION) II(YROTATION) II(ZROTATION)
            I(CHANNELS) I(FRAMES)
            S(ROOT) S(JOINT)

            I(TABLELINE)
    }
}

// Flex will not create yylex() directly, because we used --prefix=xx to set the name to xxlex
// and provide this wrapper for the parser. 
int yylex()
{
    int u=xxlex();
    if (PCX.mixlexeroutput) dumptoken(u);
    return u;
}

void lexdump(string_view filename)
{
    if (!filename.empty())
    {
        xxin=fopen(filename.data(), "rb");
        if (xxin==nullptr)
        {
            fprintf(stderr, "Cannot open/read '%s'\n", filename.data());
            exit(1);
        }
    }
    PCX.Scene=new BVHScene;
    PCX.framenum=0;
    PCX.framesep=0;
    PCX.jointlevel=0;
    PCX.linenum=0;
    PCX.mylog=false;
    PCX.nextchannel=0;
    PCX.mixlexeroutput=false;
    int u=1;
    while (u!=0)
    {
        u=xxlex();
        dumptoken(u);
    }
    delete PCX.Scene;
    PCX.Scene=nullptr;
}

extern "C" int xxwrap(){ return 1; }

void ateof(){}
