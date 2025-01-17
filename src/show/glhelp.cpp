
#include <FL/Fl_Gl_Window.H>
#include <GL/glew.h>
#include <FL/gl.h> // for gl_texture_reset()
#include "glhelp.h"

void fmtoutput(const char*format, ...);

using namespace std;

GlewInfo glewinfo;

static pair<int,int> glversion()
{
    int major=0, minor=0;
    auto str=(const char*)glGetString(GL_VERSION);
    const auto nr=sscanf(str, "%d.%d", &major, &minor);
    if (nr==2)
    {
        fmtoutput("GL version '%s'\n", str);
        return {major,minor};
    }
    else if (nr==1)
    {
        fmtoutput("GL version '%s' (Cannot retrieve minor version)\n", str);
        return {major,0};
    }
    else
    {
        fmtoutput("GL version '%s' (Cannot retrieve OpenGL version)\n", str);
        return {};
    }
}

static pair<int,int>glslversion()
{
    // Query major and minor version numbers of the shading language.
    int major=0, minor=0;
    auto str=(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    const int nr=sscanf(str, "%d.%d", &major, &minor);
    if (nr==2)
    {
        fmtoutput("GLSL version '%s'\n", str);
        return {major, minor};
    }
    else if (nr==1)
    {
        fmtoutput("GLSL version '%s' (Cannot retrieve minor)\n", str);
        return {major,0};
    }
    else
    {
        fmtoutput("GLSL version '%s' (Cannot retrieve major.minor)\n", str);
        return {};
    }
}

bool glewinitialise()
{
    static pair<int,int> glewversion;
    if (glewversion.first!=0) return true;

    GLenum err=glewInit(); // Defines pointers to functions of OpenGL V 1.2 and above
#ifdef FLTK_USE_WAYLAND
    // glewInit returns GLEW_ERROR_NO_GLX_DISPLAY with Wayland
    // see https://github.com/nigels-com/glew/issues/273
    if (fl_wl_display() && err==GLEW_ERROR_NO_GLX_DISPLAY) err=GLEW_OK;
#endif

    if (err!=GLEW_OK)
    {
        Fl::warning("glewInit() failed returning %u", err);
        return false;
    }

    auto str=(const char*)glewGetString(GLEW_VERSION);
    int major=0, minor=0;
    const int nr=sscanf(str, "%d.%d", &major, &minor);
    if (nr==2)
    {
        fmtoutput("Using Glew version %s.\n", str);
        glewversion={major, minor};
    }
    else if (nr==1)
    {
        fmtoutput("Using Glew version %d (Cannot retrieve minor from '%s')\n", major, str);
        glewversion={major,0};
    }
    else
    {
        fmtoutput("Using Glew version '%s' (Cannot retrieve major.minor)\n", str);
        glewversion={};
    }
    if (glewversion.first>0)
    {
        glewinfo.glversion=glversion();
        glewinfo.glslversion=glslversion();
        if (glewinfo.glversion.first<3) fmtoutput("\nThis platform does not support OpenGL V3:\n"
            "FLTK widgets will appear but the programmed "
            "rendering pipeline will not run.\n");
    }
    return true;
}

static GLuint mkvertexshader(string_view source)
{
    const GLuint vs=glCreateShader(GL_VERTEX_SHADER);
    const char*src[]={source.data()};
    glShaderSource(vs, 1, src, NULL);
    glCompileShader(vs);
    GLint err;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &err);
    if (err!=GL_TRUE)
    {
        GLchar CLOG[1000];
        GLsizei length;
        glGetShaderInfoLog(vs, sizeof(CLOG), &length, CLOG);
        fmtoutput("vs ShaderInfoLog=%s\n", CLOG);
    }
    return vs;
}

static GLuint mkfragmentshader(string_view source)
{
    const GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);
    const char*src[]={source.data()};
    glShaderSource(fs, 1, src, NULL);
    glCompileShader(fs);
    GLint err;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &err);
    if (err!=GL_TRUE)
    {
        GLchar CLOG[1000];
        GLsizei length;
        glGetShaderInfoLog(fs, sizeof(CLOG), &length, CLOG);
        fmtoutput("fs ShaderInfoLog=%s\n", CLOG);
    }
    return fs;
}

GLuint build_program(string_view vssource, string_view fssource)
{
    const auto vs=mkvertexshader(vssource);
    const auto fs=mkfragmentshader(fssource);
    const GLuint p=glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint err;
    glGetProgramiv(p, GL_LINK_STATUS, &err);
    if (err!=GL_TRUE)
    {
        GLchar CLOG[1000];
        GLsizei length;
        glGetProgramInfoLog(p, sizeof(CLOG), &length, CLOG);
        fmtoutput("Link Log=%s\n", CLOG);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}
