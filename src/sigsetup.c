/* 
 * NAME
 *   sigsetup
 *   
 * DESCRIPTION
 *   Setup signal handler for control-c
 *
 */


#include "tcl.h"
#include <signal.h>

typedef void (*RETSIGTYPE)();
Tcl_AsyncHandler sig_Token;
int Sig_HandleSafe(ClientData data, Tcl_Interp *interp, int code);

#ifndef SUN4
void Sig_HandleINT(int sig)
{
  Tcl_AsyncMark(sig_Token);
}
#else
void Sig_HandleINT(sig, code, scp, addr)
     int sig, code;
     struct sigcontext *scp;
     char *addr;
{
  Tcl_AsyncMark(sig_Token);
}
#endif

void Sig_Setup(Tcl_Interp *interp)
{
  RETSIGTYPE (*oldhandler)();
  oldhandler = signal(SIGINT, Sig_HandleINT);
  if ((int) oldhandler == -1) {
    perror("signal failed");
    exit(1);
  }
  sig_Token = Tcl_AsyncCreate(Sig_HandleSafe, NULL);
#if !defined(SGI) && !defined(QNX)
  siginterrupt(SIGINT, 1);
#endif
}

int Sig_HandleSafe(ClientData data, Tcl_Interp *interp, int code)
{
  if (interp) {
    Tcl_SetResult(interp, "Interrupt...", TCL_STATIC);
    signal(SIGINT, Sig_HandleINT);
  }
  return TCL_ERROR;
}


