
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

vec3 eyevector(const vpstruct&X)
{
    const float x0=0;
    return vec3 {x0*cos(X.azim)+X.dist*sin(X.azim), X.elev, -x0*sin(X.azim)+cos(X.azim)*X.dist};
}

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

static function<void()>rendermodel=[](){ glDrawArrays(GL_LINE_STRIP, 0, 7); };
static function<void()>renderboundingbox=[](){ glDrawArrays(GL_LINE_STRIP, 0, 10); };
static function<void()>rendertrace=[](){ glDrawArrays(GL_LINE_STRIP, 0, 100); };

struct vertex { vec4 pos, color; };

static struct {
    string_view vs, fs;
} source={

R"__(#version 330
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
})__",

R"__(#version 330
in vec4 FrontColor;
in vec4 BackColor;
layout(location=0) out vec4 FinalColor;
void main()
{
  if (gl_FrontFacing) FinalColor=FrontColor;
  else                FinalColor=BackColor;
}
)__"
};

pair<vec3,vec3> bb;

const size_t tlen=1000;
vector<vertex> Trace(tlen);

mat4 Projection, View;

struct proginfo
{
    GLuint progname {0};
    GLint UnifProjection {-1}, UnifViewMatrix {-2};
    GLint AttrPosition {-1}, AttrColour {-1};
    operator bool()const{ return progname!=0; }
    void activate()const;
};

void proginfo::activate()const
{
    glEnableVertexAttribArray((GLuint)AttrPosition);
    glEnableVertexAttribArray((GLuint)AttrColour);
    glVertexAttribPointer((GLuint)AttrPosition, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer((GLuint)AttrColour,   4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (char*)0+4*sizeof(GLfloat));
    glUseProgram(progname);
}

// =========================================================

class ModelWindow: public Fl_Gl_Window
{
    proginfo prog {};
    VAOBuffer VAOModel;
    GLuint VAOBox, VAOTrace;
    GLuint VBBox,  VBTrace;

public:
    ModelWindow(int x, int y, int w, int h);
    void draw(void) override;
    int handle(int event) override;
    void reset();
    void bind_boundingbox()const
    {
        glBindVertexArray(VAOBox);
        glBindBuffer(GL_ARRAY_BUFFER, VBBox);
    }
    void bind_trace()const
    {
        glBindVertexArray(VAOTrace);
        glBindBuffer(GL_ARRAY_BUFFER, VBTrace);
    }
    void inittrace()
    {
        // static_assert(sizeof Trace==tlen*sizeof vertex);
        glGenVertexArrays(1, &VAOTrace); glBindVertexArray(VAOTrace);
        glGenBuffers(1, &VBTrace);       glBindBuffer(GL_ARRAY_BUFFER, VBTrace);
        for (auto j=0u; j<tlen; ++j)
        {
            const float k=0.5+j*0.5/100.0;
            Trace[j]={vec4 {-10+j*20.0/tlen, 0, 0, 1}, vec4 {k,k,k,1}};
        }
        glBufferData(GL_ARRAY_BUFFER, tlen*sizeof(vertex), &Trace[0], GL_STATIC_DRAW);
        prog.activate();
        rendertrace=[num=current](){ glDrawArrays(GL_LINES, 0, 100); };
    }
    void initbox()
    {
        #define WE 1,1,1,1
        #define ROT 1,0,0,1
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
        #undef WE
        #undef ROT
    }
    void initgeom()
    {
        #define WE 1,1,1,1
        #define ROT 1,0,0,1
        VAOModel.generate();
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
        prog.activate();
        #undef WE
        #undef ROT
    }
    void initmodel()
    {
        #define ROT 1,0,0,1
        #define WE 1,1,1,1
        if (frameinfo.Hier.size()>0)
        {
            const auto [a,b]=boundingbox(frameinfo.Hier, frameinfo.Motion);
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
                0, b, 0, 1, ROT,
                -a, 0,-a, 1, WE,
                -a, 0, a, 1, WE,
                0, b, 0, 1, ROT,
                a, 0,-a, 1, WE,
                a, 0, a, 1, WE,
                0, b, 0, 1, ROT
            };
            glBindVertexArray(VAOBox);
            glBindBuffer(GL_ARRAY_BUFFER, VBBox);
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
            prog.activate();
            frameinfo.vp.dist=10;
        }
        #undef ROT
        #undef WE
    }
    void animatemodel()
    {
        vec3 K0={0,0,0};
        if (frameinfo.f<frameinfo.num)
        {
            const auto K=flatten(frameinfo.Hier, frameinfo.Motion[frameinfo.f]);
            if (K.size()>0) K0=K[0];
            static_assert(32==sizeof(vertex));
            vector<vertex>VertexData;
            unsigned j=0;
            const vec4 white(1,1,1,1);
            for (auto&s: frameinfo.Segments)
            {
                const auto a=K[s.first], b=K[s.second];
                VertexData.push_back({vec4(a,1.0), white});
                VertexData.push_back({vec4(b,1.0), white});
                ++j;
            }
            VAOModel.bind();
            glBufferData(GL_ARRAY_BUFFER, VertexData.size()*sizeof(vertex), &VertexData[0], GL_STATIC_DRAW);
            rendermodel=[num=VertexData.size()](){ glDrawArrays(GL_LINES, 0, num); };
        }
        // =================================================
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (true)
        {
            vec3 K0a={K0[0],bb.first[1],K0[2]};
            // This should move vp.focus so that the model remains close to the center of the window.
            // Does not work yet.
            auto pk=Projection*View*vec4(K0, 1);
            pk[1]=bb.first[1];
            const auto wo=recenter(View, Projection, frameinfo.viewport, pk);
            // vp.focus=vec3(0.9, 0.9, 0.9)*vp.focus+vec3(0.1, 0.1, 0.1)*wo;
            const auto focusneu=K0a; // vec3(0.6, 0.6, 0.6)*frameinfo.vp.focus+vec3(0.4, 0.4, 0.4)*K0a;
            frameinfo.vp.focus=vec3(0.2, 0.2, 0.2)*frameinfo.vp.focus+vec3(0.8, 0.8, 0.8)*K0a;
            // printf("%.2f %.2f %.2f\n", wo[0], wo[1], wo[2]);
            // printf("%.2f %.2f %.2f\n", vp.focus[0], vp.focus[1], vp.focus[2]);
            if (true)
            {
                static unsigned current=0;
                Trace[current]={vec4(K0a,1), {1.0,1.0,1.0,1.0}};
                for (unsigned j=0; j<tlen; ++j)
                {
                    const float k=1.0-j*0.5/tlen;
                    Trace[(current+j)%tlen].color=vec4 {k,k,k,1};
                }
                current=(current+1)%tlen;
                glBindBuffer(GL_ARRAY_BUFFER, VBTrace);
                glBufferData(GL_ARRAY_BUFFER, tlen*sizeof(vertex), &Trace[0], GL_STATIC_DRAW);
                rendertrace=[num=current](){ glDrawArrays(GL_POINTS, 0, current); glDrawArrays(GL_POINTS, current, tlen-current); };
            }
// fmtoutput("K0 %.2g %.2g %.2g pk %.2g %.2g %.2g\n", K0[0], K0[1], K0[2], pk[0], pk[1], pk[2]);
// fmtoutput("K0 %.2f %.2f %.2f ==> %.2f %.2f %.2f\n", K0[0], K0[1], K0[2], wo[0], wo[1], wo[2]);
        }
        const auto eye=eyevector(frameinfo.vp);
        Projection=glm::perspective(0.7f, 1.2f, 0.2f*frameinfo.vp.dist, 3.f*frameinfo.vp.dist);
        View=glm::lookAt(eye, frameinfo.vp.focus, vec3 {0,1,0});
        glUniformMatrix4fv(prog.UnifProjection, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(prog.UnifViewMatrix, 1, GL_FALSE, &View[0][0]);
        VAOModel.bind(); rendermodel();
        bind_boundingbox(); renderboundingbox();
        bind_trace(); rendertrace();
    }
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
    if (glewinfo.glversion.first>=3 && !prog)
    {
        const auto p=build_program(source.vs, source.fs);
        prog={
            p,
            glGetUniformLocation(p, "Projection"), glGetUniformLocation(p, "ViewMatrix"),
            glGetAttribLocation(p, "XPosition"), glGetAttribLocation(p, "XColor")
        };
    }
    else if (!valid())
    {
        glViewport(0, 0, w, h);
    }
    if (VAOTrace==0) inittrace();
    if (VAOBox==0) initbox();
    if (!VAOModel) initgeom();
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

void ModelWindow::reset()
{
    prog={0,0,0,0};
    gl_texture_reset();
}

} // anon

Fl_Window*NewModelWindow(int x, int y, int w, int h)
{
    auto*glwin=new ModelWindow(x,y,w,h);
    // Fl::set_color(FL_FREE_COLOR, 255, 255, 255, 140); // partially transparent white
    glwin->end();
    return glwin;
}
