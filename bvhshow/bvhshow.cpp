
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
                    Fl::repeat_timeout(0.1, cbredraw, (void*)GE.topwin);
                }
            }
        }
    }
    else if (str=="@||")
    {
        frameinfo.state=frameinfo.stop;
        widget->label("@>");
    }
    else if (str=="@>")
    {
        frameinfo.state=frameinfo.animatemodel;
        frameinfo.animmode=frameinfo.run;
        widget->label("@||");
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

    GE.topwin=new Fl_Window(800, 300);
    GE.topwin->callback(cbtop);
    GE.modelview=NewModelWindow(0, 0, 450, 280);
    GE.progress=new Fl_Progress(0, 280, 450, 20);
    GE.progress->minimum(0);
    GE.progress->maximum(100);
    GE.progress->color(0x88888800);               // background color
    GE.progress->selection_color(0x4444ff00);     // progress bar color
    GE.progress->labelcolor(FL_WHITE);            // text color
//  GE.progress->label("0/100");                  // update progress bar's label
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

        Fl_Button*b2=new Fl_Button(150, 250, 200, 50, "@||");
        b2->color(FL_FREE_COLOR);
        b2->box(FL_BORDER_BOX);
        // b2->image(&G_cat);
        b2->callback(cbbuttons, NULL);

        auto*Neu=new Fl_Pack(0,200,100,30);
        Neu->type(Fl_Pack::HORIZONTAL);
        Neu->box(FL_UP_FRAME);
        Neu->spacing(10);
        Fl_Button*tb0=new Fl_Button(0,0,20,20,"@<-");
//      tb0->box(FL_FLAT_BOX);
//      tb0->clear_visible_focus();
        tb0->callback(cbtoolbox);
        Fl_Button*tb3=new Fl_Button(0,0,20,20,"@->");
//      tb3->box(FL_FLAT_BOX);
//      tb3->clear_visible_focus();
        tb3->callback(cbtoolbox);
        Neu->end();
    }
    g->end();
    GE.topwin->end();
    GE.topwin->resizable(GE.modelview);
    GE.topwin->label("(No File) BVHShow");
    GE.topwin->show(argc, argv);
    frameinfo.state=frameinfo.initdummy;
    Fl::repeat_timeout(0.1, cbredraw, (void*)GE.topwin);
    Fl::run();
}
