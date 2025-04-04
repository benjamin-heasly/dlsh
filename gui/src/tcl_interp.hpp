#ifndef __TCLINTERP_H
#define __TCLINTERP_H

#include <tcl.h>
#include <cassert>

class TclInterp {
private:
  Tcl_Interp *_interp;

public:
  int DlshAppInit(Tcl_Interp *interp)
  {
    setenv("TCLLIBPATH", "", 1);

    if (Tcl_Init(interp) == TCL_ERROR) return TCL_ERROR;

    //    std::cout << "created interp" << std::endl;
    
    Tcl_VarEval(interp, "set dlshroot [file join [zipfs root] dlsh]");
    Tcl_VarEval(interp, "zipfs mount /usr/local/dlsh/dlsh.zip $dlshroot", NULL);
    Tcl_VarEval(interp,
		"set auto_path [linsert $auto_path 0 $dlshroot/lib]");
    Tcl_VarEval(interp, "package require dlsh", NULL);

    return TCL_OK;
  }


  TclInterp(int argc, char *argv[])
  {
    _interp = Tcl_CreateInterp();
    assert(_interp != NULL);
#if 0
    if (TclZipfs_Mount(_interp, "dlsh.zip", "app", NULL) != TCL_OK) {
      printf("error mounting zip from \"%s\"\n", argv[1]); 
    }
#endif    
    TclZipfs_AppHook(&argc, &argv);

    /*
     * Invoke application-specific initialization.
     */
    
    if (DlshAppInit(_interp) != TCL_OK) {
      std::cerr << "application-specific initialization failed: ";
      std::cerr << Tcl_GetStringResult(_interp) << std::endl;
    }
    else {
      Tcl_SourceRCFile(_interp);
    }
  }

  ~TclInterp(void)
  {
    /* Free the interpreter */
    Tcl_DeleteInterp(_interp);
  }

  Tcl_Interp *interp(void) { return _interp; }
  
  int eval(const char *command, std::string &resultstr)
  {
    int len;
    int result = Tcl_Eval(_interp, command);
    auto rstr = Tcl_GetStringResult(_interp);
    resultstr = std::string(rstr);
    return result;
  }

  std::string eval(const char *command)
  {
    int len;
    int result = Tcl_Eval(_interp, command);
    auto rstr = Tcl_GetStringResult(_interp);
    return std::string(rstr);
  }
};


#endif /* __TCLINTERP_H */
