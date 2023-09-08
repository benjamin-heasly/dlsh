/*****************************************\
* dlobj.c -                               *
*   code for manipulating tcl 8.0 object  *
*   representation of dlsh dynamic lists. *
*                                         *
* March 1998, Michael Thayer              *
\*****************************************/

#include "dlobj.h"
#include <string.h>

Tcl_ObjType * type_int, * type_list, * type_bool, * type_double, * type_str;

static int set_from_any(Tcl_Interp * interp, Tcl_Obj * obj);
static void update_string_proc(Tcl_Obj * obj);
static void free_int_rep_proc(Tcl_Obj * obj);
static void dup_int_rep_proc(Tcl_Obj * src, Tcl_Obj * dst);

static Tcl_ObjType tclDynListType = {
	"dynlist",
	free_int_rep_proc,
	dup_int_rep_proc,
	update_string_proc,
	set_from_any
};

static DYN_LIST * ObjToDynList(Tcl_Interp * interp, Tcl_Obj * obj);
static DYN_LIST * LongToDynList(Tcl_Interp * interp, Tcl_Obj * obj);
static DYN_LIST * FloatToDynList(Tcl_Interp * interp, Tcl_Obj * obj);
static DYN_LIST * StringToDynList(Tcl_Interp * interp, Tcl_Obj * obj);
static DYN_LIST * ListToDynList(Tcl_Interp * interp, Tcl_Obj * obj);

/* Convert a tcl object to a dynamic list */

static int set_from_any(Tcl_Interp * interp, Tcl_Obj * obj)
{
	if (obj->typePtr == & tclDynListType) return(TCL_OK);
	if ((obj->internalRep.otherValuePtr = ObjToDynList(interp, obj))) {
		obj->typePtr = & tclDynListType;
		return(TCL_OK);
	}
	return(TCL_ERROR);
}


/* Extract a dynlist from an object */

static DYN_LIST * ObjToDynList(Tcl_Interp * interp, Tcl_Obj * obj)
{
	DYN_LIST * ret;

	if (obj->typePtr == type_bool || obj->typePtr == type_int)
		return(LongToDynList(interp, obj));
	if (obj->typePtr == type_double)
		return(FloatToDynList(interp, obj));
	if (obj->typePtr == type_list)
		return(ListToDynList(interp, obj));
	if (obj->typePtr == type_str)
		return(StringToDynList(interp, obj));
	tclFindDynList(interp, Tcl_GetStringFromObj(obj, NULL), & ret);
	return(ret);
}


/* Convert a long value to a dynamic list */

static DYN_LIST * LongToDynList(Tcl_Interp * interp, Tcl_Obj * obj)
{
	DYN_LIST * list = dfuCreateDynList(DF_LONG, 1);

	if (list == NULL) {
		Tcl_SetResult(interp, "DLSH: unable to create a new list",
			TCL_STATIC);
		return(NULL);
	}
	dfuAddDynListLong(list, obj->internalRep.longValue);
	return(list);
}


/* Convert a long value to a dynamic list */

static DYN_LIST * FloatToDynList(Tcl_Interp * interp, Tcl_Obj * obj)
{
	DYN_LIST * list = dfuCreateDynList(DF_FLOAT, 1);

	if (list == NULL) {
		Tcl_SetResult(interp, "DLSH: unable to create a new list",
			TCL_STATIC);
		return(NULL);
	}
	dfuAddDynListFloat(list, (float) obj->internalRep.doubleValue);
	return(list);
}


/* Convert a string value to a dynamic list */

static DYN_LIST * StringToDynList(Tcl_Interp * interp, Tcl_Obj * obj)
{
	DYN_LIST * list = dfuCreateDynList(DF_STRING, 1);

	if (list == NULL) {
		Tcl_SetResult(interp, "DLSH: unable to create a new list",
			TCL_STATIC);
		return(NULL);
	}
	dfuAddDynListString(list, Tcl_GetStringFromObj(obj, NULL));
	return(list);
}


/*
 * Convert a list value to a dynamic list.  This case is more complex than
 * the previous cases, since dynlists are less flexible than Tcl lists -
 * they can only contain one data type, so in some cases, we will have to
 * retype members, or put them into sublists.
 */

typedef enum { longv, floatv, stringv, listv } objtype;

static objtype get_list_type(int objc, Tcl_Obj * objv[]);
static DYN_LIST * create_long_list(Tcl_Interp * interp, int objc,
	Tcl_Obj * objv[]);
static DYN_LIST * create_float_list(Tcl_Interp * interp, int objc,
	Tcl_Obj * objv[]);
static DYN_LIST * create_string_list(Tcl_Interp * interp, int objc,
	Tcl_Obj * objv[]);
static DYN_LIST * create_list_list(Tcl_Interp * interp, int objc,
	Tcl_Obj * objv[]);

/* the main part of the conversion function */

static DYN_LIST * ListToDynList(Tcl_Interp * interp, Tcl_Obj * obj)
{
  int objc;
  Tcl_Obj ** objv;
  
  dlErrorString[0] = 0;
  if (Tcl_ListObjGetElements(interp, obj, & objc, & objv) != TCL_OK)
    return(NULL);
  switch (get_list_type(objc, objv)) {
  case longv:
    return(create_long_list(interp, objc, objv));
  case floatv:
    return(create_float_list(interp, objc, objv));
  case stringv:
    return(create_string_list(interp, objc, objv));
  default:
    break;
  }
  return(create_list_list(interp, objc, objv));
}


/*
 * Determines the type which the dynlist-to-be will have, based on the
 * elements to go in it.  The catch is that all elements must be of the same
 * type, so the Tcl list contains five ints and a string, the dynlist must be
 * of type string.
 */

objtype get_list_type(int objc, Tcl_Obj * objv[])
{
	int i;
	objtype type = longv;

	for (i = 0; i < objc; i++) {
		if (objv[i]->typePtr == type_double
			&& type < floatv) type = floatv;
		if (objv[i]->typePtr == type_list
			&& type < listv) type = listv;
		if (objv[i]->typePtr != type_int && objv[i]->typePtr
			!= type_bool && type < stringv) type = stringv;
	}
	return(type);
}


/* Makes a dynlist of type long from the array of objects passed */

DYN_LIST * create_long_list(Tcl_Interp * interp, int objc, Tcl_Obj * objv[])
{
	int i;
	DYN_LIST * list = dfuCreateDynList(DF_LONG, objc);

	if (list == NULL) {
		Tcl_SetResult(interp, dlErrorString[0] ? dlErrorString :
			"DLSH: unable to create new list", TCL_STATIC);
		return(NULL);
	}
	for (i = 0; i < objc; i++)
		dfuAddDynListLong(list,
			objv[i]->internalRep.longValue);
	return(list);
}


/* Makes a dynlist of type float from the array of objects passed */

DYN_LIST * create_float_list(Tcl_Interp * interp, int objc, Tcl_Obj * objv[])
{
	int i;
	DYN_LIST * list = dfuCreateDynList(DF_FLOAT, objc);

	if (list == NULL) {
		Tcl_SetResult(interp, dlErrorString[0] ? dlErrorString :
			"DLSH: unable to create new list", TCL_STATIC);
		return(NULL);
	}
	for (i = 0; i < objc; i++)
		if (objv[i]->typePtr == type_int)
			dfuAddDynListFloat(list,
				objv[i]->internalRep.longValue);
		else dfuAddDynListFloat(list,
			objv[i]->internalRep.doubleValue);
	return(list);
}


/* Makes a dynlist of type string from the array of objects passed */

DYN_LIST * create_string_list(Tcl_Interp * interp, int objc, Tcl_Obj * objv[])
{
	int i;
	DYN_LIST * list = dfuCreateDynList(DF_STRING, 1);

	if (list == NULL) {
		Tcl_SetResult(interp, dlErrorString[0] ? dlErrorString :
			"DLSH: unable to create new list", TCL_STATIC);
		return(NULL);
	}
	for (i = 0; i < objc; i++)
		dfuAddDynListString(list,
			Tcl_GetStringFromObj(objv[i], NULL));
	return(list);
}


/* Makes a dynlist of type list from the array of objects passed */

DYN_LIST * create_list_list(Tcl_Interp * interp, int objc, Tcl_Obj * objv[])
{
	int i;
	DYN_LIST * addlist, * list = dfuCreateDynList(DF_LIST, 1);

	if (list == NULL) {
		Tcl_SetResult(interp, dlErrorString[0] ? dlErrorString :
			"DLSH: unable to create new list", TCL_STATIC);
		return(NULL);
	}
	for (i = 0; i < objc; i++) {
		addlist = ObjToDynList(interp, objv[i]);
		if (addlist) {
			dfuAddDynListList(list, addlist);
			dfuFreeDynList(addlist);
		} else
			dfuFreeDynList(list);
	}
	return(list) ;
}


/* Convert a dynamic list to a string */

static void update_string_proc(Tcl_Obj * obj)
{
	char * bytes;
	DYN_LIST * list = obj->internalRep.otherValuePtr;

	if (DYN_LIST_NAME(list)[0] == 0)
		if (tclDynListTempName(list, & bytes) != TCL_OK) return;
	obj->length = strlen(bytes);
	obj->bytes = Tcl_Alloc(obj->length + 1);
	strcpy(obj->bytes, bytes);
}


/* Delete any dynlists without a hash entry, or temporary lists */

static void free_int_rep_proc(Tcl_Obj * obj)
{
	DYN_LIST * list = obj->internalRep.otherValuePtr;

	if (DYN_LIST_FLAGS(list) & DL_SUBLIST) return;	/* Don't try to free
								sublists */
	if (DYN_LIST_NAME(list)[0] == 0)
							/* no hash entry */
		dfuFreeDynList(list);
	else if (DYN_LIST_NAME(list)[0] == '%'
		|| DYN_LIST_NAME(list)[0] == '>') {
	/* Ah well... */
		char * argv[2] = {0};

		argv[1] = DYN_LIST_NAME(list);
		dlErrorString[0] = 0;
		if (tclDeleteDynList(NULL, NULL, 2, argv) != TCL_OK
			&& dlErrorString[0]) fprintf(stderr, dlErrorString);
	}
}


/* Duplicate an internal representation */

static void dup_int_rep_proc(Tcl_Obj * src, Tcl_Obj * dst)
{
	dst->internalRep.otherValuePtr = src->internalRep.otherValuePtr;
}


/* And register the whole shooting match */

extern int register_dl_obj(void)
{
	Tcl_RegisterObjType(& tclDynListType);
	type_int = Tcl_GetObjType("int");
	type_bool = Tcl_GetObjType("boolean");
	type_double = Tcl_GetObjType("double");
	type_list = Tcl_GetObjType("list");
	type_str = Tcl_GetObjType("string");
	return(TCL_OK);
}

extern void Tcl_SetDynListObj(Tcl_Obj * obj, DYN_LIST * list)
{
	Tcl_ObjType * oldtype = obj->typePtr;

	if (Tcl_IsShared(obj)) fprintf(stderr,
		"Tcl_SetDynListObj: warning - modifying a shared object\n");
	Tcl_InvalidateStringRep(obj);
	if (oldtype && oldtype->freeIntRepProc)
		oldtype->freeIntRepProc(obj);
	obj->internalRep.otherValuePtr = list;
	obj->typePtr = & tclDynListType;
}


/* convert an object to a dynlist */

extern int Tcl_GetDynListFromObj(Tcl_Interp * interp, Tcl_Obj * objPtr,
	DYN_LIST ** dlPtr)
{
	if (Tcl_ConvertToType(interp, objPtr, & tclDynListType) != TCL_OK)
		return(TCL_ERROR);
	*dlPtr = objPtr->internalRep.otherValuePtr;
	return(TCL_OK);
}
