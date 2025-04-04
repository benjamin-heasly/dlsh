#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>

#include <tcl.h>
#include "flcgwin.hpp"
#include "cg_tab_manager.h"

static void tab_close_cb(Fl_Widget* o, void* data)
{
  switch (Fl::callback_reason()) {
  case FL_REASON_CLOSED:
    {
      Fl_Tabs *parent = (Fl_Tabs *) o->parent();
      parent->remove(o);
      /* make another tab current if exists */
      currentCG = nullptr;
      for (int i = 0; i < parent->children(); i++) {
	if (parent->child(i)) {
	  currentCG = (CGWin *) parent->child(i);
	  gbSetGeventBuffer(currentCG->gb());
	  parent->value(currentCG);
	  currentCG->redraw();
	  break;
	}
      }
      // Remove from tab manager
      CGTabManager::getInstance().removeCGWin(o->label());
      Fl::delete_widget(o);
    }
    break;
  default:
    break;
  }
}

static int
add_cgraph_tab_func(ClientData data, Tcl_Interp *interp,
		int objc, Tcl_Obj *const objv[])
{
  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "name");
    return (TCL_ERROR);
  }

  Fl_Tabs *tabs = (Fl_Tabs *) data;
  tabs->begin();
  
  // Get next tab name from manager
  std::string tab_name = CGTabManager::getInstance().getNextTabName();
  
  auto cgwin_widget = new CGWin(interp, tabs->x(), tabs->y()+25,
				tabs->w(), tabs->h()-25,
				tab_name.c_str());
  tabs->value(cgwin_widget);
  cgwin_widget->box(FL_NO_BOX);
  cgwin_widget->selection_color(FL_BACKGROUND_COLOR);
  cgwin_widget->labelfont(0);
  cgwin_widget->labelsize(14);
  cgwin_widget->labelcolor(FL_FOREGROUND_COLOR);
  cgwin_widget->labeltype(FL_NORMAL_LABEL);
  cgwin_widget->align(Fl_Align(FL_ALIGN_CENTER));
  cgwin_widget->callback(tab_close_cb, (void*) cgwin_widget);
  cgwin_widget->when(FL_WHEN_CLOSED);
  cgwin_widget->redraw();
  tabs->end();
  
  // Add to tab manager
  CGTabManager::getInstance().addCGWin(tab_name, cgwin_widget);
  
  return TCL_OK;
}

// Callback function for tab changes
static void tab_callback(Fl_Widget* widget, void* data) {
    Fl_Tabs* tabs = dynamic_cast<Fl_Tabs*>(widget);
    if (tabs) {
        Fl_Widget* selected_tab = tabs->value();
	if (selected_tab) {
	  currentCG = (CGWin *) selected_tab;

	  /* switch to our graphics buffer */
	  gbSetGeventBuffer(currentCG->gb());

	  /* and redraw */
	  selected_tab->redraw();
        }
    }
}

static int
select_cgraph_tab_func(ClientData data, Tcl_Interp *interp,
		int objc, Tcl_Obj *const objv[])
{
  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "name");
    return (TCL_ERROR);
  }

  const char* tab_name = Tcl_GetString(objv[1]);
  CGWin* widget = CGTabManager::getInstance().getCGWin(tab_name);
  if (widget) {
    Fl_Tabs* tabs = (Fl_Tabs*)data;
    tabs->value(widget);
    currentCG = widget;
    gbSetGeventBuffer(currentCG->gb());
    widget->redraw();
    return TCL_OK;
  }

  Tcl_SetResult(interp, "Tab not found", TCL_STATIC);
  return TCL_ERROR;
}

// Extension initialization function
extern "C" int Flcgwin_Init(Tcl_Interp *interp)
{
  if (Tcl_InitStubs(interp, "9.0", 0) == nullptr) {
    return TCL_ERROR;
  }

  if (Tcl_PkgProvide(interp, "flcgwin", "1.0") != TCL_OK) {
    return TCL_ERROR;
  }

  Fl_Tabs *tabs =
    static_cast<Fl_Tabs *>(Tcl_GetAssocData(interp, "cgtabs", nullptr));

  tabs->callback(tab_callback);
  
  Tcl_CreateObjCommand(interp,
		       "cgAddTab", (Tcl_ObjCmdProc *) add_cgraph_tab_func, 
		       (ClientData) tabs, NULL);
  Tcl_CreateObjCommand(interp,
		       "cgSelectTab", (Tcl_ObjCmdProc *) select_cgraph_tab_func, 
		       (ClientData) tabs, NULL);
  return TCL_OK;
}
