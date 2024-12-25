
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

static enum class OutputType {ffregular, ffmix, fflexeroutput, fftext} func=OutputType::ffregular;
static SegmentForms segmentshape=SegmentForms::cylinder;
bool has_floor=false, headlight_on=true;

int yyparse();
extern FILE*xxin;

lexercontext LCX;
parsercontext PCX;

int xxlex();

// Flex will not create yylex() directly, because we used -prefix=xx to set the name to xxlex
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
        else if (arg=="-f" && a<argc-1) fninput=argv[++a];
        else if (arg=="-s0") segmentshape=SegmentForms::none;
        else if (arg=="-s1") segmentshape=SegmentForms::line;
        else if (arg=="-s2") segmentshape=SegmentForms::cylinder;
        else if (arg=="-segments" && a<argc-1)
        {
            const string_view form=argv[++a];
            if (form=="none") segmentshape=SegmentForms::none;
            else if (form=="lines") segmentshape=SegmentForms::line;
            else if (form=="cylinder") segmentshape=SegmentForms::cylinder;
        }
    }

    // Lexer and parser have to handle linefeeds of both descriptions
    // anyway to work on Linux and Windows, so open the file explicitly in binary mode.
    if (!fninput.empty()) xxin=fopen(fninput.string().c_str(), "rb");

    int rc=1;

    switch (func)
    {
        case OutputType::ffregular:
        case OutputType::ffmix:
        case OutputType::fftext: rc=yyparse(); break;

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
    fprintf(stderr, "\n[Line linenum %d] >>>>>> %s\n", LCX.linenum, s);
}

void ateof(){}

Hierarchy BVHHumanoid;
MotionTable BVHMotion;

void parsercontext::pushjoint(const char name[])
{
    if (mylog) printf("\n%*sjoint %s {", 2*jointlevel, "", name);
    BVHHumanoid.emplace_back(jointlevel);
    if (name!=nullptr && name[0]!=0) BVHHumanoid.back().setname(name);
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

void setcurrentchannels(unsigned c0, unsigned c1, unsigned c2)
{
    if (BVHHumanoid.size()>0) BVHHumanoid.back().setchannels(channelcode(c0),channelcode(c1),channelcode(c2));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void setcurrentchannels(unsigned c0, unsigned c1, unsigned c2, unsigned c3, unsigned c4, unsigned c5)
{
    if (BVHHumanoid.size()>0) BVHHumanoid.back().setchannels(channelcode(c0),channelcode(c1),channelcode(c2),channelcode(c3),channelcode(c4),channelcode(c5));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void setcurrentoffset(double x, double y, double z)
{
    if (BVHHumanoid.size()>0) BVHHumanoid.back().offset={x,y,z};
    else fprintf(stderr, "\nFehler: offsetspec ohne offenen joint");
}

static void dumphumanoid_txt(const Hierarchy&JOINTS)
{
    for (const auto&J: JOINTS)
    {
        switch (type(J))
        {
            case jtype::root:
            case jtype::joint:
            {
                auto k=printf("\n%*s", 4*level(J), "");
                k+=printf("%s", name(J));
                const unsigned m1=channelnum(J);
                if (m1>0)
                {
                    k+=printf("%c", ' ');
                    for (unsigned m=0; m<m1; m++) k+=printf("%c", J[m]);
                    k+=printf(" %u-%u", firstchannel(J), lastchannel(J));
                }
                const auto T=J.offset;
                if (k<64) k+=printf("%*s", 64-k, "");
                else if (k<96) k+=printf("%*s", 96-k, "");
                k+=printf("(%.4g %.4g %.4g)", T[0], T[1], T[2]);
                break;
            }
            case jtype::endsite:
            {
                const auto T=J.offset;
                printf(" (%.4g %.4g %.4g)", T[0], T[1], T[2]);
                break;
            }
        }
    }
    printf("\n");
}

static void dumphumanoid_xml(const vector<hanimjoint>&JOINTS)
{
    printf(
        "<?xml version='1.0' encoding='iso-8859-1'?>"
        "\n<!DOCTYPE X3D PUBLIC 'ISO//Web3D//DTD X3D 3.1//EN' 'http://www.web3d.org/specifications/x3d-3.1.dtd'>"
        "\n<X3D version='3.1' profile='Full'>"
        "\n<Scene>"
        "\n<NavigationInfo DEF='nistart' type='\"EXAMINE\" \"ANY\"' headlight='%s' speed='1'/>"
        "\n<Viewpoint position='0 20 200'/>", headlight_on?"true":"false"
    );
    if (has_floor) printf("\n<Transform translation='0 -2 20'><Shape><Appearance><Material diffuseColor='0.2 0.4 0.2'/></Appearance><Box size='60 0.2 120'/></Shape></Transform>");

    vector<glm::dvec3>MyLine;
    for (const auto&M: BVHMotion)
    // const auto&M=MotionTable[10];
    {
        vector<glm::dvec3>Points;
        compute_traces(Points, JOINTS, M);
        MyLine.push_back(Points[41]); // [12]
    }
    printf("\n<Shape><Appearance><Material emissiveColor='1 1 1'/></Appearance><LineSet vertexCount='%u'>\n<Coordinate point='", MyLine.size());
    unsigned num=0;
    for (auto&P: MyLine) printf("%s%g %g %g", num++>0?", ":"", P[0], P[1], P[2]);
    printf("'/></LineSet></Shape>");

    dumphumanoid_x3d(BVHHumanoid, segmentshape);
    dumpmotiontable_x3d(BVHHumanoid, BVHMotion);
    printf("\n<TimeSensor DEF='T' loop='true' cycleInterval='%g'/>", PCX.framesep*(PCX.framenum+1));
    if (BVHMotion.size()>0) dumpmotionroutes_x3d(BVHHumanoid);
    printf("\n</Scene>\n</X3D>\n");
}

void dumphumanoid()
{
    if (func==OutputType::fftext) dumphumanoid_txt(BVHHumanoid);
    else dumphumanoid_xml(BVHHumanoid);
}

static unsigned nextchannel=0;

unsigned reservechannels(unsigned channels)
{
    unsigned n=nextchannel;
    nextchannel+=channels;
    return n;
}
