
#include <string_view>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <parser.h>

using namespace std;
using namespace glm;

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
