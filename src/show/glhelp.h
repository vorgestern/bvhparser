
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

struct neuprog
{
    struct attrspec
    {
        GLuint compnum;    // {3}
        unsigned comptype; // {GL_FLOAT}
        bool norm;         // {GL_TRUE}
        GLuint stride;     // {VertexSize}
        size_t attroffset; // {3*sizeof(float)}
    };
    GLuint program;
    std::vector<attrspec>specs;
};

void vertexspec(const neuprog&);

template<typename V>struct NeuProg: public neuprog
{
    using VertexType=V;
};

// ====================================================================

struct VAOBuffer
{
    GLuint VAO, VB;
    void generate(), bind();
    operator bool()const{ return VAO>0 && VB>0; }
};
