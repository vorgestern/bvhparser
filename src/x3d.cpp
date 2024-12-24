
#include <string>
#include <vector>
#include <cmath>
#include <bvhhelp.h>

using namespace std;

void dumphumanoid_x3d(const vector<hanimjoint>&JOINTS, SegmentForms segmentshape)
{
    int merklev=0;
    unsigned nummat=0;
    bool start=true;
    for (const auto&J: JOINTS)
    {
        const int lev=level(J);
        const auto T=J.offset;
        const char*nam=name(J);

        // Upon rising up again in the hierarchy, close open joints.
        bool up=!start && lev<merklev;
        if (up) for (int d=merklev; d>=lev; d--) printf("\n%*s</Transform>", 2*d, "");
        start=false;

        switch (type(J))
        {
            case jtype::root: break;

            case jtype::joint:
            case jtype::endsite:
            {
                switch (segmentshape)
                {
                    case SegmentForms::none: break;
                    case SegmentForms::line:
                    {
                        printf("\n%*s<Shape><LineSet vertexCount='2'><Coordinate point='0 0 0,%g %g %g'/></LineSet></Shape>", 2*lev, "", T[0],T[1],T[2]);
                        break;
                    }
                    case SegmentForms::cylinder:
                    {
                        const double hei=sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
                        const double rad=0.1*sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
                        const double xangle=atan2(T[2],T[1]), zangle=atan2(T[0],T[1]);
                        if (xangle!=0 || zangle!=0)
                        {
const double ANGLE[2]={xangle,-zangle};
                            const int DIR[2]={0,2};
                            const auto A=AnglesToAxisAngle(ANGLE,DIR,2);
                            printf("\n<Transform translation='%g %g %g' rotation='%g %g %g %g'>",
                                .5*T[0],.5*T[1],.5*T[2],
                                A[0],A[1],A[2],A[3]);
                        }
                        else printf("\n<Transform translation='%g %g %g'>", .5*T[0],.5*T[1],.5*T[2]);
                        if (nummat==0) printf("\n<Shape><Appearance DEF='app1'><Material DEF='mat1' diffuseColor='1 1 1' specularColor='1 1 1' shininess='1.0'/></Appearance>");
                        else           printf("\n<Shape><Appearance USE='app1'/>");
printf("<Cylinder height='%g' radius='%g'/></Shape>", hei, 0.2*rad);
                        printf("</Transform>");
                        nummat++;
                        break;
                    }
                }
                break;
            }
        }

        printf("\n%*s<%s", 2*lev, "", lev>0?"Transform":"Transform");
        printf(" translation='%g %g %g'", T[0],T[1],T[2]);
        if (nam!=nullptr) printf(" DEF='%s'>", nam);
        else              printf(">");
        if (segmentshape==SegmentForms::none)
        {
            const double rad=0.10*sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
            if (nummat==0) printf("<Shape><Appearance DEF='app1'><Material DEF='mat1' diffuseColor='1 1 1' specularColor='1 1 1' shininess='1.0'/></Appearance><Sphere radius='%g'/></Shape>", rad);
            else           printf("<Shape><Appearance USE='app1'/><Sphere radius='%g'/></Shape>", rad);
            nummat++;
        }
        merklev=level(J);
    }
    for (int d=merklev; d>=0; d--) printf("\n%*s</Transform>", 2*d, "");
}

void dumpmotiontable_x3d(const vector<hanimjoint>&joints, const vector<vector<double>>&Table)
{
    if (Table.size()>0)
    {
        printf("\n<!-- Interpolators -->");
        for (const auto&j: joints) j.dumpmotiontables_x3d(Table);
    }
}

void dumpmotionroutes_x3d(const vector<hanimjoint>&joints)
{
    printf("\n<!-- Routes -->");
    for (const auto&j: joints) j.dumpmotionroutes_x3d();
}
