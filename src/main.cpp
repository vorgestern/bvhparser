
#include <stdio.h>
#include <string.h>
#include "bvhhelp.h"
#include "bvhconv.h"

// https://github.com/g-truc/glm.git
// https://en.wikipedia.org/wiki/Biovision_Hierarchy
// http://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/Example1.bvh

static int func=0;
int segmentform=2;
bool floor=false, headlight=false;

int yyparse();
extern FILE*xxin;
extern int linenum;

int jointlevel=0;
int framenum=0;
double framesep=0;
double totaltime=0;

// Flex will not create yylex() directly, because we used -prefix=xx to set the name to xxlex
// and provide this wrapper for the parser. 
int yylex()
{
    int xxlex();
    int u=xxlex();
    if (func==1) dumptoken(u);
    return u;
}

int main(int argc, const char*argv[])
{
    const char*fninput=0L;

    for (int a=1; a<argc; a++)
    {
        if (0==strcmp(argv[a], "-lex")) func=3;               // Output lexer results
        else if (0==strcmp(argv[a], "-mix")) func=1;          // Mix output from lexer and parser.
        else if (0==strcmp(argv[a], "-s")) func=2;            // Output a simple table of variables.
        else if (0==strcmp(argv[a], "-f") && a<argc-1) fninput=argv[++a];
        else if (0==strcmp(argv[a], "-segments") && a<argc-1)
        {
            const char*form=argv[++a];
            if (0==strcmp(form, "none")) segmentform=0;
            else if (0==strcmp(form, "lines")) segmentform=1;
            else if (0==strcmp(form, "cylinder")) segmentform=2;
        }
    }

    if (fninput!=0L)
    {
        // printf("\nfninput=%s", fninput);
        xxin=fopen(fninput, "r");
    }

    int rc=0;

    switch (func)
    {
        case 0:
        case 1:
        case 2:
        {
            rc=yyparse();
            switch (func)
            {
                case 1: case 2: case 3: break;
            }
            //    printf("\n");
            break;
        }

        case 3:
        {
            int u=1;
            while (u!=0)
            {
                u=yylex();
                dumptoken(u);
            }
            rc=0;
            break;
        }

        default: rc=1;
    }

    if (fninput!=0L && xxin!=0L) fclose(xxin);

    return rc;
}

extern "C" int xxwrap(){ return 1; }

void yyerror(char const*s)
{
    fprintf(stderr, "\n[Line linenum %d] >>>>>> %s\n", linenum, s);
}

void ateof(){}

hanimjoint HUMANOID[1000];
unsigned HLEN=0;

const bool mylog=false;

void pushjoint(const char name[])
{
    if (mylog) printf("\n%*sjoint %s {", 2*jointlevel, "", name);
    HUMANOID[HLEN]=hanimjoint(jointlevel);
    if (name!=0L) HUMANOID[HLEN].setname(name);
    HLEN++;
    jointlevel++;
}

void popjoint()
{
    jointlevel--;
    if (mylog) printf("\n%*s}", 2*jointlevel, "");
}

void endsite()
{
    if (mylog) printf("\n%*s endsite", 2*jointlevel, "");
}

static unsigned char channelcode(unsigned c)
{
    return c-XPOSITION<3?'X'+c-XPOSITION : c-XROTATION<3?'x'+c-XROTATION : '?';
}

void setcurrentchannels(unsigned c0, unsigned c1, unsigned c2)
{
    if (HLEN>0) HUMANOID[HLEN-1].setchannels(channelcode(c0),channelcode(c1),channelcode(c2));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void setcurrentchannels(unsigned c0, unsigned c1, unsigned c2, unsigned c3, unsigned c4, unsigned c5)
{
    if (HLEN>0) HUMANOID[HLEN-1].setchannels(channelcode(c0),channelcode(c1),channelcode(c2),channelcode(c3),channelcode(c4),channelcode(c5));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void setcurrentoffset(double x, double y, double z)
{
    if (HLEN>0) HUMANOID[HLEN-1].setoffset(x,y,z);
    else fprintf(stderr, "\nFehler: offsetspec ohne offenen joint");
}

void dumphumanoid_txt(const hanimjoint JOINTS[], unsigned NJ)
{
    for (unsigned n=0; n<NJ; n++)
    {
        const hanimjoint&J=JOINTS[n];
        printf("\n%*s", 2*level(J), "");
        printf("%s", name(J));
        const unsigned m1=channelnum(J);
        if (m1>0)
        {
            printf(" channels: (%u-%u) ", firstchannel(J), lastchannel(J));
            for (unsigned m=0; m<m1; m++) printf("%c", J[m]);
        }
    }
    printf("\n");
}

void dumphumanoid()
{
#if 0
    dumphumanoid_txt(HUMANOID, HLEN);
#else
    printf(
        "<?xml version='1.0' encoding='iso-8859-1'?>"
        "\n<!DOCTYPE X3D PUBLIC 'ISO//Web3D//DTD X3D 3.1//EN' 'http://www.web3d.org/specifications/x3d-3.1.dtd'>"
        "\n<X3D version='3.1' profile='Full'>"
        "\n<Scene>"
        "\n<NavigationInfo DEF='nistart' type='\"EXAMINE\" \"ANY\"' headlight='%s' speed='1'/>"
        "\n<Viewpoint position='0 20 200'/>"
        , headlight?"true":"false"
    );
    if (floor) printf("\n<Transform translation='0 -2 20'><Shape><Appearance><Material diffuseColor='0.2 0.4 0.2'/></Appearance><Box size='60 0.2 120'/></Shape></Transform>");
    dumphumanoid_x3d(HUMANOID, HLEN);
    dumpmotiontable_x3d(HUMANOID, HLEN);
    printf("\n<TimeSensor DEF='T' loop='true' cycleInterval='%g'/>", framesep*(framenum+1));
    dumpmotionroutes_x3d(HUMANOID, HLEN);
    printf("\n</Scene>\n</X3D>\n");
#endif
}

static unsigned nextchannel=0;

unsigned getchannelrange(unsigned channels)
{
    unsigned n=nextchannel;
    nextchannel+=channels;
    return n;
}

unsigned channelsused(){ return nextchannel; }
