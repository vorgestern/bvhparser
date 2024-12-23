
#include <string>
#include <vector>
#include "bvhhelp.h"
#include "bvhconv.h"
#include "parsercontext.h"

using namespace std;

void dumptoken(int u)
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

static vector<double>MotionLine;
vector<vector<double>>MotionTable;

int scanline(const char pad[])
{
    MotionLine.clear();
    int offset=0, nr=0;
    double value=0;
    while (1==sscanf(pad+offset, "%lg%n", &value, &nr))
    {
        MotionLine.push_back(value);
        offset+=nr;
    } 
    return MotionLine.size();
}

void storetableline(unsigned columns){ MotionTable.push_back(MotionLine); }
