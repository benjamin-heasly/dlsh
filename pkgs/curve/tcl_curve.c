/*
 * NAME
 *   tcl_curve.c
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

#include "curve.h"
#include "gpc.h"		/* for polygon clipping */

/* From dl_clipper.cpp */
extern int add_clipper_commands(Tcl_Interp *interp);

/*
 * NAME
 *    cubic
 * CALLS
 *    third_order_poly (curve.c)
 */
static DYN_LIST *cubic(DYN_LIST *x, DYN_LIST *y, int res)
{
  DYN_LIST *newlist = NULL, *tmplist;
  int nout, ret;
  float *interpx, *interpy;

  if (DYN_LIST_N(x) != DYN_LIST_N(y)) return NULL;
  if (res <= 0) return NULL;

  if (DYN_LIST_DATATYPE(x) == DF_FLOAT &&
      DYN_LIST_DATATYPE(y) == DF_FLOAT) {    
    nout = (res+1)*DYN_LIST_N(x);
    interpx = (float *) calloc(nout, sizeof(float));
    interpy = (float *) calloc(nout, sizeof(float));
    
    ret = third_order_poly(DYN_LIST_N(x), 
			   (float *) DYN_LIST_VALS(x), 
			   (float *) DYN_LIST_VALS(y),
			   res, interpx, interpy);
    
    if (ret != nout) return NULL;
    
    newlist = dfuCreateDynList(DF_LIST, 2);
    
    tmplist = dfuCreateDynListWithVals(DF_FLOAT, ret, interpx);
    dfuMoveDynListList(newlist, tmplist);
    
    tmplist = dfuCreateDynListWithVals(DF_FLOAT, ret, interpy);
    dfuMoveDynListList(newlist, tmplist);
  } 
  else if (DYN_LIST_DATATYPE(x) == DF_LIST &&
	   DYN_LIST_DATATYPE(y) == DF_LIST) {
    int i;
    DYN_LIST **xvals = (DYN_LIST **) DYN_LIST_VALS(x);
    DYN_LIST **yvals = (DYN_LIST **) DYN_LIST_VALS(y);
    newlist = dfuCreateDynList(DF_LIST, DYN_LIST_N(x));

    for (i = 0; i < DYN_LIST_N(x); i++) {
      tmplist = cubic(xvals[i], yvals[i], res);
      if (tmplist) dfuMoveDynListList(newlist, tmplist);
      else {
	if (newlist) dfuFreeDynList(newlist);
	return NULL;
      }
    }
  }
  return newlist;
}


/*****************************************************************************
 *
 * FUNCTION
 *    cubicCmd
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    curve::cubic
 *
 * DESCRIPTION
 *    Perform third order polynomial fit
 *
 ****************************************************************************/

static int cubicCmd (ClientData data, Tcl_Interp *interp,
			   int argc, char *argv[])
{
  DYN_LIST *x, *y, *newlist;	/* i/o dynlists                 */
  int res = 20;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals [res]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &x) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &y) != TCL_OK) return TCL_ERROR;
  if (argc > 3) {
    if (Tcl_GetInt(interp, argv[3], &res) != TCL_OK) return TCL_ERROR;
  }

  if (DYN_LIST_N(x) != DYN_LIST_N(y)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }

  newlist = cubic(x, y, res);

  if (!newlist) {
    Tcl_AppendResult(interp, argv[0], ": unable to interpolate data",
		     (char *) NULL);
    return TCL_ERROR;    
  }
  return(tclPutList(interp, newlist));
}



/*
 * NAME
 *    bezier
 * CALLS
 *    bezier_interpolate (curve.c)
 */
static DYN_LIST *bezier(DYN_LIST *x, DYN_LIST *y, int res)
{
  DYN_LIST *newlist = NULL, *tmplist;
  int nout, ret;
  float *interpx, *interpy;

  if (DYN_LIST_N(x) != DYN_LIST_N(y)) return NULL;
  if (DYN_LIST_N(x) % 4 != 0) return NULL;
  if (res <= 0) return NULL;

  if (DYN_LIST_DATATYPE(x) == DF_FLOAT &&
      DYN_LIST_DATATYPE(y) == DF_FLOAT) {    
    nout = (res+1)*(DYN_LIST_N(x)/4);
    interpx = (float *) calloc(nout, sizeof(float));
    interpy = (float *) calloc(nout, sizeof(float));
    
    ret = bezier_interpolate(DYN_LIST_N(x), 
			     (float *) DYN_LIST_VALS(x), 
			     (float *) DYN_LIST_VALS(y),
			     res, interpx, interpy);
    
    if (ret != nout) return NULL;
    
    newlist = dfuCreateDynList(DF_LIST, 2);
    
    tmplist = dfuCreateDynListWithVals(DF_FLOAT, ret, interpx);
    dfuMoveDynListList(newlist, tmplist);
    
    tmplist = dfuCreateDynListWithVals(DF_FLOAT, ret, interpy);
    dfuMoveDynListList(newlist, tmplist);
  } 
  else if (DYN_LIST_DATATYPE(x) == DF_LIST &&
	   DYN_LIST_DATATYPE(y) == DF_LIST) {
    int i;
    DYN_LIST **xvals = (DYN_LIST **) DYN_LIST_VALS(x);
    DYN_LIST **yvals = (DYN_LIST **) DYN_LIST_VALS(y);
    newlist = dfuCreateDynList(DF_LIST, DYN_LIST_N(x));

    for (i = 0; i < DYN_LIST_N(x); i++) {
      tmplist = bezier(xvals[i], yvals[i], res);
      if (tmplist) dfuMoveDynListList(newlist, tmplist);
      else {
	if (newlist) dfuFreeDynList(newlist);
	return NULL;
      }
    }
  }
  return newlist;
}

static int bezierCmd (ClientData data, Tcl_Interp *interp,
		      int argc, char *argv[])
{
  DYN_LIST *x, *y, *newlist;	/* i/o dynlists                 */
  int res = 20;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals [res]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &x) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &y) != TCL_OK) return TCL_ERROR;
  if (argc > 3) {
    if (Tcl_GetInt(interp, argv[3], &res) != TCL_OK) return TCL_ERROR;
  }

  if (DYN_LIST_N(x) != DYN_LIST_N(y)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }

  if (DYN_LIST_N(x) % 4 != 0) {
    Tcl_AppendResult(interp, argv[0], ": x & y lengths must be multiples of 4",
		     (char *) NULL);
    return TCL_ERROR;
  }
  newlist = bezier(x, y, res);

  if (!newlist) {
    Tcl_AppendResult(interp, argv[0], ": unable to interpolate data",
		     (char *) NULL);
    return TCL_ERROR;    
  }
  return(tclPutList(interp, newlist));
}




/*****************************************************************************
 *
 * FUNCTION
 *    closestPointCmd
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    curves::closestPoint
 *
 * DESCRIPTION
 *    Find point index nearest to new point (for ordered insertion)
 *
 ****************************************************************************/

static int closestPointCmd (ClientData data, Tcl_Interp *interp,
			    int argc, char *argv[])
{
  DYN_LIST *xp, *yp;
  double x, y;
  int result;

  if (argc < 5) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals x y", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &xp) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &yp) != TCL_OK) return TCL_ERROR;
  if (Tcl_GetDouble(interp, argv[3], &x) != TCL_OK) return TCL_ERROR;
  if (Tcl_GetDouble(interp, argv[4], &y) != TCL_OK) return TCL_ERROR;

  if (DYN_LIST_N(xp) != DYN_LIST_N(yp)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  result = closest_point(DYN_LIST_N(xp), 
			 (float *) DYN_LIST_VALS(xp), 
			 (float *) DYN_LIST_VALS(yp),
			 x, y);

  Tcl_SetObjResult(interp, Tcl_NewIntObj(result));
  return TCL_OK;
}


/*****************************************************************************
 *
 * FUNCTION
 *    polygonCentroid
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    curves::polygonCentroid
 *
 * DESCRIPTION
 *    Find area and centroid of a polygon
 *
 ****************************************************************************/

static int polygonCentroidCmd (ClientData data, Tcl_Interp *interp,
			    int argc, char *argv[])
{
  DYN_LIST *xp, *yp;
  int result;
  float area, xc, yc;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &xp) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &yp) != TCL_OK) return TCL_ERROR;

  if (DYN_LIST_N(xp) != DYN_LIST_N(yp)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }

  if (DYN_LIST_DATATYPE(xp) != DF_FLOAT &&
      DYN_LIST_DATATYPE(yp) != DF_FLOAT) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be floats",
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  result = polygon_centroid((float *) DYN_LIST_VALS(xp), 
			    (float *) DYN_LIST_VALS(yp),
			    DYN_LIST_N(xp), 
			    &xc, &yc, &area);
  if (result) {
    Tcl_AppendResult(interp, argv[0], ": error computing area/centroid",
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (area < 0) {
    area = -area;
  }

  Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
  Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(area));
  Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(xc));
  Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewDoubleObj(yc));
  Tcl_SetObjResult(interp, listPtr);

  return TCL_OK;
}


/*****************************************************************************
 *
 * FUNCTION
 *    polygonClose
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    curves::polygonClose
 *
 * DESCRIPTION
 *    close a polygon (if not already closed)
 *
 ****************************************************************************/

static int polygonCloseCmd (ClientData data, Tcl_Interp *interp,
			    int argc, char *argv[])
{
  DYN_LIST *xp, *yp;
  float *xvals, *yvals;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &xp) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &yp) != TCL_OK) return TCL_ERROR;

  if (DYN_LIST_N(xp) != DYN_LIST_N(yp)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }

  if (DYN_LIST_DATATYPE(xp) != DF_FLOAT &&
      DYN_LIST_DATATYPE(yp) != DF_FLOAT) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be floats",
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  xvals = (float *) DYN_LIST_VALS(xp);
  yvals = (float *) DYN_LIST_VALS(yp);
  if (xvals[0] == xvals[DYN_LIST_N(xp)-1] &&
      xvals[0] == xvals[DYN_LIST_N(xp)-1]) {
    /* already closed */
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
  }
  else {
    dfuAddDynListFloat(xp, xvals[0]);
    dfuAddDynListFloat(yp, yvals[0]);
    return TCL_OK;
  }
}

/*****************************************************************************
 *
 * polygon clipping code
 *
 *****************************************************************************/

static int dynListToPolygon(DYN_LIST *dl, gpc_polygon *p)
{
  float *xs, *ys;
  int *holes;
  int i, j, ncontours;
  DYN_LIST **sublists, **contours;
  if (DYN_LIST_DATATYPE(dl) != DF_LIST) return -1;
  if (DYN_LIST_N(dl) > 2) {
    return -1;
  }

  sublists = (DYN_LIST **) DYN_LIST_VALS(dl);

  /* if list has two sublists, then the second represents hole status */
  if (DYN_LIST_N(dl) == 2) {
    if (DYN_LIST_N(sublists[0]) != DYN_LIST_N(sublists[1])) return -1;
    if (DYN_LIST_DATATYPE(sublists[1]) != DF_LONG) return -1;
    holes = (int *) DYN_LIST_VALS(sublists[1]);
  }
  else holes = NULL;

  if (DYN_LIST_DATATYPE(sublists[0]) != DF_LIST) return -1;
  ncontours = DYN_LIST_N(sublists[0]);

  contours = (DYN_LIST **) DYN_LIST_VALS(sublists[0]);

  /*
   * Now check sublists
   */
  for (i = 0; i < ncontours; i++) {
    if (DYN_LIST_DATATYPE(contours[i]) != DF_LIST) return -2;
    if (DYN_LIST_N(contours[i]) != 2) return -2;
    
    sublists =  (DYN_LIST **) DYN_LIST_VALS(contours[i]);
    if (DYN_LIST_DATATYPE(sublists[0]) != DF_FLOAT) return -2;
    if (DYN_LIST_DATATYPE(sublists[1]) != DF_FLOAT) return -2;
    if (DYN_LIST_N(sublists[0]) != DYN_LIST_N(sublists[1])) return -2;
  }

  p->num_contours = ncontours;
  p->hole = calloc(p->num_contours, sizeof(int));
  p->contour = calloc(p->num_contours, sizeof(gpc_vertex_list));

  for (i = 0; i < ncontours; i++) {
    sublists = (DYN_LIST **) DYN_LIST_VALS(contours[i]);
    xs = (float *) DYN_LIST_VALS(sublists[0]);
    ys = (float *) DYN_LIST_VALS(sublists[1]);
    p->contour[i].num_vertices = DYN_LIST_N(sublists[0]);
    p->contour[i].vertex = calloc(p->contour[i].num_vertices, 
				  sizeof(gpc_vertex));
    for (j = 0; j < p->contour[i].num_vertices; j++) {
      p->contour[i].vertex[j].x = xs[j];
      p->contour[i].vertex[j].y = ys[j];
    }
    if (holes) p->hole[i] = holes[i];
    else p->hole[i] = 0;
  }
  return 1;
}

static DYN_LIST *polygonToDynList(gpc_polygon *p, int close)
{
  int i, j;
  DYN_LIST *newlist, *contours, *holes, *verts, *xs, *ys;

  contours = dfuCreateDynList(DF_LIST, p->num_contours);
  holes = dfuCreateDynList(DF_LONG, p->num_contours);

  for (i = 0; i < p->num_contours; i++) {
    verts = dfuCreateDynList(DF_LIST, 2);
    xs = dfuCreateDynList(DF_FLOAT, p->contour[i].num_vertices);
    ys = dfuCreateDynList(DF_FLOAT, p->contour[i].num_vertices);
    for (j = 0; j < p->contour[i].num_vertices; j++) {
      dfuAddDynListFloat(xs, p->contour[i].vertex[j].x);
      dfuAddDynListFloat(ys, p->contour[i].vertex[j].y);
    }
    if (close) {
      dfuAddDynListFloat(xs, p->contour[i].vertex[0].x);
      dfuAddDynListFloat(ys, p->contour[i].vertex[0].y);
    }
    dfuMoveDynListList(verts, xs);
    dfuMoveDynListList(verts, ys);
    dfuMoveDynListList(contours, verts);

    dfuAddDynListLong(holes,p->hole[i]);
  }

  newlist = dfuCreateDynList(DF_LIST, 2);
  dfuMoveDynListList(newlist, contours);
  dfuMoveDynListList(newlist, holes);

  return newlist;
}

static int polycombineCmd (ClientData data, Tcl_Interp *interp,
			   int argc, char *argv[])
{
  DYN_LIST *subj, *clip, *retlist;
  int result;
  gpc_polygon subject_polygon, clip_polygon, result_polygon;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " subject_poly clip_poly", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &subj) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &clip) != TCL_OK) return TCL_ERROR;

  result = dynListToPolygon(subj, &subject_polygon);
  if (result < 0) {
    Tcl_AppendResult(interp, argv[0], ": cannot create polygon from ",
		     argv[1], (char *) NULL);
    return TCL_ERROR;
  }

  result = dynListToPolygon(clip, &clip_polygon);
  if (result < 0) {
    Tcl_AppendResult(interp, argv[0], ": cannot create polygon from ",
		     argv[1], (char *) NULL);
    return TCL_ERROR;
  }

  gpc_polygon_clip((gpc_op) data, 
		   &subject_polygon, &clip_polygon, &result_polygon);

  retlist = polygonToDynList(&result_polygon, 1); /* convert and close */
  if (!retlist) {
    Tcl_AppendResult(interp, argv[0], ": cannot create dynlist from polygon",
		     (char *) NULL);
    return TCL_ERROR;
  }

  gpc_free_polygon(&subject_polygon);
  gpc_free_polygon(&clip_polygon);
  gpc_free_polygon(&result_polygon);

  return(tclPutList(interp, retlist));
}


/*****************************************************************************
 *
 * FUNCTION
 *    polygonSelfIntersectsCmd
 *
 * ARGS
 *    Tcl Args
 *
 * TCL FUNCTION
 *    curves::polygonSelfIntersects
 *
 * DESCRIPTION
 *    Test for self intersection
 *
 ****************************************************************************/

static int polygonSelfIntersectsCmd (ClientData data, Tcl_Interp *interp,
				     int argc, char *argv[])
{
  DYN_LIST *xp, *yp;
  int result;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " xvals yvals", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &xp) != TCL_OK) return TCL_ERROR;
  if (tclFindDynList(interp, argv[2], &yp) != TCL_OK) return TCL_ERROR;

  if (DYN_LIST_N(xp) != DYN_LIST_N(yp)) {
    Tcl_AppendResult(interp, argv[0], ": x/y lists must be of same length",
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  result = polygon_self_intersects(DYN_LIST_N(xp), 
				   (float *) DYN_LIST_VALS(xp), 
				   (float *) DYN_LIST_VALS(yp));
  Tcl_SetObjResult(interp, Tcl_NewIntObj(result));

  return(TCL_OK);
}


/*****************************************************************************
 *
 * EXPORT
 *
 *****************************************************************************/

#ifdef WIN32
EXPORT(int,Curve_Init) (Tcl_Interp *interp)
#else
int Curve_Init(Tcl_Interp *interp)
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

  if (Tcl_PkgProvide(interp, "curve", "1.1") != TCL_OK) {
    return TCL_ERROR;
  }

  Tcl_Eval(interp, "namespace eval curve {}");
  Tcl_CreateCommand(interp, "curve::cubic", 
		    (Tcl_CmdProc *) cubicCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::bezier", 
		    (Tcl_CmdProc *) bezierCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::closestPoint", 
		    (Tcl_CmdProc *) closestPointCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /*
   * GPC supported routines
   */
  Tcl_CreateCommand(interp, "curve::polygonUnion", 
		    (Tcl_CmdProc *) polycombineCmd, 
		    (ClientData) GPC_UNION, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::polygonIntersection", 
		    (Tcl_CmdProc *) polycombineCmd, 
		    (ClientData) GPC_INT, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::polygonXOR", 
		    (Tcl_CmdProc *) polycombineCmd, 
		    (ClientData) GPC_XOR, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::polygonDifference", 
		    (Tcl_CmdProc *) polycombineCmd, 
		    (ClientData) GPC_DIFF, (Tcl_CmdDeleteProc *) NULL);


  Tcl_CreateCommand(interp, "curve::polygonSelfIntersects", 
		    (Tcl_CmdProc *) polygonSelfIntersectsCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "curve::polygonCentroid", 
		    (Tcl_CmdProc *) polygonCentroidCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "curve::polygonClose", 
		    (Tcl_CmdProc *) polygonCloseCmd, 
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  add_clipper_commands(interp);

  return TCL_OK;
}





