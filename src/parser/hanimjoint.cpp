
#include <parser.h>
#include <numbers>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

const double deg=std::numbers::pi_v<double>/180.0;

jtype type(const hanimjoint&X)
{
    return X.level==0?jtype::root
        :X.name.empty()?jtype::endsite:jtype::joint; // regular joints are expected to be named
}

void hanimjoint::setname(const char s[])
{
    name=s;
}

bool haspositionchannels(const hanimjoint&J)
{
    for (unsigned c=0; c<J.channelnum; c++) switch (J.channels[c])
    {
        case 'X': case 'Y': case 'Z': return true;
    }
    return false;
}

bool hasrotationchannels(const hanimjoint&J)
{
    for (unsigned c=0; c<J.channelnum; c++) switch (J.channels[c])
    {
        case 'x': case 'y': case 'z': return true;
    }
    return false;
}

void hanimjoint::setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2)
{
    channelnum=3;
    channels[0]=c0; channels[1]=c1; channels[2]=c2;
    firstchannelindex=start;
}

void hanimjoint::setchannels(unsigned start, unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5)
{
    channelnum=6;
    channels[0]=c0; channels[1]=c1; channels[2]=c2; channels[3]=c3; channels[4]=c4; channels[5]=c5;
    firstchannelindex=start;
}

void hanimjoint::getpositionindexes(int index[])const
{
    index[0]=index[1]=index[2]=-1;
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'X': index[0]=firstchannelindex+c; break;
        case 'Y': index[1]=firstchannelindex+c; break;
        case 'Z': index[2]=firstchannelindex+c; break;
    }
}

unsigned hanimjoint::getrotationindexes(int index[], int dir[])const
{
    index[0]=index[1]=index[2]=-1;
    unsigned u=0;
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'x': index[u]=firstchannelindex+c; dir[u]=0; u++; break;
        case 'y': index[u]=firstchannelindex+c; dir[u]=1; u++; break;
        case 'z': index[u]=firstchannelindex+c; dir[u]=2; u++; break;
    }
    return u;
}

glm::dmat4 hanimjoint::getrotation(const MotionLine&Line)const
{
    glm::dmat4 Result={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'x': Result*=glm::eulerAngleX(Line[firstchannelindex+c]*deg); break;
        case 'y': Result*=glm::eulerAngleY(Line[firstchannelindex+c]*deg); break;
        case 'z': Result*=glm::eulerAngleZ(Line[firstchannelindex+c]*deg); break;
    }
    return Result;
}

glm::dmat4 hanimjoint::gettransform(const MotionLine&Line)const
{
    glm::dmat4 Result={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'X': Result=glm::translate(Result, {Line[firstchannelindex+c],0,0}); break;
        case 'Y': Result=glm::translate(Result, {0,Line[firstchannelindex+c],0}); break;
        case 'Z': Result=glm::translate(Result, {0,0,Line[firstchannelindex+c]}); break;
        case 'x': Result*=glm::eulerAngleX(Line[firstchannelindex+c]*deg); break;
        case 'y': Result*=glm::eulerAngleY(Line[firstchannelindex+c]*deg); break;
        case 'z': Result*=glm::eulerAngleZ(Line[firstchannelindex+c]*deg); break;
    }
    return Result;
}
