
#include <GL/glew.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h> // for gl_texture_reset()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "bvhshow.h"
#include <functional>

// Probleme:
// cmu 15.02-15.04
// 29.24

using namespace std;
using namespace glm;

struct vertex { vec3 pos, color; };

vec3 eyevector(const vpstruct&X)
{
    const float x0=0;
    return {x0*cos(X.azim)+X.dist*sin(X.azim), X.elev, -x0*sin(X.azim)+cos(X.azim)*X.dist};
}

namespace ProgVC {

const string_view vs=R"__(#version 330
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
})__";

const string_view fs=R"__(#version 330
in vec4 FrontColor;
in vec4 BackColor;
layout(location=0) out vec4 FinalColor;
void main()
{
  if (gl_FrontFacing) FinalColor=FrontColor;
  else                FinalColor=BackColor;
}
)__";

NeuProg<vertex> buildprog()
{
    static_assert(sizeof(vertex)==6*sizeof(GLfloat));
    return {build_program(vs, fs), {
        {3,GL_FLOAT,GL_FALSE,sizeof(vertex),0},
        {3,GL_FLOAT,GL_FALSE,sizeof(vertex),3*sizeof(GLfloat)}
    }};
}

} // ProgVC

namespace {

vec3 recenter(const mat4&V, const mat4&P, const vec4&viewport, const vec3&eyepoint)
{
    const vec2 winpoint={
        viewport[0]+0.5*(eyepoint.x+1)*viewport[2],
        viewport[1]+0.5*(eyepoint.y+1)*viewport[3]
    };
    const vec3 a=glm::unProject({winpoint[0],winpoint[1],-1}, V, P, viewport);
    const vec3 b=glm::unProject({winpoint[0],winpoint[1],+1}, V, P, viewport);
    const auto wo=b+(eyepoint.z-b[1])/(a[1]-b[1])*(a-b);
    return wo;
}

static function<void()>rendermodel=[](){};
static function<void()>renderboundingbox=[](){};
static function<void()>rendertrace=[](){};

pair<vec3,vec3> bb;
const size_t tlen=1000;
vector<vertex> Trace(tlen);
mat4 Projection, View;

// =========================================================

class ModelWindow: public Fl_Gl_Window
{
    NeuProg<vertex> prog;
    GLint UnifProjection {-1}, UnifViewMatrix {-2};
    VAOBuffer VAOModel, VAOBox, VAOTrace;

public:
    ModelWindow(int x, int y, int w, int h);
    void draw(void) override;
    int handle(int event) override;
    void initmodel();
    void animatemodel();
};

ModelWindow::ModelWindow(int x, int y, int w, int h): Fl_Gl_Window(x, y, w, h)
{
    mode(FL_RGB8|FL_DOUBLE|FL_OPENGL3);
}

void ModelWindow::draw()
{
    static unsigned nc=0;
    ++nc;
    const auto w=pixel_w(), h=pixel_h();
    if (glewinfo.glversion.first>=3 && !isvalidprog(prog))
    {
        prog=ProgVC::buildprog();
        UnifProjection=glGetUniformLocation(prog.program, "Projection");
        UnifViewMatrix=glGetUniformLocation(prog.program, "ViewMatrix");
    }
    else if (!valid())
    {
        glViewport(0, 0, w, h);
    }
    if (!VAOTrace)
    {
        // static_assert(sizeof Trace==tlen*sizeof vertex);
        VAOTrace.generate();
        vertexspec(prog);
        for (auto j=0u; j<tlen; ++j)
        {
            const float k=0.5+j*0.5/100.0;
            Trace[j]={{-10+j*20.0/tlen, 0, 0}, {k,k,k}};
        }
        glBufferData(GL_ARRAY_BUFFER, tlen*sizeof(vertex), &Trace[0], GL_STATIC_DRAW);
        rendertrace=[prog=this->prog](){ useprog(prog); glDrawArrays(GL_POINTS, 0, tlen); };
    }
    if (!VAOBox)
    {
        #define WE 1,1,1
        #define ROT 1,0,0
        VAOBox.generate();
        vertexspec(prog);
        const float a=20, b=30;
        const GLfloat VertexData[]=
        {
            -a, 0, -a, ROT,
            -a, 0,  a, ROT,
            a, 0,  a, ROT,
            a, 0, -a, ROT,
            -a, 0, -a, ROT,
            -a, b, -a, ROT,
            -a, b,  a, ROT,
            a, b,  a, ROT,
            a, b, -a, ROT,
            -a, b, -a, ROT,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
        renderboundingbox=[prog=this->prog](){ useprog(prog); glDrawArrays(GL_LINE_STRIP, 0, 10); };
        #undef WE
        #undef ROT
    }
    if (!VAOModel)
    {
        #define WE 1,1,1
        #define ROT 1,0,0
        VAOModel.generate();
        vertexspec(prog);
        const GLfloat a=0.2, b=2;
        const GLfloat VertexData[]=
        {
            0, b, 0, ROT,
            -a, 0,-a, WE,
            -a, 0, a, WE,
            0, b, 0, ROT,
            a, 0,-a, WE,
            a, 0, a, WE,
            0, b, 0, ROT
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
        #undef WE
        #undef ROT
    }
    switch (frameinfo.state)
    {
        case frameinfo.init: initmodel(); frameinfo.state=frameinfo.animate; break;
        case frameinfo.animate: animatemodel(); break;
        case frameinfo.step: animatemodel(); break;
        case frameinfo.stop: animatemodel(); break;
    }
    Fl_Gl_Window::draw();
}

int ModelWindow::handle(int event)
{
    static bool firstcall=true;
    if (firstcall && event==FL_SHOW && shown())
    {
        firstcall=false;
        make_current();
        glewinitialise();
        if (glewinfo.glversion.first<3) mode(mode() & ~FL_OPENGL3);
    }
    return Fl_Gl_Window::handle(event);
}

void ModelWindow::initmodel()
{
#define ROT 1,0,0
#define WE 1,1,1
    if (frameinfo.Hier.size()>0)
    {
        const auto [a,b]=boundingbox(frameinfo.Hier, frameinfo.Motion);
        const GLfloat VertexData[]=
        {
            a[0], a[1], a[2], ROT,
            a[0], a[1], b[2], ROT,
            b[0], a[1], b[2], ROT,
            b[0], a[1], a[2], ROT,
            a[0], a[1], a[2], ROT,
            a[0], b[1], a[2], ROT,
            a[0], b[1], b[2], ROT,
            b[0], b[1], b[2], ROT,
            b[0], b[1], a[2], ROT,
            a[0], b[1], a[2], ROT,
        };
        VAOBox.bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
        frameinfo.vp.bbcenter=vec3(.5,.5,.5)*(a+b);
        frameinfo.vp.focus=frameinfo.vp.bbcenter;
        frameinfo.vp.elev=b[1];
        frameinfo.vp.dist=1.4*glm::length(b-a);
        bb={a,b};
    }
    else
    {
        const GLfloat a=0.2, b=2;
        const GLfloat VertexData[]=
        {
            0, b, 0, ROT,
            -a, 0,-a, WE,
            -a, 0, a, WE,
            0, b, 0, ROT,
            a, 0,-a, WE,
            a, 0, a, WE,
            0, b, 0, ROT
        };
        VAOBox.bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
        frameinfo.vp.dist=10;
    }
#undef ROT
#undef WE
}

void ModelWindow::animatemodel()
{
    vec3 K0={0,0,0};
    if (frameinfo.f<frameinfo.num)
    {
        const auto K=flatten(frameinfo.Hier, frameinfo.Motion[frameinfo.f]);
        if (K.size()>0) K0=K[0];
        static_assert(24==sizeof(vertex));
        vector<vertex>VertexData;
        unsigned j=0;
        const vec4 white(1,1,1,1);
        for (auto&s: frameinfo.Segments)
        {
            VertexData.push_back({K[s.first], white});
            VertexData.push_back({K[s.second], white});
            ++j;
        }
        VAOModel.bind();
        glBufferData(GL_ARRAY_BUFFER, VertexData.size()*sizeof(vertex), &VertexData[0], GL_STATIC_DRAW);
        rendermodel=[num=VertexData.size(), prog=this->prog](){ useprog(prog); glDrawArrays(GL_LINES, 0, num); };
    }
    // =================================================
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (true)
    {
        vec3 K0a={K0[0],bb.first[1],K0[2]};
        // This moves vp.focus so that the model remains close to the center of the window.
        auto pk=Projection*View*vec4(K0, 1);
        pk[1]=bb.first[1];
        const auto wo=recenter(View, Projection, frameinfo.viewport, pk);
        const auto focusneu=K0a;
        frameinfo.vp.focus=vec3(0.2, 0.2, 0.2)*frameinfo.vp.focus+vec3(0.8, 0.8, 0.8)*K0a;
        if (true)
        {
            static unsigned current=0;
            Trace[current]={K0a, {1.0,1.0,1.0}};
            for (unsigned j=0; j<tlen; ++j)
            {
                const float k=1.0-j*0.5/tlen;
                Trace[(current+j)%tlen].color={k,k,k};
            }
            current=(current+1)%tlen;
            VAOTrace.bind();
            glBufferData(GL_ARRAY_BUFFER, tlen*sizeof(vertex), &Trace[0], GL_STATIC_DRAW);
        }
    }
    const auto eye=eyevector(frameinfo.vp);
    Projection=glm::perspective(0.7f, 1.2f, 0.2f*frameinfo.vp.dist, 3.f*frameinfo.vp.dist);
    View=glm::lookAt(eye, frameinfo.vp.focus, {0,1,0});
    glUniformMatrix4fv(UnifProjection, 1, GL_FALSE, &Projection[0][0]);
    glUniformMatrix4fv(UnifViewMatrix, 1, GL_FALSE, &View[0][0]);
    VAOModel.bind(); rendermodel();
    VAOBox.bind(); renderboundingbox();
    VAOTrace.bind(); rendertrace();
}

} // anon

Fl_Window*NewModelWindow(int x, int y, int w, int h)
{
    auto*glwin=new ModelWindow(x,y,w,h);
    glwin->end();
    return glwin;
}
