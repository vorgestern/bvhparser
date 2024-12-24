
#include <string>
#include <vector>
#include <numbers>
#include <bvhhelp.h>

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

bool hanimjoint::haspositionchannels()const
{
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'X': case 'Y': case 'Z': return true;
    }
    return false;
}

bool hanimjoint::hasrotationchannels()const
{
    for (unsigned c=0; c<channelnum; c++) switch (channels[c])
    {
        case 'x': case 'y': case 'z': return true;
    }
    return false;
}

void hanimjoint::setchannels(unsigned char c0, unsigned char c1, unsigned char c2)
{
    channelnum=3;
    channels[0]=c0; channels[1]=c1; channels[2]=c2;
    firstchannelindex=reservechannels(3);
}

void hanimjoint::setchannels(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5)
{
    channelnum=6;
    channels[0]=c0; channels[1]=c1; channels[2]=c2; channels[3]=c3; channels[4]=c4; channels[5]=c5;
    firstchannelindex=reservechannels(6);
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

void hanimjoint::dumpmotiontables_x3d(const std::vector<std::vector<double>>&Table)const
{
    size_t col=0;
    const auto lines=Table.size();
    if (haspositionchannels())
    {
        int index[3];
        getpositionindexes(index);
        printf("\n<!-- positionindex %d %d %d:", index[0],index[1],index[2]);
        for (unsigned n=0; n<channelnum; n++) printf(" %d", channels[n]);
        printf(" -->");
        col=printf("\n<PositionInterpolator DEF='nppos_%s'", name.c_str());
        col+=printf(" key='");
        for (unsigned n=0; n<lines; n++)
        {
            col+=printf("%s%g", n>0?" ":"", n*1.0/(lines-1));
            if (col>128){ printf("\n"); col=0; }
        }
        col+=printf("'");
        col+=printf(" keyValue='");
        for (unsigned n=0; n<lines; n++)
        {
            const auto&L=Table[n];
            col+=printf("%g %g %g%s", L[index[0]],L[index[1]],L[index[2]], n<lines-1?",":"");
            if (col>128 && n+1<lines){ printf("\n"); col=0; }
        }
        col+=printf("'/>");
    }
    if (hasrotationchannels())
    {
        int index[3], DIR[3];
        const unsigned num=getrotationindexes(index, DIR);
        col=printf("\n<!-- orientationindex %d(%d)  %d(%d)  %d(%d):", index[0],DIR[0],index[1],DIR[1],index[2],DIR[2]);
        for (unsigned n=0; n<channelnum; n++) col+=printf(" %c", channels[n]);
        col+=printf(" -->");
        col=printf("\n<OrientationInterpolator DEF='nprot_%s'", name.c_str());
        col+=printf(" key='");
        for (unsigned n=0; n<lines; n++)
        {
            col+=printf("%s%g", n>0?" ":"", n*1.0/(lines-1));
            if (col>128){ printf("\n"); col=0; }
        }
        col+=printf("'");
        col+=printf(" keyValue='");
        for (unsigned n=0; n<lines; n++)
        {
            const auto&L=Table[n];
            const double ANGLE[3]={L[index[0]]*deg,L[index[1]]*deg,L[index[2]]*deg};
            double axis[3], angle;
            AnglesToAxisAngle(axis,&angle,ANGLE,DIR,num);
            col+=printf("%g %g %g %g%s", axis[0],axis[1],axis[2],angle, n<lines-1?",":"");
            if (col>128 && n+1<lines){ printf("\n"); col=0; }
        }
        printf("'/>");
    }
}

void hanimjoint::dumpmotionroutes_x3d()const
{
    if (haspositionchannels())
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nppos_%s' toField='set_fraction'/>", name.c_str());
        printf("\n<ROUTE fromNode='nppos_%s' fromField='value_changed' toNode='%s' toField='set_translation'/>", name.c_str(), name.c_str());
    }
    if (hasrotationchannels())
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nprot_%s' toField='set_fraction'/>", name.c_str());
        printf("\n<ROUTE fromNode='nprot_%s' fromField='value_changed' toNode='%s' toField='set_rotation'/>", name.c_str(), name.c_str());
    }
}
