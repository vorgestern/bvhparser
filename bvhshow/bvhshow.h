
#include <tuple>
#include <vector>
#include <string_view>
#include <parser.h>

extern struct GlewInfo
{
    std::pair<int,int> glewversion, glversion, glslversion; // major, minor
} glewinfo;

extern struct FrameInfo
{
    enum {stop, initmodel, initdummy, animatedummy, animatemodel} state;
    Hierarchy Hier;
    MotionTable Motion;
    std::vector<std::pair<unsigned,unsigned>> Segments;
    unsigned f, num;
    double dt;
} frameinfo;

void initialise_glew();
void fmtoutput(const char*format, ...);
Fl_Window*NewGlWindow(int x, int y, int w, int h);
Fl_Window*NewModelWindow(int x, int y, int w, int h);

GLuint mkvertexshader(const char source[]);
GLuint mkfragmentshader(const char source[]);
GLuint linkshaderprogram(GLint vs, GLint fs, std::vector<std::string_view>fragdataloc);
