
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

const GLfloat a=0.2, b=2;
const vec3 ROT={1,0,0}, WE={1,1,1};
const vector<vertex>Pyramid=
{
    {{ 0, b, 0}, ROT},
    {{-a, 0,-a}, WE},
    {{-a, 0, a}, WE},
    {{ 0, b, 0}, ROT},
    {{ a, 0,-a}, WE},
    {{ a, 0, a}, WE},
    {{ 0, b, 0}, ROT}
};

namespace BoundingBox
{
    const vector<vertex>&from_min_max(const vec3&a, const vec3&b)
    {
        static vector<vertex>X(10);
        const vec3 C={1,0,0};
        X[0]={{a[0], a[1], a[2]}, C};
        X[1]={{a[0], a[1], b[2]}, C};
        X[2]={{b[0], a[1], b[2]}, C};
        X[3]={{b[0], a[1], a[2]}, C};
        X[4]={{a[0], a[1], a[2]}, C};
        X[5]={{a[0], b[1], a[2]}, C};
        X[6]={{a[0], b[1], b[2]}, C};
        X[7]={{b[0], b[1], b[2]}, C};
        X[8]={{b[0], b[1], a[2]}, C};
        X[9]={{a[0], b[1], a[2]}, C};
        return X;
    }
};

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

pair<vec3,vec3> bb;
const size_t tlen=1000;
vector<vertex> Trace;
mat4 Projection, View;

static function<void()>rendermodel=[](){};
static function<void()>renderboundingbox=[](){};
static function<void()>rendertrace=[](){};

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
        VAOTrace.generate();
        vertexspec(prog);
        Trace=vector<vertex>(tlen, {{0,0,0},{1,1,1}});
        glBufferData(GL_ARRAY_BUFFER, tlen*sizeof(vertex), &Trace[0], GL_STATIC_DRAW);
        rendertrace=[prog=this->prog](){ useprog(prog); glDrawArrays(GL_POINTS, 0, tlen); };
    }
    if (!VAOBox)
    {
        VAOBox.generate();
        vertexspec(prog);
        const vec3 a={-2, 0, -2}, b={2, 3, 2};
        const auto V=BoundingBox::from_min_max(a,b);
        glBufferData(GL_ARRAY_BUFFER, V.size()*sizeof(vertex), &V[0], GL_STATIC_DRAW);
        renderboundingbox=[prog=this->prog, num=V.size()](){ useprog(prog); glDrawArrays(GL_LINE_STRIP, 0, num); };
    }
    if (!VAOModel)
    {
        // fmtoutput("draw init VAOModel\n");
        VAOModel.generate();
        vertexspec(prog);
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
    if (frameinfo.Hier.size()>0)
    {
        VAOBox.bind();
        const auto [a,b]=boundingbox(frameinfo.Hier, frameinfo.Motion);
        const auto V=BoundingBox::from_min_max(a,b);
        glBufferData(GL_ARRAY_BUFFER, V.size()*sizeof(vertex), &V[0], GL_STATIC_DRAW);
        rendermodel=[prog=this->prog, num=V.size()](){ useprog(prog); glDrawArrays(GL_LINES, 0, num); };
        frameinfo.vp.bbcenter=vec3(.5,.5,.5)*(a+b);
        frameinfo.vp.focus=frameinfo.vp.bbcenter;
        frameinfo.vp.elev=a[1]+1.1*(b[1]-a[1]);
        frameinfo.vp.dist=1.4*glm::length(b-a);
        bb={a,b};
    }
    else
    {
        VAOModel.bind();
        glBufferData(GL_ARRAY_BUFFER, Pyramid.size()*sizeof(vertex), &Pyramid[0], GL_STATIC_DRAW);
        rendermodel=[prog=this->prog](){ useprog(prog); glDrawArrays(GL_LINE_STRIP, 0, 7); };
        frameinfo.vp.elev=Pyramid[0].pos.y+0.1*(Pyramid[0].pos.y-Pyramid[1].pos.y);
        frameinfo.vp.dist=10;
    }
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
        frameinfo.vp.focus=vec3(0.2, 0.2, 0.2)*frameinfo.vp.focus+vec3(0.8, 0.8, 0.8)*K0a;
        if (Trace.size()>=tlen)
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
