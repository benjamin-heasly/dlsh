//
// DLSH - FLTK based DLSH console
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H> // fl_message()

#include "Fl_Console.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <df.h>
#include <dynio.h>

#include "tcl_interp.hpp"
#include <DGFile.hpp>
#include <DGTable.hpp>

#include "dlfuncs.h"



void menu_cb(Fl_Widget*, void*) {
}

void exit_cb(Fl_Widget*, void*) {
  exit(0);
}

Fl_Menu_Item menuitems[] = {
  { "&File",              0, 0, 0, FL_SUBMENU },
  { "&New File",        0, (Fl_Callback *) menu_cb },
    { "&Open File...",    FL_COMMAND + 'o', (Fl_Callback *)menu_cb },
    { "&Insert File...",  FL_COMMAND + 'i', (Fl_Callback *)menu_cb, 0, FL_MENU_DIVIDER },
    { "&Save File",       FL_COMMAND + 's', (Fl_Callback *)menu_cb },
    { "Save File &As...", FL_COMMAND + FL_SHIFT + 's', (Fl_Callback *)menu_cb, 0, FL_MENU_DIVIDER },
    { "New &View",        FL_ALT
#ifdef __APPLE__
      + FL_COMMAND
#endif
      + 'v', (Fl_Callback *)menu_cb, 0 },
    { "&Close View",      FL_COMMAND + 'w', (Fl_Callback *)menu_cb, 0, FL_MENU_DIVIDER },
    { "E&xit",            FL_COMMAND + 'q', (Fl_Callback *)exit_cb, 0 },
    { 0 },

  { "&Edit", 0, 0, 0, FL_SUBMENU },
    { "Cu&t",             FL_COMMAND + 'x', (Fl_Callback *)menu_cb },
    { "&Copy",            FL_COMMAND + 'c', (Fl_Callback *)menu_cb },
    { "&Paste",           FL_COMMAND + 'v', (Fl_Callback *)menu_cb },
    { "&Delete",          0, (Fl_Callback *)menu_cb },
    { "Preferences",      0, 0, 0, FL_SUBMENU },
      { "Line Numbers",   FL_COMMAND + 'l', (Fl_Callback *)menu_cb, 0, FL_MENU_TOGGLE },
      { "Word Wrap",      0,                (Fl_Callback *)menu_cb, 0, FL_MENU_TOGGLE },
      { 0 },
    { 0 },
  { 0 }
};

class App;

typedef struct _alarm_info {
  App *app;
  int alarm_id;
  std::string script;
} alarm_info_t;

class App
{
public:
  TclInterp *interp;
  Fl_Console *term {};
  Fl_Tabs *tabs;

  static const int nalarms = 8;
  alarm_info_t alarm_cbs[nalarms];
  

  static void tab_close_cb(Fl_Widget* o, void* data)
  {
    if (Fl::callback_reason() == FL_REASON_CLOSED) {
      Fl_Tabs *parent = (Fl_Tabs *) o->parent();
      parent->value(o);
      parent->remove(o);
      Fl::delete_widget(o);
      return;
    }
  }
  
  int add_table(const DYN_GROUP *dg)
  {
    auto g = new Fl_Group(tabs->x()+5, tabs->y()+25, tabs->w()-5, tabs->h()-45);
    auto t = new DGTable(dg, g->x(), g->y(), g->w(), g->h());
    tabs->value(t);    
    g->end();
    g->callback(App::tab_close_cb, (void*) g);
    g->when(FL_WHEN_CLOSED);
    g->label(DYN_GROUP_NAME(t->dg));
    return 1;
  }
  
  int add_table(const char *filename)
  {
    DYN_GROUP *dg = DGFile::read_dgz((char *) filename);
    if (!dg) return 0;
    auto g = new Fl_Group(tabs->x()+5, tabs->y()+25, tabs->w()-5, tabs->h()-45);
    auto t = new DGTable(dg, g->x(), g->y(), g->w(), g->h());
    if (t) {
      tabs->value(t);    
      g->end();
      g->callback(App::tab_close_cb, (void*) g);
      g->when(FL_WHEN_CLOSED);
      g->label(DYN_GROUP_NAME(t->dg));
    }
    
    return 1;
  }

  static int
  dg_view_func(ClientData data, Tcl_Interp *interp,
	       int objc, Tcl_Obj *const objv[])
  {
    App *app = static_cast<App *>(data);
    DYN_GROUP *dg;
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 1, objv, "dg|filename");
      return (TCL_ERROR);
    }
    
    if (tclFindDynGroup(interp, Tcl_GetString(objv[1]), &dg) == TCL_OK) { 
      app->tabs->begin();
      app->add_table(dg);
      app->tabs->end();
      return (TCL_OK);
    }

    return TCL_ERROR;
  }
  
  static int
  print_func(ClientData data, Tcl_Interp *interp,
	       int objc, Tcl_Obj *const objv[])
  {
    App *app = static_cast<App *>(data);

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 1, objv, "string");
      return (TCL_ERROR);
    }

    app->term->append(Tcl_GetString(objv[1]));

    // should check for -nonewline
    app->term->append("\n");

    return (TCL_OK);
  }

  static int
  rect_func(ClientData data, Tcl_Interp *interp,
	       int objc, Tcl_Obj *const objv[])
  {
    App *app = static_cast<App *>(data);
    int x, y, w, h;
    if (objc != 5) {
      Tcl_WrongNumArgs(interp, 1, objv, "x y w h");
      return (TCL_ERROR);
    }


    if (Tcl_GetIntFromObj(interp, objv[1], &x) != TCL_OK) return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[2], &y) != TCL_OK) return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &w) != TCL_OK) return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[4], &h) != TCL_OK) return TCL_ERROR;

    return (TCL_OK);
  }

  static void alarm_callback(void *data) {
    alarm_info_t *alarm_info = static_cast<alarm_info_t *>(data);
    std::string resultstr;
    auto result = alarm_info->app->interp->eval(alarm_info->script.c_str(), resultstr);
  }
  
  static int
  alarm_func(ClientData data, Tcl_Interp *interp,
	     int objc, Tcl_Obj *const objv[])
  {
    App *app = static_cast<App *>(data);
    int timeout;
    int alarm_id;
    static int epsilon = 1;	// try to make timer a bit more accurate
    
    if (objc != 4) {
      Tcl_WrongNumArgs(interp, 1, objv, "alarm_id time_ms script");
      return (TCL_ERROR);
    }

    if (Tcl_GetIntFromObj(interp, objv[1], &alarm_id) != TCL_OK)
      return TCL_ERROR;
    if (alarm_id >= nalarms) {
      Tcl_AppendResult(interp, Tcl_GetString(objv[1]),
		       ": invalid alarm", (char *) NULL);
      return TCL_ERROR;
    }
    if (Tcl_GetIntFromObj(interp, objv[2], &timeout) != TCL_OK)
      return TCL_ERROR;

    app->alarm_cbs[alarm_id].script = std::string(Tcl_GetString(objv[3]));
    Fl::add_timeout((timeout-epsilon)/1000.0, alarm_callback, &app->alarm_cbs[alarm_id]);     

    return (TCL_OK);
  }
  
  App(int argc, char *argv[],
      Fl_Tabs *tabs, Fl_Console *term): tabs { tabs }, term { term } {

    for (auto i = 0; i < nalarms; i++) {
      alarm_cbs[i].app = this;
      alarm_cbs[i].alarm_id = i;
    }
    interp = new TclInterp(argc, argv);

    /* Register our Tcl commands */
    Tcl_CreateObjCommand(interp->interp(),
			 "print", (Tcl_ObjCmdProc *) print_func, 
			 (ClientData) this, NULL);
    Tcl_CreateObjCommand(interp->interp(),
			 "dg_view", (Tcl_ObjCmdProc *) dg_view_func, 
			 (ClientData) this, NULL);
    Tcl_CreateObjCommand(interp->interp(),
			 "rect", (Tcl_ObjCmdProc *) rect_func, 
			 (ClientData) this, NULL);
    Tcl_CreateObjCommand(interp->interp(),
			 "alarm", (Tcl_ObjCmdProc *) alarm_func,
			 (ClientData) this, NULL);
  }

  ~App() {
    delete interp;
  }
};


Fl_Console *G_tty = nullptr;

// User-provided getch() function
int linenoise_getch(void) { return G_tty->getch(); }

// User-provided console write() function
void linenoise_write(const char *buf, size_t n) { G_tty->append(buf, n); }


static int eval(char *command, void *clientData) {
  App *app = static_cast<App *>(clientData);

  std::string resultstr;
  auto result = app->interp->eval(command, resultstr);

  if (resultstr.length()) {
    if (result != TCL_OK) app->term->append_ascii("\033[31m");
    app->term->append(resultstr.c_str());
    if (result != TCL_OK) app->term->append_ascii("\033[0m");
    app->term->append("\n");
  }
  app->term->redraw();
  return result;
}

class DgViewer : public Fl_Tabs {
  int dnd_inside;
public:
  // Ctor
  DgViewer(int x,int y,int w,int h) : Fl_Tabs(x,y,w,h) {
    dnd_inside = 0;
  }
  // Receiver event handler
  int handle(int event) FL_OVERRIDE {
    int ret = Fl_Tabs::handle(event);
    int len;
    switch ( event ) {
      case FL_DND_ENTER:        // return(1) for this event to 'accept' dnd
        dnd_inside = 1;         // status: inside the widget, accept drop
        ret = 1;
        break;
      case FL_DND_DRAG:         // return(1) for this event to 'accept' dnd
        ret = 1;
        break;
      case FL_DND_RELEASE:      // return(1) for this event to 'accept' the payload (drop)
        if (dnd_inside) {
          ret = 1;              // return(1) and expect FL_PASTE event to follow
        } else {
          ret = 0;              // return(0) to reject the DND payload (drop)
        }
        break;
      case FL_PASTE:              // handle actual drop (paste) operation
        len = Fl::event_length();
        if (len && Fl::event_text()) {
	  App *app = static_cast<App *>(user_data());
	  begin();
	  app->add_table(Fl::event_text());
	  end();
	}
        ret = 1;
        break;
      case FL_DND_LEAVE:        // not strictly necessary to return(1) for this event
        dnd_inside = 0;         // status: mouse is outside, don't accept drop
        ret = 1;                // return(1) anyway..
        break;
    }
    return(ret);
  }
};

int main(int argc, char *argv[])
{
  int w = 900, h = 600;
  Fl_Double_Window win{w, h};

  Fl_Menu_Bar* m = new Fl_Menu_Bar(0, 0, win.w(), 30);
  m->copy(menuitems, &w);

  //  auto tile = new Fl_Tile{0, 30, win.w(), win.h()-30};
  // tile->init_size_range(30, 30); // all children's size shall be at least 30x30
  
  auto dgview = new Fl_Group{0, 30, 400, 320};
  DgViewer tabs{0, 30, 400, 320};
  tabs.resizable(&tabs);
  tabs.end();
  dgview->end();
  //  tile->add(&tabs);
  dgview->resizable(&tabs);

  // Add a terminal to bottom of app window for scrolling history of status messages.
  auto term = new Fl_Console(0, 350 ,win.w(), win.h()-350);
  term->ansi(true);  // enable use of "\033[32m"
  term->end();
  term->resizable(term);

  G_tty = term;
  
  App app(argc, argv, &tabs, term);

  tabs.user_data(&app);
  
#ifdef ADD_EDITOR  
  auto tbuffer = new Fl_Text_Buffer;
  auto editor = new Fl_Text_Editor(0, 30, win.w(), 265);
  editor->buffer(tbuffer);
  editor->textfont(FL_COURIER);
  editor->textsize(12);
  editor->align(FL_ALIGN_CLIP);
  editor->resizable(editor);
  
  tile->resizable(editor);
#endif
  
  auto cgview = new Fl_Group{400, 30, win.w()-400, 320};
  auto cgtabs = new Fl_Tabs{cgview->x(), cgview->y(), cgview->w(), cgview->h()};
  cgtabs->resizable(tabs);
  cgtabs->end();
  cgview->end();
  
  Fl_Group command_term{0, 350, win.w(), win.h()-350};
  command_term.add(term);
  command_term.end();
  //  tile->add(&command_term);

  // Share the pointer to the tab widget so the flcgwin package can access
  Tcl_SetAssocData(app.interp->interp(), "cgtabs", NULL,
		   static_cast<ClientData>(cgtabs));
  app.interp->eval("package require flcgwin; cgAddTab cgtabs");
 
  win.resizable(win);
  win.show();

  term->init_linenoise();
  term->set_callback(eval, &app);

  return(Fl::run());
}

