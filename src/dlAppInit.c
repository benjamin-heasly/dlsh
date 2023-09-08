/* 
 * dlAppInit.c --
 *
 */

#include "tcl.h"
#include "labtcl.h"

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */
/*
extern int matherr();
int *tclDummyMathPtr = (int *) matherr;
*/


/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tcl_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(argc, argv)
    int argc;			/* Number of command-line arguments. */
    char **argv;		/* Values of command-line arguments. */
{
    Tcl_ReadlineMain(argc, argv, Tcl_AppInit);
    return 0;			/* Needed only to prevent compiler warning. */
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
  
  if (Tcl_Init(interp) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  /*
   * Call the init procedures for included packages.  Each call should
   * look like this:
   *
   * if (Mod_Init(interp) == TCL_ERROR) {
   *     return TCL_ERROR;
   * }
   *
   * where "Mod" is the name of the module.
   */
  
  if (CgPS_Init(interp)  == TCL_ERROR) {
    return(TCL_ERROR);
  }

  if (Dl_Init(interp)  == TCL_ERROR) {
    return(TCL_ERROR);
  }
  if (Dlg_Init(interp)  == TCL_ERROR) {
    return(TCL_ERROR);
  }
  if (Df_Init(interp)  == TCL_ERROR) {
    return(TCL_ERROR);
  }

  Sig_Setup(interp);

  /*
   * Call Tcl_CreateCommand for application-specific commands, if
   * they weren't already created by the init procedures called above.
   */
  
  /*
   * Specify a user-specific startup file to invoke if the application
   * is run interactively.  Typically the startup file is "~/.apprc"
   * where "app" is the name of the application.  If this line is deleted
   * then no user-specific startup file will be run under any conditions.
   */
  
  Tcl_SetVar(interp, "tcl_rcFileName", "~/.dfshrc", TCL_GLOBAL_ONLY);
  return TCL_OK;
}
