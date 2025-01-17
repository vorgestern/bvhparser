
#include <tuple>
#include <vector>
#include <string_view>
#include <parser.h>
#include "glhelp.h"

struct vpstruct {
    float dist, elev, azim;
    glm::vec3 focus, bbcenter;
};

glm::vec3 eyevector(const vpstruct&);

extern struct FrameInfo
{
    glm::vec4 viewport;
    enum {stop, init, animate, step} state;
    static const enum class dir {a,b} forward=dir::a, back=dir::b;
    dir animdir;
    Hierarchy Hier;
    MotionTable Motion;
    std::vector<std::pair<unsigned,unsigned>> Segments;
    unsigned f, num;
    double timestep;
    vpstruct vp;
} frameinfo;

void fmtoutput(const char*format, ...);
Fl_Window*NewGlWindow(int x, int y, int w, int h);
Fl_Window*NewModelWindow(int x, int y, int w, int h);

GLuint mkvertexshader(std::string_view source);
GLuint mkfragmentshader(std::string_view source);
GLuint linkshaderprogram(GLint vs, GLint fs, std::vector<std::string_view>fragdataloc);
