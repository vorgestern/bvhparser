
#include <string>
#include <string_view>
#include <vector>
#include <bvhhelp.h>

using namespace std;

void dumphumanoid_x3d(const Hierarchy&, SegmentForms),
     dumpmotiontable_x3d(const Hierarchy&, const MotionTable&),
     dumpmotionroutes_x3d(const Hierarchy&);

void output_x3d(const Hierarchy&H, const MotionTable&M, const OutputOptions&opt)
{
    const auto B=compute_boundingbox(H, M);
    const auto d=B.bmax-B.bmin;
    const auto r=0.01*sqrt(d.x*d.x+d.y*d.y+d.z*d.z);
    const auto c=0.5*glm::dvec3(B.bmin[0]+B.bmax[0], 2*B.bmin[1], B.bmin[2]+B.bmax[2]);

    printf(
        "<?xml version='1.0' encoding='iso-8859-1'?>"
        "\n<!DOCTYPE X3D PUBLIC 'ISO//Web3D//DTD X3D 3.1//EN' 'http://www.web3d.org/specifications/x3d-3.1.dtd'>"
        "\n<X3D version='3.1' profile='Full'>"
        "\n<Scene>"
        "\n<NavigationInfo DEF='nistart' type='\"EXAMINE\" \"ANY\"' headlight='%s' speed='1'/>"
        "\n<Viewpoint position='%.3g %.3g %.3g'/>", opt.has_headlight?"true":"false", c[0], c[1], c[2]+500*r
    );

    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape DEF='bb1'><Appearance><Material diffuseColor='1 1 1'/></Appearance><Sphere radius='%g'/></Shape></Transform>", B.bmin[0], B.bmin[1], B.bmin[2], r);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmin[0], B.bmin[1], B.bmax[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmin[0], B.bmax[1], B.bmin[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmin[0], B.bmax[1], B.bmax[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmax[0], B.bmin[1], B.bmin[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmax[0], B.bmin[1], B.bmax[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmax[0], B.bmax[1], B.bmin[2]);
    printf("\n<Transform translation='%.4g %.4g %.4g'><Shape USE='bb1'/></Transform>", B.bmax[0], B.bmax[1], B.bmax[2]);

    if (opt.has_floor)
    {
        printf(R"__(
<Transform translation='%.4g %.4g %.4g'>
    <Shape><Appearance><Material diffuseColor='0.2 0.4 0.2'/></Appearance><Box size='%.4g %.4g %.4g'/></Shape>
</Transform>)__", 0.5*(B.bmin[0]+B.bmax[0]), B.bmin[1], 0.5*(B.bmin[2]+B.bmax[2]),
        B.bmax[0]-B.bmin[0], 0.01*(B.bmax[1]-B.bmin[1]), B.bmax[2]-B.bmin[2]);
    }
    dumphumanoid_x3d(H, opt.segmentshape);
    dumpmotiontable_x3d(H, M);
    printf("\n<TimeSensor DEF='T' loop='true' cycleInterval='%g'/>", opt.totaltime);
    if (M.size()>0) dumpmotionroutes_x3d(H);
    printf("\n</Scene>\n</X3D>\n");
}

// ===================================================

void dumphumanoid_bb(const Hierarchy&H)
{
#if 0
    vector<glm::dvec3>MyLine;
    for (const auto&M: BVHMotion)
        // const auto&M=MotionTable[10];
    {
        vector<glm::dvec3>Points;
        compute_traces(Points, H, M);
        MyLine.push_back(Points[41]); // [12]
    }
    printf("\n<Shape><Appearance><Material emissiveColor='1 1 1'/></Appearance><LineSet vertexCount='%u'>\n<Coordinate point='", MyLine.size());
    unsigned num=0;
    for (auto&P: MyLine) printf("%s%g %g %g", num++>0?", ":"", P[0], P[1], P[2]);
    printf("'/></LineSet></Shape>");
#endif
}

// ==================================================

void dumphumanoid_txt(const Hierarchy&JOINTS)
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
