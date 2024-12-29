
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
#include <GL/glew.h>
#include <parser.h>
#include "flshow.h"

BVHScene*LoadedScene=nullptr;

struct {
    Fl_Window*topwin {nullptr};
    Fl_Text_Display*textwidget {nullptr};
    Fl_File_Chooser*fileselect {nullptr};
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
            // for (int i=1; i<=count&&GE.fileselect->value(i); i++) fmtoutput("%d/%d '%s'\n", i, count, GE.fileselect->value(i));
            if (!filename.empty())
            {
                LoadedScene=parse(filename.c_str());
                if (LoadedScene!=nullptr) GE.topwin->copy_label(filename.c_str());
            }
        }
    }
    else fmtoutput("Run callback for %s\n", str);
}

int main(int argc, char**argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    Fl_File_Icon::load_system_icons();
    Fl::use_high_res_GL(1);

    GE.fileselect=new Fl_File_Chooser(nullptr, "*", Fl_File_Chooser::SINGLE, "Select bvh file");
    // GE.fileselect->callback(fc_callback);

    GE.topwin=new Fl_Window(800, 300);
    auto*glwin=NewModelWindow(0, 0, 450, 300);
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
    GE.topwin->resizable(glwin);
    GE.topwin->label("Model View");
    GE.topwin->show(argc, argv);
    Fl::run();
}
