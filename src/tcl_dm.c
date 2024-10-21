/*****************************************************************************
 *
 * NAME
 *      tcl_dm.c
 *
 * PURPOSE
 *      Provide code for creating/manipulating DynLists as matrices
 *
 * AUTHOR
 *      DLS, JUL-95
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(SUN4) || defined(LYNX)
#include <unistd.h>
#endif

#include <tcl.h>
#include <df.h>
#include <dfana.h>
#include <labtcl.h>
#include <tcl_dl.h>

#include <lablib.h>
#include <utilc.h>


enum DM_DUMP_TYPES   { DM_DUMP, DM_DUMP_IN_COLS };
enum DM_ARITH_TYPES  { DM_MULT, DM_DIV, DM_ADD, DM_SUB };
enum DM_GENERATORS   { DM_IDENTITY, DM_ZEROS, DM_URANDS, DM_ZRANDS };
enum DM_MAT_FROM_MAT { DM_TRANSPOSE, DM_INVERSE, DM_LUINV, DM_LUDCMP, DM_DIAG};
enum DM_MEAN_TYPES   { DM_ROWS, DM_COLS };

/*****************************************************************************
 *                           TCL Bound Functions 
 *****************************************************************************/

static int tclDynMatrixReshape       (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixGenerate      (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixDump          (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixDims          (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixFromMatrix    (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixArith         (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixMeans         (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixSums          (ClientData, Tcl_Interp *, int, char **);
static int tclDynMatrixCenter        (ClientData, Tcl_Interp *, int, char **);
static int tclDMHelp                 (ClientData, Tcl_Interp *, int, char **);

static TCL_COMMANDS DMcommands[] = {
  { "dm_dump",              tclDynMatrixDump,        (void *) DM_DUMP,
      "dump values of matrix" },
  { "dm_dumpInCols",        tclDynMatrixDump,        (void *) DM_DUMP_IN_COLS,
      "dump values of matrix in columns" },

  { "dm_identity",          tclDynMatrixGenerate,    (void *) DM_IDENTITY,
      "create an n x n identity" },
  { "dm_zeros",             tclDynMatrixGenerate,    (void *) DM_ZEROS,
      "create a zero filled matrix" },
  { "dm_urand",             tclDynMatrixGenerate,    (void *) DM_URANDS,
      "fill matrix with uniformly distributed rands" },
  { "dm_zrand",             tclDynMatrixGenerate,    (void *) DM_ZRANDS,
      "fill matrix with normally distributed rands" },

  { "dm_dims",              tclDynMatrixDims,        NULL,
      "dump values of matrix" },
  { "dm_transpose",         tclDynMatrixFromMatrix, (void *) DM_TRANSPOSE,
      "returns transpose of a matrix" },
  { "dm_reshape",           tclDynMatrixReshape,    NULL,
      "reshapes matrix to new dimensions" },
  { "dm_add",              tclDynMatrixArith,       (void *) DM_ADD,
      "returns matrix + operand" },
  { "dm_sub",              tclDynMatrixArith,       (void *) DM_SUB,
      "returns matrix - operand" },
  { "dm_mult",              tclDynMatrixArith,      (void *) DM_MULT,
      "returns matrix * operand" },
  { "dm_div",              tclDynMatrixArith,       (void *) DM_DIV,
      "returns matrix / operand" },

  { "dm_rowSums",         tclDynMatrixSums,         (void *) DM_ROWS,
      "returns sums of rows" },
  { "dm_colSums",         tclDynMatrixSums,         (void *) DM_COLS,
      "returns sums of cols" },

  { "dm_rowMeans",         tclDynMatrixMeans,       (void *) DM_ROWS,
      "returns means of rows" },
  { "dm_colMeans",         tclDynMatrixMeans,       (void *) DM_COLS,
      "returns means of cols" },

  { "dm_centerRows",       tclDynMatrixCenter,      (void *) DM_ROWS,
      "center matrix by rows" },
  { "dm_centerCols",       tclDynMatrixCenter,      (void *) DM_COLS,
      "center matrix by cols" },

  { "dm_inverse",          tclDynMatrixFromMatrix,  (void *) DM_INVERSE,
      "returns inverse of a matrix" },
  { "dm_luinverse",        tclDynMatrixFromMatrix,  (void *) DM_LUINV,
      "returns inverse of a matrix using LU decomp" },
  { "dm_diag",             tclDynMatrixFromMatrix,  (void *) DM_DIAG,
      "returns diag of a matrix" },
  { "dm_ludcmp",           tclDynMatrixFromMatrix,  (void *) DM_LUDCMP,
      "returns LU decomposition of a matrix" },
  
  { "dm_help",              tclDMHelp,             (void *) DMcommands, 
      "display help for matrix operations" },
  { NULL, NULL, NULL, NULL }
};



/*****************************************************************************
 *
 * FUNCTION
 *    Dm_Init
 *
 * ARGS
 *    Tcl_Inter *interp
 *
 * DESCRIPTION
 *    Creates tcl commands for creating and manipulating dynLists & dynGroups
 * and initializes two static hash table for referring to them.
 *
 *****************************************************************************/

int Dm_Init(Tcl_Interp *interp)
{
  int i = 0;

  while (DMcommands[i].name) {
    Tcl_CreateCommand(interp, DMcommands[i].name,
		      (Tcl_CmdProc *) DMcommands[i].func, 
		      (ClientData) DMcommands[i].cd, 
		      (Tcl_CmdDeleteProc *) NULL);
    i++;
  }
  return TCL_OK;
}

/*****************************************************************************
 *
 * FUNCTION
 *    tclDMHelp
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Loop through DMCommands and print out help messages
 *
 *****************************************************************************/

static int tclDMHelp (ClientData cd, Tcl_Interp *interp, 
                      int argc, char *argv[])
{
  TCL_COMMANDS *commands = (TCL_COMMANDS *) cd;
  int i = 0;

  while (commands[i].name) {
    printf("%-22s - %s\n", commands[i].name, commands[i].desc);
    i++;
  }
  return TCL_OK;
}

/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixIdentity
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return identity matrix
 *
 *****************************************************************************/

static int tclDynMatrixGenerate (ClientData cd, Tcl_Interp *interp, 
				 int argc, char *argv[])
{
  DYN_LIST *m = NULL;
  int nrows, ncols;
  int operation = (Tcl_Size) cd;
  
  switch (operation) {
  case DM_IDENTITY:
    if (argc < 2) {
      Tcl_AppendResult(interp, "usage: ", argv[0], " nrows", 
		       (char *) NULL);
      return TCL_ERROR;
    }
    else if (Tcl_GetInt(interp, argv[1], &nrows) != TCL_OK) return TCL_ERROR;
    break;

  case DM_ZEROS:
  case DM_URANDS:
  case DM_ZRANDS:
    if (argc < 2) {
      Tcl_AppendResult(interp, "usage: ", argv[0], " nrows [ncols]",
		       (char *) NULL);
      return TCL_ERROR;
    }
    else if (Tcl_GetInt(interp, argv[1], &nrows) != TCL_OK) return TCL_ERROR;
    if (argc > 2) {
      if (Tcl_GetInt(interp, argv[2], &ncols) != TCL_OK) return TCL_ERROR;
    }
    else ncols = nrows;
    break;
  }

  switch (operation) {
  case DM_IDENTITY:
    m = dynMatrixIdentity(nrows);
    break;
  case DM_ZEROS:
    m = dynMatrixZeros(nrows, ncols);
    break;
  case DM_URANDS:
    m = dynMatrixUrands(nrows, ncols);
    break;
 case DM_ZRANDS:
    m = dynMatrixZrands(nrows, ncols);
    break;
  }
  if (!m) {
    Tcl_AppendResult(interp, argv[0], ": error creating output matrix",
		     (char *) NULL);
    return TCL_ERROR;
  }
  else return(tclPutList(interp, m));
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixDump
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Dump matrix
 *
 *****************************************************************************/

static int tclDynMatrixDump (ClientData cd, Tcl_Interp *interp, 
			     int argc, char *argv[])
{
  DYN_LIST *m;
  Tcl_Channel outChannel = NULL ;
  int separator = '\t';
  int output_format = (Tcl_Size) cd;
  int status = 0;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix [fileID]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) return TCL_ERROR;
  
  if (argc > 2) {
    int mode;
    if ((outChannel = Tcl_GetChannel(interp, argv[2], &mode)) == NULL) {
      return TCL_ERROR;
    }
  }
  if (argc > 3) {
    if (Tcl_GetInt(interp, argv[3], &separator) != TCL_OK) {
      return TCL_ERROR;
    }
    else if (separator > 255) {
      Tcl_AppendResult(interp, argv[0], 
		       ": separator must be between 0 and 255",
		       (char *) NULL);
      return TCL_ERROR;
    }
  }
  
  switch (output_format) {
  case DM_DUMP:
    status = dynListChanDumpMatrix(interp, m, outChannel, (char) separator);
    break;
  case DM_DUMP_IN_COLS:
    status = dynListChanDumpMatrixInCols(interp, m, outChannel,
    	(char) separator);
    break;
  }
  if (!status) {
    Tcl_AppendResult(interp, argv[0], ": invalid matrix", (char *) NULL);
    return TCL_ERROR;
  }
  
  return TCL_OK;
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixDims
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return dims of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixDims (ClientData cd, Tcl_Interp *interp, 
			     int argc, char *argv[])
{
  DYN_LIST *m;
  int rows, cols;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) 
    return TCL_ERROR;

  if (!dynMatrixDims(m, &rows, &cols)) return TCL_ERROR;

  Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
  Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(rows));
  Tcl_ListObjAppendElement(interp, listPtr, Tcl_NewIntObj(cols));
  Tcl_SetObjResult(interp, listPtr);

  return TCL_OK;
}

/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixFromMatrix
 *
 * ARGS
 *    Tcl Args
 *
 * BOUND FUNCTIONS
 *    dm_transpose
 *    dm_inverse
 *    dm_ludcmp
 *
 * DESCRIPTION
 *    Return transpose of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixFromMatrix (ClientData cd, Tcl_Interp *interp, 
				  int argc, char *argv[])
{
  DYN_LIST *m, *newmat = NULL;
  int operation = (Tcl_Size) cd;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) 
    return TCL_ERROR;
  
  switch (operation) {
  case DM_TRANSPOSE:
    return(Tcl_VarEval(interp, "dl_transpose ", argv[1], (char *) NULL));
    break;
  case DM_DIAG:
    newmat = dynMatrixDiag(m);
    if (!newmat) {
      Tcl_AppendResult(interp, argv[0], ": error extracting diagonal of (", 
		       argv[1], ")", (char *) NULL);
      return TCL_ERROR;
    }
    break;
  case DM_INVERSE:
    {
      int nrows, ncols;
      dynMatrixDims(m, &nrows, &ncols);
      if (nrows != ncols) {
	Tcl_AppendResult(interp, argv[0], ": matrix must be square", 
			 (char *) NULL);
	return TCL_ERROR;
      }
      newmat = dynMatrixInverse(m);
      if (!newmat) {
	Tcl_AppendResult(interp, argv[0], ": singular matrix (", argv[1], ")", 
			 (char *) NULL);
	return TCL_ERROR;
      }
    }
    break;
  case DM_LUINV:
    {
      int nrows, ncols;
      dynMatrixDims(m, &nrows, &ncols);
      if (nrows != ncols) {
	Tcl_AppendResult(interp, argv[0], ": matrix must be square", 
			 (char *) NULL);
	return TCL_ERROR;
      }
      newmat = dynMatrixLUInverse(m);
      if (!newmat) {
	Tcl_AppendResult(interp, argv[0], ": singular matrix (", argv[1], ")", 
			 (char *) NULL);
	return TCL_ERROR;
      }
    }
    break;
  case DM_LUDCMP:
    {
      int nrows, ncols;
      dynMatrixDims(m, &nrows, &ncols);
      if (nrows != ncols) {
	Tcl_AppendResult(interp, argv[0], ": matrix must be square", 
			 (char *) NULL);
	return TCL_ERROR;
      }
      newmat = dynMatrixLudcmp(m);
      if (!newmat) {
	Tcl_AppendResult(interp, argv[0], ": singular matrix (", argv[1], ")", 
			 (char *) NULL);
	return TCL_ERROR;
      }
    }
    break;
  }
  return(tclPutList(interp, newmat));
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixReshape
 *
 * ARGS
 *    Tcl Args
 *
 * BOUND FUNCTIONS
 *    dm_reshape
 *
 * DESCRIPTION
 *    Return reshaped matrix from a list or another matrix
 *
 *****************************************************************************/

static int tclDynMatrixReshape (ClientData cd, Tcl_Interp *interp, 
				int argc, char *argv[])
{
  DYN_LIST *l, *newmat;
  int nrows, ncols, total_elements = 0;
  
  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix nrows [ncols]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, argv[1], &l) != TCL_OK) 
    return TCL_ERROR;
  
  if (Tcl_GetInt(interp, argv[2], &nrows) != TCL_OK) return TCL_ERROR;
  if (nrows <= 0) {
    Tcl_AppendResult(interp, argv[0], ": nrows must be greater than 0", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  switch(DYN_LIST_DATATYPE(l)) {
  case DF_LONG:
  case DF_SHORT:
  case DF_CHAR:
  case DF_FLOAT:
    total_elements = DYN_LIST_N(l);
    break;
  case DF_LIST:
    {
      int n, m;
      if (dynListIsMatrix(l)) {
	dynMatrixDims(l, &n, &m);
	total_elements = n*m;
      }
      else {
	Tcl_AppendResult(interp, argv[0], ": cannot reshape list \"", 
			 argv[1], "\"", (char *) NULL);
	return TCL_ERROR;
	
      }
    }
    break;
  case DF_STRING:
    Tcl_AppendResult(interp, argv[0], ": cannot reshape string", 
		     (char *) NULL);
    return TCL_ERROR;
    break;
  }

  if (argc > 3) {
    if (Tcl_GetInt(interp, argv[3], &ncols) != TCL_OK) return TCL_ERROR;
  }
  else {
    ncols = total_elements / nrows;
  }
  
  if ((ncols * nrows) != total_elements) {
    char resultstr[128];
    snprintf(resultstr, sizeof(resultstr),
	     "%s: cannot create a %dx%d matrix from %d elements",
	     argv[0], nrows, ncols, total_elements);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(resultstr, -1));
    return TCL_ERROR;
  }
  
  newmat = dynMatrixReshape(l, nrows, ncols);

  return(tclPutList(interp, newmat));
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixArith
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return transpose of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixArith (ClientData cd, Tcl_Interp *interp, 
			      int argc, char *argv[])
{
  DYN_LIST *m, *l, *newmat = NULL;
  double k;
  int operation = (Tcl_Size) cd;

  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix [matrix|vec|float]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) return TCL_ERROR;
  Tcl_ResetResult(interp);
  if (tclFindDynList(interp, argv[2], &l) == TCL_OK) {
    switch (operation) {
    case DM_MULT:
      if (!(newmat = dynMatrixMultiply(m, l))) {
	Tcl_AppendResult(interp, "unable to multiply matrix \"", 
			 argv[1], "\" by \"", argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      break;
    case DM_DIV:
      Tcl_AppendResult(interp, "matrix division not supported", (char *) NULL);
      return TCL_ERROR;
      break;
    case DM_ADD:
      if (!(newmat = dynMatrixAdd(m, l))) {
	Tcl_AppendResult(interp, "unable to add matrix \"", 
			 argv[1], "\" and \"", argv[2], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      break;
    case DM_SUB:
      if (!(newmat = dynMatrixSubtract(m, l))) {
	Tcl_AppendResult(interp, "unable to subtract \"", 
			 argv[2], "\" from \"", argv[1], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      break;
    }
  }
  else if (Tcl_GetDouble(interp, argv[2], &k) == TCL_OK) {
    switch (operation) {
    case DM_MULT:
      if (!(newmat = dynMatrixMultFloat(m, k))) {
	Tcl_AppendResult(interp, "unable to multiply matrix \"", 
			 argv[1], "\" by ", argv[2], (char *) NULL);
	return TCL_ERROR;
      }
      break;
    case DM_DIV:
      if (!(newmat = dynMatrixDivFloat(m, k))) {
	Tcl_AppendResult(interp, "unable to divide matrix \"", 
			 argv[1], "\" by ", argv[2], (char *) NULL);
	return TCL_ERROR;
      }
      break;
    case DM_ADD:
      if (!(newmat = dynMatrixAddFloat(m, k))) {
	Tcl_AppendResult(interp, "unable to add matrix \"", 
			 argv[1], "\" and ", argv[2], (char *) NULL);
	return TCL_ERROR;
      }
      break;
    case DM_SUB:
      if (!(newmat = dynMatrixSubFloat(m, k))) {
	Tcl_AppendResult(interp, "unable to subtract ", argv[2], 
			 " from matrix \"", argv[1], "\"", (char *) NULL);
	return TCL_ERROR;
      }
      break;
    }
  }
  else {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix [matrix|vec|float]", 
		     (char *) NULL);
    return TCL_ERROR;
    
  }
  
  return(tclPutList(interp, newmat));
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixSums
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return sums of rows/cols of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixSums (ClientData cd, Tcl_Interp *interp, 
			      int argc, char *argv[])
{
  DYN_LIST *newlist = NULL;
  DYN_LIST *m;
  int operation = (Tcl_Size) cd;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) 
    return TCL_ERROR;

  switch (operation) {
  case DM_ROWS: newlist = dynMatrixRowSums(m); break;
  case DM_COLS: newlist = dynMatrixColSums(m); break;
  }
  
  if (!newlist) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": error computing sums", 
		     (char *) NULL);
    return TCL_ERROR;
  }

  return(tclPutList(interp, newlist));
}



/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixMeans
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return means of rows/cols of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixMeans (ClientData cd, Tcl_Interp *interp, 
			      int argc, char *argv[])
{
  DYN_LIST *newlist = NULL;
  DYN_LIST *m;
  int operation = (Tcl_Size) cd;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) 
    return TCL_ERROR;

  switch (operation) {
  case DM_ROWS: newlist = dynMatrixRowMeans(m); break;
  case DM_COLS: newlist = dynMatrixColMeans(m); break;
  }
  
  if (!newlist) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": error computing means", 
		     (char *) NULL);
    return TCL_ERROR;
  }

  return(tclPutList(interp, newlist));
}


/*****************************************************************************
 *
 * FUNCTION
 *    tclDynMatrixCenter
 *
 * ARGS
 *    Tcl Args
 *
 * DESCRIPTION
 *    Return centered rows/cols of a matrix
 *
 *****************************************************************************/

static int tclDynMatrixCenter (ClientData cd, Tcl_Interp *interp, 
			       int argc, char *argv[])
{
  DYN_LIST *newlist = NULL;
  DYN_LIST *m, *v = NULL;
  int nrows, ncols;
  int operation = (Tcl_Size) cd;

  if (argc < 2) {
    Tcl_AppendResult(interp, "usage: ", argv[0], " matrix [vec]", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if (tclFindDynMatrix(interp, argv[1], &m) != TCL_OK) return TCL_ERROR;
  if (!dynMatrixDims(m, &nrows, &ncols)) return TCL_ERROR;

  if (argc > 2) {
    if (tclFindDynList(interp, argv[2], &v) != TCL_OK) return TCL_ERROR;
  }
  else {
    switch (operation) {
    case DM_ROWS: v = dynMatrixRowMeans(m); break;
    case DM_COLS: v = dynMatrixColMeans(m); break;
    }
  }
  if (!v) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": error computing center vector", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  if ((operation == DM_ROWS && DYN_LIST_N(v) != nrows) ||
      (operation == DM_COLS && DYN_LIST_N(v) != ncols)) {
    if (argc < 2) dfuFreeDynList(v); /* should never happen */
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": incompatible center vector", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  switch (operation) {
  case DM_ROWS: newlist = dynMatrixCenterRows(m, v); break;
  case DM_COLS: newlist = dynMatrixCenterCols(m, v); break;
  }

  /* Don't need this anymore, so delete if we created it above */
  if (argc < 2) dfuFreeDynList(v);

  if (!newlist) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, argv[0], ": error centering matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  
  return(tclPutList(interp, newlist));
}

/*****************************************************************************
 *
 * FUNCTION
 *    tclFindDynMatrix
 *
 * ARGS
 *    Tcl_Interp *interp, char *name, DYN_LIST **
 *
 * DESCRIPTION
 *    Looks for dynlist called name, and then makes sure it is a matrix.
 *
 *****************************************************************************/

int tclFindDynMatrix(Tcl_Interp *interp, char *name, DYN_LIST **dl)
{
  if (tclFindDynList(interp, name, dl) != TCL_OK) {
    Tcl_AppendResult(interp, "matrix \"", name, " does not exist", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  if (tclIsDynMatrix(interp, *dl) != TCL_OK) {
    Tcl_AppendResult(interp, "\"", name, "\" is not a valid matrix", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  return TCL_OK;
}




/*****************************************************************************
 *
 * FUNCTION
 *    tclIsDynMatrix
 *
 * ARGS
 *    Tcl_Interp *interp, DYN_LIST **
 *
 * DESCRIPTION
 *    Determines whether dynlist is an appropriate matrix.
 *
 *****************************************************************************/

int tclIsDynMatrix(Tcl_Interp *interp, DYN_LIST *dl)
{
  if (!dynListIsMatrix(dl)) return TCL_ERROR;
  return TCL_OK;
}

