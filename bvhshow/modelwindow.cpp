
#include <GL/glew.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h> // for gl_texture_reset()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <parser.h>
#include "flshow.h"

using namespace std;
using namespace glm;

extern BVHScene*LoadedScene;

Hierarchy Hier;
MotionTable Motion;
vector<pair<unsigned,unsigned>> Segments;

function<void()>rendermodel=[](){ glDrawArrays(GL_LINE_STRIP, 0, 7); };
function<void()>renderboundingbox=[](){ glDrawArrays(GL_LINE_STRIP, 0, 10); };

static struct {
    float dist, elev, azim;
    vec3 focus;
} vp;

static const char*mkvertexshadersource()
{
    static char pad[512];
    sprintf_s(pad, sizeof pad, R"__(#version 330
layout(location=0) in vec3 XPosition;
layout(location=1) in vec3 XColor;
out vec4 FrontColor;
out vec4 BackColor;
uniform mat4 Projection;
uniform mat4 ViewMatrix;
void main()
{
    FrontColor=clamp(vec4(XColor,1), 0, 1);
    BackColor=FrontColor;
    gl_Position=Projection*ViewMatrix*vec4(XPosition,1);
})__");
    return pad;
}

static const char*mkfragmentshadersource()
{
    static char pad[512];
    sprintf_s(pad, sizeof pad, R"__(#version 330
in vec4 FrontColor;
in vec4 BackColor;
layout(location=0) out vec4 FinalColor;
void main()
{
  if (gl_FrontFacing) FinalColor=FrontColor;
  else                FinalColor=BackColor;
}
)__");
    return pad;
}

namespace {

struct proginfo
{
    GLuint progname {0};
    GLint UnifProjection {-1}, UnifViewMatrix {-2};
    GLint AttrPosition {-1}, AttrColour {-1};

//  proginfo(){}
//  proginfo(GLuint p, GLint upos, GLint acolor, GLint aposition):
//      progname(p), positionUniform(upos), colourAttribute(acolor), positionAttribute(aposition){}
    operator bool()const{ return progname!=0; }
//  void setposition(GLfloat x, GLfloat y) const;
    void activate()const;
};

static proginfo build_program()
{
    const auto vs=mkvertexshader(mkvertexshadersource());
    const auto fs=mkfragmentshader(mkfragmentshadersource());
    const auto p=linkshaderprogram(vs, fs, {"FinalColor"});
    glDeleteShader(vs);
    glDeleteShader(fs);
    return {
        p,
        glGetUniformLocation(p, "Projection"), glGetUniformLocation(p, "ViewMatrix"),
        glGetAttribLocation(p, "XPosition"), glGetAttribLocation(p, "XColor")
    };
}

// void proginfo::setposition(GLfloat x, GLfloat y)const
// {
//     const GLfloat p[]={x,y};
//     glUniform2fv(positionUniform, 1, p);
// }

void proginfo::activate()const
{
    glEnableVertexAttribArray((GLuint)AttrPosition);
    glEnableVertexAttribArray((GLuint)AttrColour);
    glVertexAttribPointer((GLuint)AttrPosition, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer((GLuint)AttrColour,   4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (char*)0+4*sizeof(GLfloat));
    glUseProgram(progname);
}

} // anon

class ModelWindow: public Fl_Gl_Window
{
    proginfo prog {};
    GLuint VAOModel, VAOBox;
    GLuint VBModel,  VBBox;

public:
    ModelWindow(int x, int y, int w, int h);
    void draw(void) override;
    int handle(int event) override;
    void reset();
    void bind_model()const
    {
        glBindVertexArray(VAOModel);
        glBindBuffer(GL_ARRAY_BUFFER, VBModel);
    }
    void bind_boundingbox()const
    {
        glBindVertexArray(VAOBox);
        glBindBuffer(GL_ARRAY_BUFFER, VBBox);
    }
};

void cbredraw(void*data)
{
    static unsigned nc=0;
    auto*W=reinterpret_cast<ModelWindow*>(data);
    // fmtoutput("redraw %p %u\n", W, ++nc);
    W->draw();
    Fl::repeat_timeout(1.0/60.0, cbredraw, data);
}

ModelWindow::ModelWindow(int x, int y, int w, int h): Fl_Gl_Window(x, y, w, h)
{
    mode(FL_RGB8|FL_DOUBLE|FL_OPENGL3);
}

void ModelWindow::draw(void)
{
    static unsigned nc=0;
    ++nc;
    const auto w=pixel_w(), h=pixel_h();
    if (glewinfo.glversion.first>=3 && !prog)
    {
        #define WE 1,1,1,1
        #define ROT 1,0,0,1

        prog=build_program();

        if (true)
        {
            glGenVertexArrays(1, &VAOBox); glBindVertexArray(VAOBox);
            glGenBuffers(1, &VBBox);       glBindBuffer(GL_ARRAY_BUFFER, VBBox);
            const float a=20, b=30;
            const GLfloat VertexData[]=
            {
                -a, 0, -a, 1, ROT,
                -a, 0,  a, 1, ROT,
                 a, 0,  a, 1, ROT,
                 a, 0, -a, 1, ROT,
                -a, 0, -a, 1, ROT,
                -a, b, -a, 1, ROT,
                -a, b,  a, 1, ROT,
                 a, b,  a, 1, ROT,
                 a, b, -a, 1, ROT,
                -a, b, -a, 1, ROT,
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
            prog.activate();
        }

        // fmtoutput("%u build program ==> %u\n", nc, prog.progname);
        // fmtoutput("draw first %u (%d %d %d)\n", prog.progname, prog.positionUniform, prog.colourAttribute, prog.positionAttribute);
        if (true)
        {
            glGenVertexArrays(1, &VAOModel); glBindVertexArray(VAOModel);
            glGenBuffers(1, &VBModel);       glBindBuffer(GL_ARRAY_BUFFER, VBModel);
#if 1
            const GLfloat a=0.2, b=2;
            const GLfloat VertexData[]=
            {
                 0, b, 0, 1, ROT,
                -a, 0,-a, 1, WE,
                -a, 0, a, 1, WE,
                 0, b, 0, 1, ROT,
                 a, 0,-a, 1, WE,
                 a, 0, a, 1, WE,
                 0, b, 0, 1, ROT
            };
#else
            const GLfloat a=0.2, b=1.6, z=0;
            const GLfloat VertexData[]=
            {
                 0,  0, b, 1, ROT,
                -a,  a, 0, 1, WE,
                -a, -a, 0, 1, WE,
                 0,  0, b, 1, ROT,
                 a, -a, 0, 1, WE,
                 a,  a, 0, 1, WE,
                 0,  0, b, 1, ROT
            };
#endif
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
            prog.activate();
            vp.dist=10;
            vp.azim=0.2;
        }
    }
    else if (!valid())
    {
        glViewport(0, 0, w, h);
    }
    if (true)
    {
        // fmtoutput("%u %s\n", nc, "draw");
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (prog)
        {
            // prog.setposition(0,0);
            // fmtoutput("draw sonst %u (%d %d %d)\n", prog.progname, prog.positionUniform, prog.colourAttribute, prog.positionAttribute);
            vp.azim+=0.002;
            const float x0=0;
            glm::vec3 eye {x0*cos(vp.azim)+vp.dist*sin(vp.azim), vp.elev, -x0*sin(vp.azim)+cos(vp.azim)*vp.dist}, center {0,1,0}, up {0,1,0};
            glm::mat4
                P=glm::perspective(0.7f, 1.2f, 0.5f*vp.dist, 2.f*vp.dist),
                V=glm::lookAt(eye, center, up);
            glUniformMatrix4fv(prog.UnifProjection, 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(prog.UnifViewMatrix, 1, GL_FALSE, &V[0][0]);

            // =================================================

            static unsigned motionindex=0, motiondiv=0;
            if (auto LS=LoadedScene; LS!=nullptr)
            {
                LoadedScene=nullptr;
                Hier=LS->H;
                Motion=LS->M;
                delete LS;
                motionindex=0;
                Segments=segments(Hier);
                const auto K=flatten(Hier, Motion[motionindex]);
                struct vertex { vec4 pos, color; };
                static_assert(32==sizeof vertex);
                vector<vertex>VertexData;
                unsigned j=0;
                const vec4 white(1,1,1,1);
                for (auto&s: Segments)
                {
                    const auto a=K[s.first], b=K[s.second];
                    VertexData.push_back({vec4(a,1.0), white});
                    VertexData.push_back({vec4(b,1.0), white});
                    ++j;
                }
                glBindBuffer(GL_ARRAY_BUFFER, VBModel);
                glBufferData(GL_ARRAY_BUFFER, VertexData.size()*sizeof vertex, &VertexData[0], GL_STATIC_DRAW);
                rendermodel=[num=VertexData.size()](){ glDrawArrays(GL_LINES, 0, num); };

                if (true)
                {
                    const auto [a,b]=boundingbox(Hier, Motion);
                    const GLfloat VertexData[]=
                    {
                        a[0], a[1], a[2], 1, ROT,
                        a[0], a[1], b[2], 1, ROT,
                        b[0], a[1], b[2], 1, ROT,
                        b[0], a[1], a[2], 1, ROT,
                        a[0], a[1], a[2], 1, ROT,
                        a[0], b[1], a[2], 1, ROT,
                        a[0], b[1], b[2], 1, ROT,
                        b[0], b[1], b[2], 1, ROT,
                        b[0], b[1], a[2], 1, ROT,
                        a[0], b[1], a[2], 1, ROT,
                    };

                    glBindVertexArray(VAOBox);
                    glBindBuffer(GL_ARRAY_BUFFER, VBBox);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
                    prog.activate();
                    vp.focus=vec3(.5,.5,.5)*(a+b);
                    vp.elev=b[1];
                    vp.dist=1.5*glm::length(b-a);
                }
            }
            else if (motiondiv<4) ++motiondiv;
            else if (motionindex+1<Motion.size())
            {
                motiondiv=0;
                ++motionindex;
                const auto K=flatten(Hier, Motion[motionindex]);
                struct vertex { vec4 pos, color; };
                static_assert(32==sizeof vertex);
                vector<vertex>VertexData;
                unsigned j=0;
                const vec4 white(1,1,1,1);
                for (auto&s: Segments)
                {
                    const auto a=K[s.first], b=K[s.second];
                    VertexData.push_back({vec4(a,1.0), white});
                    VertexData.push_back({vec4(b,1.0), white});
                    ++j;
                }
                glBindBuffer(GL_ARRAY_BUFFER, VBModel);
                glBufferData(GL_ARRAY_BUFFER, VertexData.size()*sizeof vertex, &VertexData[0], GL_STATIC_DRAW);
                rendermodel=[num=VertexData.size()](){ glDrawArrays(GL_LINES, 0, num); };
            }
            else if (Motion.size()>0) motionindex=0;
            bind_model(); rendermodel();
            bind_boundingbox(); renderboundingbox();
        }
    }
    Fl_Gl_Window::draw();
}

int ModelWindow::handle(int event)
{
    static bool firstcall=true;
    if (firstcall && event==FL_SHOW && shown())
    {
        // fmtoutput("handle %d first\n", event);
        firstcall=false;
        make_current();
        initialise_glew();
        if (glewinfo.glversion.first<3) mode(mode() & ~FL_OPENGL3);
        Fl::repeat_timeout(1.0, cbredraw, (void*)this);
        // fmtoutput("start redraw %p\n", this);
    }
    return Fl_Gl_Window::handle(event);
}

void ModelWindow::reset()
{
    prog={0,0,0,0};
    gl_texture_reset();
}

Fl_Window*NewModelWindow(int x, int y, int w, int h)
{
    auto*glwin=new ModelWindow(x,y,w,h);
    // Fl::set_color(FL_FREE_COLOR, 255, 255, 255, 140); // partially transparent white
    glwin->end();
    return glwin;
}
