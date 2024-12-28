
#include <string_view>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <parser.h>

using namespace std;
using namespace glm;

vector<vec3>flatten(const Hierarchy&H)
{
    vector<vec3>Segments;
    vector<mat4>XFStack;
    XFStack.push_back(glm::identity<mat4>());
    struct { int lev; } merk={-1};
    for (auto&J: H)
    {
        const int lev=level(J);
        assert(lev<=merk.lev+1);
        if (lev==merk.lev+1)
        {
            const auto offset=vec3(J.offset);
            const auto von=XFStack.back()*vec4 {0,0,0,1};
            const auto neu=XFStack.back()*vec4(J.offset, 1);
            Segments.push_back(vec3(neu));
            auto XFNeu=translate(XFStack.back(), offset);
            XFStack.push_back(XFNeu);
            merk.lev=lev;
        }
        else if (lev==merk.lev)
        {
            const auto offset=vec3(J.offset);
            const auto von=XFStack.back()*vec4 {0,0,0,1};
            const auto neu=XFStack.back()*vec4(J.offset, 1);
            Segments.push_back(vec3(neu));
            auto XFNeu=translate(XFStack.back(), offset);
            XFStack.back()=XFNeu;
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
            const auto von=XFStack.back()*vec4 {0,0,0,1};
            const auto neu=XFStack.back()*vec4(offset, 1);
            Segments.push_back(vec3(neu));
            // Replace the transform of the sibling with our own. 
            auto XFNeu=translate(XFStack.back(), vec3(offset));
            XFStack.back()=XFNeu;
        }
        // else printf("\nTSNH\n");
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
