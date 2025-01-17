
#include <string_view>
#include <vector>

extern struct GlewInfo
{
    std::pair<int,int> glewversion, glversion, glslversion; // major, minor
} glewinfo;

bool glewinitialise();
std::pair<int,int> glversion();
std::pair<int,int> glslversion();
