
#include <string>
#include <parser.h>
#include <parsercontext.h>
#include "bvhconv.h"

using namespace std;

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
    if (Scene->H.size()>0) Scene->H.back().setchannels(reservechannels(3), channelcode(c0),channelcode(c1),channelcode(c2));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void parsercontext::setcurrentchannels(unsigned c0, unsigned c1, unsigned c2, unsigned c3, unsigned c4, unsigned c5)
{
    if (Scene->H.size()>0) Scene->H.back().setchannels(reservechannels(6), channelcode(c0),channelcode(c1),channelcode(c2),channelcode(c3),channelcode(c4),channelcode(c5));
    else fprintf(stderr, "\nFehler: channelspec ohne offenen joint");
}

void parsercontext::setcurrentoffset(double x, double y, double z)
{
    if (Scene->H.size()>0) Scene->H.back().offset={x,y,z};
    else fprintf(stderr, "\nFehler: offsetspec ohne offenen joint");
}

void parsercontext::parserfinished()
{
    Scene->totaltime=framesep*(framenum+1);
}

// =========================================

int parsercontext::scanmotionline(const char pad[])
{
    int offset=0, nr=0;
    double value=0;
    Scene->BVHLine.clear();
    while (1==sscanf(pad+offset, "%lg%n", &value, &nr))
    {
        Scene->BVHLine.push_back(value);
        offset+=nr;
    } 
    return Scene->BVHLine.size();
}

void parsercontext::storetableline(unsigned columns){ Scene->M.push_back(Scene->BVHLine); }

unsigned parsercontext::reservechannels(unsigned channels)
{
    unsigned n=nextchannel;
    nextchannel+=channels;
    return n;
}
