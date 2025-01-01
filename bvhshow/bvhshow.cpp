
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

FrameInfo frameinfo;

struct {
    Fl_Window*topwin {nullptr};
    Fl_Widget*modelview {nullptr};
    Fl_Text_Display*textwidget {nullptr};
    Fl_File_Chooser*fileselect {nullptr};
    Fl_Pack*stepper {nullptr};
    Fl_Progress*progress {nullptr};
    char pad_progress[100];
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
        case frameinfo.initdummy:
        {
            GE.progress->label("dummy");
            GE.progress->value(0);
            GE.modelview->draw();
            Fl::repeat_timeout(frameinfo.dt, cbredraw, data);
            break;
        }
        case frameinfo.initmodel:
        {
            GE.progress->label("loading");
            GE.progress->value(0);
            GE.modelview->draw();
            Fl::repeat_timeout(frameinfo.dt, cbredraw, data);
            break;
        }
        case frameinfo.animatedummy:
        {
            GE.modelview->draw();
            Fl::repeat_timeout(frameinfo.dt, cbredraw, data);
            break;
        }
        case frameinfo.animatemodel:
        {
            const int step=frameinfo.animmode==frameinfo.back?-1:1;
            frameinfo.f=frameinfo.dt>=frameinfo.num?0:(frameinfo.f+step)%frameinfo.num;
            sprintf(GE.pad_progress, "%.1fs %u/%u", frameinfo.f*frameinfo.dt, frameinfo.f+1, frameinfo.num);
            GE.progress->label(GE.pad_progress);
            GE.progress->value(frameinfo.f);
            GE.modelview->draw();
            if (frameinfo.animmode==frameinfo.run) Fl::repeat_timeout(frameinfo.dt, cbredraw, data);
            break;
        }
        case frameinfo.stop:
        {
            // GE.progress->label("stopped");
            // GE.progress->value(0);
            break;
        }
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
            if (!filename.empty())
            {
                BVHScene*LoadedScene=parse(filename.c_str());
                if (LoadedScene!=nullptr)
                {
                    fmtoutput("Duration: %.1f sec (%.0f fps)\n", LoadedScene->totaltime, LoadedScene->M.size()/LoadedScene->totaltime);
                    frameinfo.viewport=glm::vec4 {0,0,450,280};
                    frameinfo.Hier=LoadedScene->H;
                    frameinfo.Segments=segments(frameinfo.Hier);
                    frameinfo.Motion=LoadedScene->M;
                    frameinfo.num=LoadedScene->M.size();
                    frameinfo.dt=LoadedScene->totaltime/LoadedScene->M.size();
                    frameinfo.f=frameinfo.num;
                    frameinfo.state=frameinfo.initmodel;
                    delete LoadedScene;
                    GE.topwin->copy_label(filename.c_str());
                    GE.progress->minimum(0);
                    GE.progress->maximum(frameinfo.num);
                    GE.stepper->hide();
                    Fl::repeat_timeout(0.1, cbredraw, (void*)GE.topwin);
                }
            }
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
        frameinfo.state=frameinfo.animatemodel;
        frameinfo.animmode=frameinfo.run;
        widget->label("@||");
        GE.stepper->hide();
        Fl::repeat_timeout(frameinfo.dt, cbredraw, &GE.topwin);
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
        frameinfo.animmode=frameinfo.forward;
        frameinfo.state=frameinfo.animatemodel;
        Fl::repeat_timeout(frameinfo.dt, cbredraw, &GE.topwin);
    }
    else if (0==strcmp(widget->label(), "@<-"))
    {
        frameinfo.animmode=frameinfo.back;
        frameinfo.state=frameinfo.animatemodel;
        Fl::repeat_timeout(frameinfo.dt, cbredraw, &GE.topwin);
    }
}

int main(int argc, char**argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    Fl_File_Icon::load_system_icons();
    Fl::use_high_res_GL(1);

    GE.fileselect=new Fl_File_Chooser(nullptr, "*", Fl_File_Chooser::SINGLE, "Select bvh file");
    // GE.fileselect->callback(fc_callback);

    frameinfo.state=frameinfo.initdummy;

    GE.topwin=new Fl_Window(800, 400);
    GE.topwin->callback(cbtop);
    GE.modelview=NewModelWindow(0, 0, 450, 380);
    if (auto p=new Fl_Progress(0, 380, 450, 20); p!=nullptr)
    {
        p->minimum(0);
        p->maximum(100);
        p->color(0x88888800);               // background color
        p->selection_color(0x4444ff00);     // progress bar color
        p->labelcolor(FL_WHITE);            // text color
    //  p->label("0/100");                  // update progress bar's label
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
    GE.topwin->show(argc, argv);
    frameinfo.state=frameinfo.initdummy;
    Fl::repeat_timeout(0.1, cbredraw, (void*)GE.topwin);
    Fl::run();
}
