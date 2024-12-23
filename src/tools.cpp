
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include "bvhhelp.h"
#include "bvhconv.h"
#include "parsercontext.h"

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

double lextableline[1000];

int scanline(const char pad[])
{
  int numfloat=0, offset=0, nr=0;
  // printf("\nscanline: ");
  while (1==sscanf(pad+offset, "%lg%n", lextableline+numfloat, &nr))
  {
    // printf(" %g", lextableline[numfloat]);
    offset+=nr;
    numfloat++;
  } 
  return numfloat;
}

double*motiontable=nullptr;
int tablelinesfilled=0;
static unsigned tablecolumns=0;

void storetableline(unsigned columns)
{
  if (tablelinesfilled==0 && PCX.framenum>0)
  {
    if (tablecolumns==0)
    {
      tablecolumns=channelsused();
      PCX.totaltime=PCX.framenum-1*PCX.framesep;
    }
    if (tablecolumns>0) motiontable=new double[PCX.framenum*tablecolumns];
  }

  if (tablecolumns>0)
  {
    double*T=motiontable+tablelinesfilled*tablecolumns;
    memcpy(T, lextableline, (columns<=tablecolumns?columns:tablecolumns)*sizeof(double));
    for (unsigned c=columns; c<tablecolumns; c++) T[c]=0;

    #if 0
    printf("\nTable line %u:", tablelinesfilled);
    for (unsigned n=0; n<tablecolumns; n++) printf(" %g", T[n]);
    #endif

    tablelinesfilled++;
  }
}

void getmotiontable(const double**table, unsigned*lines, unsigned*columns)
{
  *table=motiontable;
  *lines=tablelinesfilled;
  *columns=tablecolumns;
}
