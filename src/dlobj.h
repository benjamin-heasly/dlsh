/******************************************\
* dlobj.h -                                *
*   header for manipulating tcl 8.0 object *
*   representation of dlsh dynamic lists.  *
*                                          *
* March 1998, Michael Thayer               *
\******************************************/

#ifndef __dlobj_h__
  #define __dlobj_h__

#include <stdlib.h>
#include <stdio.h>
#include <tcl.h>
#include <df.h>
#include <tcl_dl.h>

/* from dlobj.c */

extern int register_dl_obj(void) ;
extern void Tcl_SetDynListObj(Tcl_Obj * obj, DYN_LIST * list) ;
extern int Tcl_GetDynListFromObj(Tcl_Interp * interp, Tcl_Obj * objPtr,
	DYN_LIST ** dlPtr) ;

/* from tcl_dl.c */

extern char dlErrorString[] ;
DLLEXP int tclFindDynList(Tcl_Interp *interp, char *name, DYN_LIST **dl) ;
extern int tclDynListTempName(DYN_LIST *dl, char ** result) ;
extern int tclDeleteDynList (ClientData data, Tcl_Interp *interp,
	int argc, char *argv[]) ;

#endif /* defined(__dlobj_h__) */
