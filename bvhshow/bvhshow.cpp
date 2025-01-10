
#include <string_view>
// #include <cstdarg>
// #include <FL/Fl.H>
// #include <FL/platform.H>
// #include <FL/Fl_Window.H>
// #include <FL/Fl_Text_Buffer.H>
// #include <FL/Fl_Pixmap.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Widget.h>
#include <FL/Fl_Pack.H>
#include <GL/glew.h>
#include "bvhshow.h"

// Using OpenGL in fltk (mostly old OpenGL)
// https://fltk.gitlab.io/fltk/opengl.html

using namespace std;

FrameInfo frameinfo;
const auto delay_idle=1./30.;

struct {
    string arg_filename;
    Fl_Window*topwin {nullptr};
    Fl_Widget*modelview {nullptr};
    Fl_Text_Display*textwidget {nullptr};
    Fl_File_Chooser*fileselect {nullptr};
    Fl_Pack*stepper {nullptr};
    Fl_Progress*progress {nullptr};
} GE;

void fmtoutput(const char*format, ...)
{
    va_list args;
    char line_buffer[10000];
    va_start(args, format);
    vsnprintf(line_buffer, sizeof(line_buffer)-1, format, args);
    va_end(args);
    GE.textwidget->buffer()->append(line_buffer);
    GE.textwidget->scroll(10000, 0);
    GE.textwidget->redraw();
}

static void cbredraw(void*data)
{
    switch (frameinfo.state)
    {
        case frameinfo.init:
        {
            GE.progress->label(GE.arg_filename.empty()?"dummy":"loading");
            GE.progress->value(0);
            GE.modelview->redraw();
            Fl::repeat_timeout(frameinfo.timestep, cbredraw, data);
            break;
        }
        case frameinfo.animate:
        {
            frameinfo.vp.azim+=0.006;
            frameinfo.f=frameinfo.num>0?(frameinfo.f+1)%frameinfo.num:0;
            static char pad[100];
            sprintf(pad, "%.1fs     %u/%u", frameinfo.f*frameinfo.timestep, frameinfo.f+1, frameinfo.num);
            GE.progress->label(pad);
            GE.progress->value(frameinfo.f);
            GE.modelview->redraw();
            Fl::repeat_timeout(frameinfo.timestep, cbredraw, data);
            break;
        }
        case frameinfo.step:
        {
            if (frameinfo.num>0)
            {
                // frameinfo.vp.azim+=0.006;
                const int step=frameinfo.animdir==frameinfo.back?-1:1;
                frameinfo.f=(frameinfo.f+step)%frameinfo.num;
                static char pad[100];
                sprintf(pad, "%.1fs     %u/%u", frameinfo.f*frameinfo.timestep, frameinfo.f+1, frameinfo.num);
                GE.progress->label(pad);
                GE.progress->value(frameinfo.f);
                GE.modelview->redraw();
            }
            frameinfo.state=frameinfo.stop;
            Fl::repeat_timeout(delay_idle, cbredraw, data);
            break;
        }
        case frameinfo.stop:
        {
            GE.modelview->redraw();
            Fl::repeat_timeout(delay_idle, cbredraw, data);
            break;
        }
    }
}

static void bvhload(string_view filename)
{
    BVHScene*LoadedScene=parse(filename.data());
    fmtoutput("bvhload %p '%s'\n", LoadedScene, filename.data());
    if (LoadedScene!=nullptr)
    {
        fmtoutput("Duration: %.1f sec (%.0f fps)\n", LoadedScene->totaltime, LoadedScene->M.size()/LoadedScene->totaltime);
        frameinfo.viewport=glm::vec4 {0,0,450,280};
        frameinfo.Hier=LoadedScene->H;
        frameinfo.Segments=segments(frameinfo.Hier);
        frameinfo.Motion=LoadedScene->M;
        frameinfo.num=LoadedScene->M.size();
        frameinfo.timestep=LoadedScene->totaltime/LoadedScene->M.size();
        frameinfo.f=frameinfo.num;
        frameinfo.state=frameinfo.init;
        delete LoadedScene;
        GE.topwin->copy_label(filename.data());
        GE.progress->minimum(0);
        GE.progress->maximum(frameinfo.num);
        GE.stepper->hide();
    }
}

static void cbbuttons(Fl_Widget*widget, void*ctx)
{
    auto str=ctx==nullptr?widget->label():(const char*)ctx;
    if (str=="select_bvh")
    {
        GE.fileselect->filter("*.bvh");
        GE.fileselect->show();
        while (GE.fileselect->visible()) Fl::wait();
        if (const int count=GE.fileselect->count(); count>0)
        {
            std::string filename=GE.fileselect->value(1);
            if (!filename.empty()) bvhload(filename);
        }
    }
    else if (str=="@||")
    {
        frameinfo.state=frameinfo.stop;
        widget->label("@>");
        GE.stepper->show();
    }
    else if (str=="@>")
    {
        frameinfo.state=frameinfo.animate;
        frameinfo.animdir=frameinfo.forward;
        widget->label("@||");
        GE.stepper->hide();
    }
    else fmtoutput("Run callback for %s\n", str);
}

static void cbtop(Fl_Widget*widget, void*)
{
    fmtoutput("cbtop called.\n");
    // Prevent ModelView from using pointer after release?
    ((Fl_Window*)widget)->hide();
}

static void cbtoolbox(Fl_Widget*widget, void*)
{
    if (0==strcmp(widget->label(), "@->"))
    {
        frameinfo.animdir=frameinfo.forward;
        frameinfo.state=frameinfo.step;
    }
    else if (0==strcmp(widget->label(), "@<-"))
    {
        frameinfo.animdir=frameinfo.back;
        frameinfo.state=frameinfo.step;
    }
}

struct mainwindow: public Fl_Window
{
    mainwindow(int w, int h): Fl_Window(w, h){}
    int handle(int event) override
    {
        // Override to process command line after creation of user interface.
        const auto rc=Fl_Window::handle(event);
        static bool firstcall=true;
        if (firstcall && event==FL_SHOW && shown())
        {
            firstcall=false;
            if (!GE.arg_filename.empty()) bvhload(GE.arg_filename);
        }
        return rc;
    }
};

int main(int argc, char**argv)
{
    if (argc>1) GE.arg_filename=argv[1];

    setvbuf(stdout, nullptr, _IONBF, 0);
    Fl_File_Icon::load_system_icons();
    Fl::use_high_res_GL(1);

    GE.fileselect=new Fl_File_Chooser(nullptr, "*", Fl_File_Chooser::SINGLE, "Select bvh file");
    // GE.fileselect->callback(fc_callback);

    frameinfo.state=frameinfo.init;
    frameinfo.timestep=1.0/30.0;

    GE.topwin=new mainwindow(800, 400);
    GE.topwin->callback(cbtop);
    GE.modelview=NewModelWindow(0, 0, 450, 380);
    if (auto p=new Fl_Progress(0, 380, 450, 20); p!=nullptr)
    {
        p->minimum(0);
        p->maximum(100);
        p->color(0x88888800);               // background color
        p->selection_color(0x4444ff00);     // progress bar color
        p->labelcolor(FL_WHITE);            // text color
        GE.progress=p;
    }
 
    if (auto*g=new Fl_Window(450,0,350,400); g!=nullptr)
    {
        GE.textwidget=new Fl_Text_Display(0,0,350,200);
        GE.textwidget->buffer(new Fl_Text_Buffer());
        GE.textwidget->end();
        if (Fl_Button*fileselect=new Fl_Button(350-100-10, 400-20-30-20, 100, 30, ".."); fileselect!=nullptr)
        {
            fileselect->color(FL_FREE_COLOR);
            fileselect->box(FL_BORDER_BOX);
            fileselect->callback(cbbuttons, (void*)"select_bvh");
            if (auto icon=Fl_File_Icon::find(".", Fl_File_Icon::DIRECTORY); icon!=nullptr)
            {
                fileselect->labelcolor(FL_YELLOW);
                icon->label(fileselect);
            }
        }
        if (Fl_Button*startstop=new Fl_Button(10, 400-20-30-20, 100, 30, "@||"); startstop!=nullptr)
        {
            startstop->color(FL_FREE_COLOR);
            startstop->box(FL_BORDER_BOX);
            // startstop->image();
            startstop->callback(cbbuttons, NULL);
        }
        if (auto p=new Fl_Pack(10,400-20-10,100,20); p!=nullptr)
        {
            p->type(Fl_Pack::HORIZONTAL);
            p->box(FL_UP_FRAME);
            p->spacing(55);
            p->hide();
            for (auto g: {"@<-","@->"})
            {
                Fl_Button*tb=new Fl_Button(0,0,20,20,g);
                tb->callback(cbtoolbox);
                tb->box(FL_FLAT_BOX);
                // tb->clear_visible_focus();
            }
            p->end();
            GE.stepper=p;
        }
        g->end();
    }
    GE.topwin->end();
    GE.topwin->resizable(GE.modelview);
    GE.topwin->label("(No File) BVHShow");
    GE.topwin->show();
    Fl::add_timeout(0.1, cbredraw, (void*)GE.topwin);
    Fl::run();
}
