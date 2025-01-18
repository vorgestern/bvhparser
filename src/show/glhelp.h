
#include <string_view>
#include <vector>

extern struct GlewInfo
{
    std::pair<int,int> glewversion, glversion, glslversion; // major, minor
} glewinfo;

bool glewinitialise();
std::pair<int,int> glversion();
std::pair<int,int> glslversion();

// ====================================================================

GLuint build_program(std::string_view vssource, std::string_view fssource);

template<typename T> concept HasGlProgram=requires(T a){ a.program; };
template<typename P> requires HasGlProgram<P> inline void useprog(const P&a){ glUseProgram(a.program); }
template<typename P> requires HasGlProgram<P> inline void resetprog(const P&a){ void gl_texture_reset(); a.program=0; gl_texture_reset(); }
template<typename P> requires HasGlProgram<P> inline bool isvalidprog(const P&a){ return a.program!=0; }

// ====================================================================

struct VAOBuffer
{
    GLuint VAO, VB;
    void generate(), bind();
    operator bool()const{ return VAO>0 && VB>0; }
};
