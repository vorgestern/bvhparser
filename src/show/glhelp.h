
#include <string_view>
#include <vector>

extern struct GlewInfo
{
    std::pair<int,int> glewversion, glversion, glslversion; // major, minor
} glewinfo;

bool glewinitialise();
std::pair<int,int> glversion();
std::pair<int,int> glslversion();

GLuint build_program(std::string_view vssource, std::string_view fssource);

// ====================================================================

struct VAOBuffer
{
    GLuint VAO, VB;
    void generate(), bind();
    operator bool()const{ return VAO>0 && VB>0; }
};
