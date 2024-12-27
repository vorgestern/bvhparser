
#include <string>
#include "bvhconv.h"

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
