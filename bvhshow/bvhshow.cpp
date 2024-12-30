
#include <cstdarg>
// #include <FL/Fl.H>
// #include <FL/platform.H>
// #include <FL/Fl_Window.H>
// #include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Widget.h>
#include <GL/glew.h>
#include <parser.h>
#include "flshow.h"

BVHScene*LoadedScene=nullptr;
FrameInfo frameinfo;

struct {
    Fl_Window*topwin {nullptr};
    Fl_Widget*modelview {nullptr};
    Fl_Text_Display*textwidget {nullptr};
    Fl_File_Chooser*fileselect {nullptr};
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
    static unsigned nc=0;
    GE.modelview->draw();
    frameinfo.f=(frameinfo.f+1)%frameinfo.num;
    sprintf(GE.pad_progress, "%u/%u", frameinfo.f+1, frameinfo.num);
    GE.progress->label(GE.pad_progress);
    GE.progress->value(frameinfo.f);
    Fl::repeat_timeout(frameinfo.dt, cbredraw, data);
}

static void cbbuttons(Fl_Widget*widget, void*ctx)
{
    auto str=ctx==nullptr?widget->label():(const char*)ctx;
    if (str=="select_bvh")
    {
        GE.fileselect->filter("*.bvh");
        GE.fileselect->show();
        while (GE.fileselect->visible()) Fl::wait();
        const int count=GE.fileselect->count();
        if (count>0)
        {
            std::string filename=GE.fileselect->value(1);
            if (!filename.empty())
            {
                LoadedScene=parse(filename.c_str());
                if (LoadedScene!=nullptr)
                {
                    frameinfo.dt=LoadedScene->totaltime/LoadedScene->M.size();
                    frameinfo.num=LoadedScene->M.size();
                    frameinfo.f=0;
                    GE.topwin->copy_label(filename.c_str());
                    GE.progress->minimum(0);
                    GE.progress->maximum(frameinfo.num);
                    fmtoutput("Duration: %.1f sec (%.0f fps)\n", LoadedScene->totaltime, 1.0/frameinfo.dt);
                    Fl::repeat_timeout(0.1, cbredraw, (void*)GE.topwin);
                }
            }
        }
    }
    else fmtoutput("Run callback for %s\n", str);
}

static void cbtop(Fl_Widget*widget, void*)
{
    fmtoutput("cbtop called.\n");
    // Prevent ModelView from using pointer after release?
    ((Fl_Window*)widget)->hide();
}

int main(int argc, char**argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    Fl_File_Icon::load_system_icons();
    Fl::use_high_res_GL(1);

    GE.fileselect=new Fl_File_Chooser(nullptr, "*", Fl_File_Chooser::SINGLE, "Select bvh file");
    // GE.fileselect->callback(fc_callback);

    GE.topwin=new Fl_Window(800, 300);
    GE.topwin->callback(cbtop);
    GE.modelview=NewModelWindow(0, 0, 450, 280);
    GE.progress=new Fl_Progress(0, 280, 450, 20);
    GE.progress->minimum(0);
    GE.progress->maximum(100);
    GE.progress->color(0x88888800);               // background color
    GE.progress->selection_color(0x4444ff00);     // progress bar color
    GE.progress->labelcolor(FL_WHITE);            // text color
    GE.progress->label("0/100");                  // update progress bar's label
    auto*g=new Fl_Window(450,0,500,300);
    if (true)
    {
        GE.textwidget=new Fl_Text_Display(0,0,350,200);
        GE.textwidget->buffer(new Fl_Text_Buffer());
        GE.textwidget->end();

        Fl_Button*b1=new Fl_Button(150, 200, 200, 50, "..");
        b1->color(FL_FREE_COLOR);
        b1->box(FL_BORDER_BOX);
        b1->callback(cbbuttons, (void*)"select_bvh");
        if (auto icon=Fl_File_Icon::find(".", Fl_File_Icon::DIRECTORY); icon!=nullptr)
        {
            b1->labelcolor(FL_YELLOW);
            icon->label(b1);
        }

        Fl_Button*b2=new Fl_Button(150, 250, 200, 50, "Button 2");
        b2->color(FL_FREE_COLOR);
        b2->box(FL_BORDER_BOX);
        b2->callback(cbbuttons, NULL);
    }
    g->end();
    GE.topwin->end();
    GE.topwin->resizable(GE.modelview);
    GE.topwin->label("(No File) BVHShow");
    GE.topwin->show(argc, argv);
    Fl::run();
}
