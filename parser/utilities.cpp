
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
    if (H.size()>0)
    {
        XF[0]=glm::identity<mat4>();
        Segments[0]=vec3(H[0].offset);
    }
    for (auto j=1u; j<H.size(); ++j)
    {
        auto K=translate(XF[parent(H[j])], vec3(H[j].offset));
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
