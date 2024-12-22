
#include <cstring>
#include <string>
#include <numbers>
#include "bvhhelp.h"

const double deg=std::numbers::pi_v<double>/180.0;

static char*xcp(const char q[])
{
    if (q==nullptr) return nullptr;
    int n=strlen(q);
    char*s=new char[n+1];
    if (s!=nullptr) strcpy(s, q);
    return s;
}

void hanimjoint::setname(const char s[])
{
    if (name!=nullptr) delete[]name;
    name=xcp(s);
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

void hanimjoint::dumpmotiontables_x3d(const double table[], unsigned lines, unsigned columns)const
{
#define TI(n) (index[n]<0?0.0:T[index[n]])
    if (haspositionchannels())
    {
        int index[3];
        getpositionindexes(index);
        printf("\n<!-- positionindex %d %d %d:", index[0],index[1],index[2]);
        for (unsigned n=0; n<channelnum; n++) printf(" %d", channels[n]);
        printf(" -->");
        printf("\n<PositionInterpolator DEF='nppos_%s'", name);
        printf(" key='");
        for (unsigned n=0; n<lines; n++) printf("%s%g", n>0?" ":"", n*1.0/(lines-1));
        printf("'");
        printf(" keyValue='");
        for (unsigned n=0; n<lines; n++)
        {
            const double*T=table+n*columns;
            printf("%g %g %g%s", TI(0),TI(1),TI(2), n<lines-1?",":"");
        }
        printf("'/>");
    }
    if (hasrotationchannels())
    {
        int index[3], DIR[3];
        const unsigned num=getrotationindexes(index, DIR);
        printf("\n<!-- orientationindex %d(%d)  %d(%d)  %d(%d):", index[0],DIR[0],index[1],DIR[1],index[2],DIR[2]);
        for (unsigned n=0; n<channelnum; n++) printf(" %c", channels[n]);
        printf(" -->");
        printf("\n<OrientationInterpolator DEF='nprot_%s'", name);
        printf(" key='");
        for (unsigned n=0; n<lines; n++) printf("%s%g", n>0?" ":"", n*1.0/(lines-1));
        printf("'");
        printf(" keyValue='");
        for (unsigned n=0; n<lines; n++)
        {
            const double*T=table+n*columns; //!< Tabellenzeile
            const double ANGLE[3]={TI(0)*deg,TI(1)*deg,TI(2)*deg};
            double axis[3], angle;
            AnglesToAxisAngle(axis,&angle,ANGLE,DIR,num);
            printf("%g %g %g %g%s", axis[0],axis[1],axis[2],angle, n<lines-1?",":"");
        }
        printf("'/>");
    }
}

void hanimjoint::dumpmotionroutes_x3d()const
{
    if (haspositionchannels())
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nppos_%s' toField='set_fraction'/>", name);
        printf("\n<ROUTE fromNode='nppos_%s' fromField='value_changed' toNode='%s' toField='set_translation'/>", name, name);
    }
    if (hasrotationchannels())
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nprot_%s' toField='set_fraction'/>", name);
        printf("\n<ROUTE fromNode='nprot_%s' fromField='value_changed' toNode='%s' toField='set_rotation'/>", name, name);
    }
}
