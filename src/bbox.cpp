
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <bvhhelp.h>
#include <glm/ext/matrix_transform.hpp>

using namespace std;
using namespace glm;

static bool undefined(const bbox&X)
{
    return X.bmin[0]>X.bmax[0] || X.bmin[1]>X.bmax[1] || X.bmin[2]>X.bmax[2];
}

static void bbinclude(bbox&X, const dvec3&T)
{
    if (undefined(X)) for (auto j=0; j<3; ++j)
    {
        X.bmin[j]=X.bmax[j]=T[j];
    }
    else for (auto j=0; j<3; ++j)
    {
        if (T[j]<X.bmin[j]) X.bmin[j]=T[j];
        if (T[j]>X.bmax[j]) X.bmax[j]=T[j];
    }
}

static void bbcompute(bbox&B, const Hierarchy&H, const MotionLine&L)
{
    vector<dmat4>Stack;
    Stack.push_back(glm::identity<dmat4>());
    struct
    {
        int lev;
    } merk={(int)level(H[0])-1};
    for (const auto&J: H)
    {
        const int lev=level(J);
        if (lev>merk.lev+1)
        {
            // This should never happen.
            throw runtime_error("Unexpected level.");
        }
        else if (lev==merk.lev+1)
        {
            Stack.push_back(Stack.back()*J.gettransform(L));
            ++merk.lev;
        }
        else while (Stack.size()>1 && lev<merk.lev)
        {
            Stack.pop_back();
            --merk.lev;
        }
        const auto position=Stack.back()*dvec4(J.offset, 1);
        bbinclude(B, position);
    }
}

bbox compute_boundingbox(const Hierarchy&H, const MotionTable&M)
{
    bbox X;
    for (const auto&L: M) bbcompute(X, H, L);
    return X;
}

void compute_traces(vector<dvec3>&Lines, const vector<hanimjoint>&JOINTS, const vector<double>&MotionLine)
{
    vector<dmat4>Stack;
    Stack.push_back(glm::identity<dmat4>());
    struct
    {
        int lev;
    } merk={(int)level(JOINTS[0])-1};
    Lines.clear();
    for (const auto&J: JOINTS)
    {
        const int lev=level(J);
        if (lev>merk.lev+1)
        {
            // This should never happen.
            throw runtime_error("Unexpected level.");
        }
        else if (lev==merk.lev+1)
        {
            Stack.push_back(Stack.back()*J.gettransform(MotionLine));
            ++merk.lev;
        }
        else while (Stack.size()>1 && lev<merk.lev)
        {
            Stack.pop_back();
            --merk.lev;
        }
// printf(" ([%u])", Stack.size());
        const auto position=Stack.back()*dvec4(J.offset, 1);
// printf(" %.3g %.3g %.3g", position[0], position[1], position[2]);
        Lines.push_back(position);
    }
}
