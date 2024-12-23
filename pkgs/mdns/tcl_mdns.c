/*
 * NAME
 *      tcl_mdns.c
 *
 * DESCRIPTION
 *      Find advertised servers on local network
 *
 * DETAILS
 *      
 *
 * AUTHOR
 *      DLS
 *
 * DATE
 *      6/24
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

#include <tcl.h>
#include "mdns_query_response.h"

int mdnsFindCmd(ClientData data, Tcl_Interp *interp,
		int objc, Tcl_Obj * const objv[])
{
  int timeout_ms = 500;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "service [timeout_ms]");
    return TCL_ERROR;
  }

  if (objc > 2) {
    if (Tcl_GetIntFromObj(interp, objv[2], &timeout_ms) != TCL_OK)
      return TCL_ERROR;
  }
  
  char buf[1024];
  send_mdns_query_service(Tcl_GetString(objv[1]), buf, sizeof(buf), timeout_ms);

  if (strlen(buf)) Tcl_SetObjResult(interp, Tcl_NewStringObj(buf, strlen(buf)));
  return TCL_OK;
}


/************************************************************************/
/*                            Points_Init                               */
/************************************************************************/

#ifdef WIN32
EXPORT(int,Tcl_mdns_Init) (Tcl_Interp *interp)
#else
     int Tcl_mdns_Init(Tcl_Interp *interp)
#endif
{
  if (
#ifdef USE_TCL_STUBS
      Tcl_InitStubs(interp, "8.6-", 0)
#else
      Tcl_PkgRequire(interp, "Tcl", "8.6-", 0)
#endif
      == NULL) {
    return TCL_ERROR;
  }

  if (Tcl_PkgProvide(interp, "mdns", "1.0") != TCL_OK) {
	return TCL_ERROR;
  }

  Tcl_Eval(interp, "namespace eval ::mdns {}");
  Tcl_CreateObjCommand(interp, "::mdns::find",
		       (Tcl_ObjCmdProc *) mdnsFindCmd,
		       (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  return TCL_OK;
}



#ifdef WIN32
BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;
    DWORD reason;
    LPVOID reserved;
{
  return TRUE;
}
#endif
