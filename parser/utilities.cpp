
#include <string_view>
#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <parser.h>

const auto deg=static_cast<float>(std::numbers::pi_v<double>/180.0);

using namespace std;
using namespace glm;

vector<vec3>flatten(const Hierarchy&H, const MotionLine&M)
{
    vector<vec3>Segments(H.size());
    vector<mat4>XF(H.size());
    for (auto j=0u; j<H.size(); ++j)
    {
        auto K=j>0?translate(XF[parent(H[j])], vec3(H[j].offset)):glm::identity<mat4>();
        const auto nc=channelnum(H[j]);
        const auto ci=firstchannel(H[j]);
        for (auto c=0u; c<nc; ++c)
        {
            const float u=static_cast<float>(M[ci+c]);
            switch (H[j][c])
            {
                case 'X': K=translate(K, vec3(u,0,0)); break;
                case 'Y': K=translate(K, vec3(0,u,0)); break;
                case 'Z': K=translate(K, vec3(0,0,u)); break;
                case 'x': K=rotate(K, u*deg, vec3(1,0,0)); break;
                case 'y': K=rotate(K, u*deg, vec3(0,1,0)); break;
                case 'z': K=rotate(K, u*deg, vec3(0,0,1)); break;
            }
        }
        Segments[j]=vec3(K*vec4(0,0,0,1));
        XF[j]=K;
    }
    return Segments;
}

vector<vec3>flatten(const Hierarchy&H)
{
    vector<vec3>Segments(H.size());
    vector<mat4>XF(H.size());
    if (H.size()>0)
    {
        XF[0]=glm::identity<mat4>();
        Segments[0]=vec3(H[0].offset);
    }
    for (auto j=1u; j<H.size(); ++j)
    {
        XF[j]=translate(XF[parent(H[j])], vec3(H[j].offset));
        Segments[j]=vec3(XF[j]*vec4(0,0,0,1));
    }
    return Segments;
}

vector<pair<unsigned,unsigned>> segments(const Hierarchy&H)
{
    vector<pair<unsigned,unsigned>> Segments;
    unsigned j=0;
    for (auto&J: H)
    {
        if (const int p=parent(J); p>=0) Segments.push_back({p,j});
        ++j;
    }
    return Segments;
}

pair<vec3,vec3>boundingbox(const Hierarchy&H)
{
    const auto K=flatten(H);
    if (K.size()<1) return {{1,1,1},{-1,-1,-1}};
    vec3 bmin=K[0], bmax=K[0];
    for (auto p: K) for (int j=0; j<3; ++j)
    {
        if (p[j]<bmin[j]) bmin[j]=p[j];
        if (p[j]>bmax[j]) bmax[j]=p[j];
    }
    return {bmin,bmax};
}

pair<vec3,vec3>boundingbox(const Hierarchy&H, const MotionLine&L)
{
    const auto K=flatten(H, L);
    if (K.size()<1) return {{1,1,1},{-1,-1,-1}};
    vec3 bmin=K[0], bmax=K[0];
    for (auto p: K) for (int j=0; j<3; ++j)
    {
        if (p[j]<bmin[j]) bmin[j]=p[j];
        if (p[j]>bmax[j]) bmax[j]=p[j];
    }
    return {bmin,bmax};
}

pair<vec3,vec3>boundingbox(const Hierarchy&H, const MotionTable&M)
{
    if (M.size()<1) return {{1,1,1},{-1,-1,-1}};
    auto [bmin,bmax]=boundingbox(H,M[0]);
    for (auto k=1u; k<M.size(); ++k)
    {
        const auto [amin,amax]=boundingbox(H,M[k]);
        for (int j=0; j<3; ++j)
        {
            if (amin[j]<bmin[j]) bmin[j]=amin[j];
            if (amax[j]>bmax[j]) bmax[j]=amax[j];
        }
    }
    return {bmin,bmax};
}

vec3 recenter(const mat4&V, const mat4&P, const vec4&viewport, const vec3&modelpoint)
{
    // printf("Unproject\n");
    // const vec4 viewport {0,0,400,300};
    const vec3 wo1=glm::unProject({modelpoint[0],modelpoint[1],-1}, V, P, viewport); // printf("wo1=%.2f %.2f %.2f\n", wo1[0],wo1[1],wo1[2]);
    const vec3 wo2=glm::unProject({modelpoint[0],modelpoint[1],+1}, V, P, viewport); // printf("wo2=%.2f %.2f %.2f\n", wo2[0],wo2[1],wo2[2]);
    const auto wo=wo2-wo2[1]/(wo1[1]-wo2[1])*(wo1-wo2);          // printf("wo =%.2f %.2f %.2f\n", wo[0],wo[1],wo[2]);
    return wo;
}
