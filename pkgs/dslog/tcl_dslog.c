/*
 * NAME
 *   tcl_dslog.c
 *
 * DESCRIPTION
 *
 * AUTHOR
 *   DLS, 06/11
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h" 
#undef WIN32_LEAN_AND_MEAN
#pragma warning (disable:4244)
#pragma warning (disable:4305)
#define DllEntryPoint DllMain
#define EXPORT(a,b) __declspec(dllexport) a b
#else
#define DllEntryPoint
#define EXPORT a b
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <tcl.h>
#include <df.h>
#include <tcl_dl.h>
#include <math.h>
#include <dslog.h>

#define DSLOG_READ     1
#define DSLOG_READ_ESS 2

/*****************************************************************************
 *
 * FUNCTION
 *    dslogReadCmd
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    dslog::read
 *
 * DESCRIPTION
 *    Read a dslog file into dlsh
 *
 ****************************************************************************/

static int dslogReadCmd (ClientData data, Tcl_Interp *interp,
			 int objc, Tcl_Obj *objv[])
{
  int rc;
  int type = (Tcl_Size) data;
  
  DYN_GROUP *dg;
  
  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "path");
    return TCL_ERROR;
  }

  switch (type) {
  case DSLOG_READ:
    rc = dslog_to_dg(Tcl_GetString(objv[1]), &dg);
    break;
  case DSLOG_READ_ESS:
    rc = dslog_to_essdg(Tcl_GetString(objv[1]), &dg);
    break;
  default:
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]), ": read type not supported",
                     (char *) NULL);
    return TCL_ERROR;
  }
  
  switch (rc) {
  case DSLOG_FileNotFound:
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]), ": not found",
                     (char *) NULL);
    return TCL_ERROR;
    break;
  case DSLOG_FileUnreadable:
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]), ": error reading file",
                     (char *) NULL);
    return TCL_ERROR;
    break;
  case DSLOG_InvalidFormat:
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]), ": not recognized",
                     (char *) NULL);
    return TCL_ERROR;
    break;
  }

  return (tclPutGroup(interp, dg));
}

/*****************************************************************************
 *
 * EXPORT
 *
 *****************************************************************************/

#ifdef WIN32
EXPORT(int,Dslog_Init) (Tcl_Interp *interp)
#else
int Dslog_Init(Tcl_Interp *interp)
#endif
{
  
  if (
#ifdef USE_TCL_STUBS
      Tcl_InitStubs(interp, "8.5-", 0)
#else
      Tcl_PkgRequire(interp, "Tcl", "8.5-", 0)
#endif
      == NULL) {
    return TCL_ERROR;
  }
  if (Tcl_PkgRequire(interp, "dlsh", "1.2", 0) == NULL) {
    return TCL_ERROR;
  }

  if (Tcl_PkgProvide(interp, "dslog", "1.0") != TCL_OK) {
    return TCL_ERROR;
  }
  
  Tcl_Eval(interp, "namespace eval dslog {}");
  
  Tcl_CreateObjCommand(interp, "dslog::read",
		       (Tcl_ObjCmdProc *) dslogReadCmd,
		       (ClientData) DSLOG_READ,
		       (Tcl_CmdDeleteProc *) NULL);
  
  Tcl_CreateObjCommand(interp, "dslog::readESS",
		       (Tcl_ObjCmdProc *) dslogReadCmd,
		       (ClientData) DSLOG_READ_ESS,
		       (Tcl_CmdDeleteProc *) NULL);
  
  return TCL_OK;
 }





