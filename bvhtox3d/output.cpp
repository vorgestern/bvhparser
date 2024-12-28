
#include <string>
#include <string_view>
#include <vector>
#include <bvhhelp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

void dumphumanoid_x3d(const Hierarchy&, SegmentForms),
     dumpmotiontable_x3d(const Hierarchy&, const MotionTable&);

static void dumpmotionroutes_x3d(const hanimjoint&J)
{
    if (haspositionchannels(J))
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nppos_%s' toField='set_fraction'/>", name(J));
        printf("\n<ROUTE fromNode='nppos_%s' fromField='value_changed' toNode='%s' toField='set_translation'/>", name(J), name(J));
    }
    if (hasrotationchannels(J))
    {
        printf("\n<ROUTE fromNode='T' fromField='fraction_changed' toNode='nprot_%s' toField='set_fraction'/>", name(J));
        printf("\n<ROUTE fromNode='nprot_%s' fromField='value_changed' toNode='%s' toField='set_rotation'/>", name(J), name(J));
    }
}

void output_x3d(const Hierarchy&H, const MotionTable&M, double totaltime, const OutputOptions&opt)
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
    printf("\n<TimeSensor DEF='T' loop='true' cycleInterval='%g'/>", totaltime);
    if (M.size()>0)
    {
        printf("\n<!-- Routes -->");
        for (const auto&J: H) dumpmotionroutes_x3d(J);
    }
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

void dump_flat(const Hierarchy&H)
{
    printf("dump_flat\n");
    struct Seg { vec4 a, b; jtype typ; int lev; string name; };
    vector<Seg>Segments;

    struct XFS { mat4 XF; int lev; };
    vector<XFS>XFStack;
    XFStack.push_back({glm::identity<mat4>(),-1});
    struct { int lev; } merk={-1};
    for (auto&J: H)
    {
        const int lev=level(J);
        if (lev>merk.lev+1)
        {
            printf("\nTSNH 1\n");
        }
        else if (lev==merk.lev+1)
        {
            const auto offset=vec3(J.offset);
            const auto von=XFStack.back().XF*vec4 {0,0,0,1};
            const auto neu=XFStack.back().XF*vec4(J.offset, 1);
            Segments.push_back({von, neu, type(J), lev, name(J)});
            auto XFNeu=translate(XFStack.back().XF, offset);
            XFStack.push_back({XFNeu, lev});
            merk.lev=lev;
        }
        else if (lev<merk.lev)
        {
            while (lev<merk.lev)
            {
                XFStack.pop_back();
                --merk.lev;
            }
            const auto offset=vec3(J.offset);
            const auto von=XFStack.back().XF*vec4 {0,0,0,1};
            const auto neu=XFStack.back().XF*vec4(offset, 1);
            Segments.push_back({von, neu, type(J), lev, name(J)});
            // Replace the transform of the sibling with our own. 
            auto XFNeu=translate(XFStack.back().XF, vec3(offset));
            XFStack.back()={XFNeu, lev};
        }
        else printf("\nTSNH\n");
    }

    const auto num=Segments.size();
    printf("VertexData %zu joints:\n", num);
    auto j=0u;
    char types[]="rje";
    for (auto S: Segments)
    {
        char A[100], B[100];
        sprintf(A, "{%.3g, %.3g, %.3g}", S.a[0], S.a[1], S.a[2]);
        sprintf(B, "{%.3g, %.3g, %.3g}", S.b[0], S.b[1], S.b[2]);
        printf("\n%3u %c %d\t%24s\t%24s %s", j, types[(int)S.typ], S.lev, A, B, S.name.c_str());
        ++j;
    }
}

// ==================================================

void dumphumanoid_txt(const Hierarchy&JOINTS)
{
    int jointindex=0;
    for (const auto&J: JOINTS)
    {
        switch (type(J))
        {
            case jtype::root:
            case jtype::joint:
            {
                auto k=printf("\n%2d %2d %*s", jointindex, parent(J), 4*level(J), "");
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
        ++jointindex;
    }
    printf("\n");
    auto Segments=segments(JOINTS);
    printf("%zu Segments:\n", Segments.size());
    for (auto s: Segments) printf("\t%u %u\n", s.first, s.second);
    printf("========\n");
    dump_flat(JOINTS);
    auto X=flatten(JOINTS);
    unsigned j=0;
    for (auto S: X)
    {
        char A[100];
        sprintf(A, "{%.3g, %.3g, %.3g}", S[0], S[1], S[2]);
        printf("\n%3u\t%24s", j, A);
        ++j;
    }
}
