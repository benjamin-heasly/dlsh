/**************************************************************************\
*  cg_mswin.c -- create a cgraph window for dlsh under Windows().
*
\**************************************************************************/

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <stdlib.h>
#include <process.h>
#include <math.h>
#include <tcl.h>

#include "cgraph.h"
#include <gbuf.h>

#pragma warning (disable:4244)
#pragma warning (disable:4305)

#define MAX_COLOR_INDICES 32
static unsigned char ColorTableVals[MAX_COLOR_INDICES][4] = {
/* R    G    B   Grey */
  0, 0, 0, 0,
  26, 26, 102, 102,
  0, 89, 0, 26,
  0, 179, 179, 179,
  204, 13, 0, 77,
  204, 0, 204, 0,
  0, 0, 0, 0,
  255, 255, 255, 255,
  179, 179, 179, 179,
  77, 115, 230, 0,
  13, 242, 26, 0,
  0, 230, 230, 230,
  0, 0, 0, 0,
  0, 0, 0, 0,
  240, 240, 13, 204,
  0, 0, 0, 51,
  255, 255, 255, 255,
  245, 245, 245, 245,
};
static int NColorVals = 18;

static char	className[] = "dlsh";

struct dl_win_ctx {
  int width, height;
  char name[64];
  HANDLE hThread;
  HWND hWnd;
  HANDLE createEvent;
  FRAME fr;
  GBUF_DATA gb;
  struct dl_win_ctx *next ;
};

typedef struct dl_win_ctx DPC;

static DPC *first_dpc = NULL;
static DPC *default_dpc = NULL;
static DPC *current_dpc = NULL;

HANDLE NewWindow(DPC *inst);
int dl_win_destroy(DPC * inst);

static PLOGFONT Plf;		/* Local font structure */
static HFONT hCurrentFont = NULL;

static PLOGPEN Pen;		/* Pen structure        */
static HPEN hCurrentPen = NULL;

static PLOGBRUSH Brush;
static HBRUSH hCurrentBrush = NULL;
static HBRUSH hHollowBrush = NULL;
static HBRUSH hBackgroundBrush = NULL;

static BYTE CurColor[3] = { 255, 255, 255 };

/**************************************************************************
*
*  function:  WndProc()
*
*  input parameters:  normal window procedure parameters.
*
*  global variables: none.
*
**************************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, 
			     WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    
    
    /**********************************************************************\
     *  WM_CREATE
     *
     * Initialize the static variables.
     \**********************************************************************/
  case WM_CREATE:
    Pen = (PLOGPEN) LocalAlloc(LPTR, sizeof(LOGPEN)); 
    Pen->lopnColor = RGB(200,200,200);
    Pen->lopnWidth.x = 1;
    Pen->lopnStyle = PS_SOLID;
    hCurrentPen = CreatePenIndirect(Pen); 

    Plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT)); 
    lstrcpy(Plf->lfFaceName, "Arial"); 
    Plf->lfWeight = FW_NORMAL; 
    hCurrentFont = CreateFontIndirect(Plf); 

    Brush = (PLOGBRUSH) LocalAlloc(LPTR, sizeof(LOGBRUSH)); 
    Brush->lbStyle = BS_SOLID;
    Brush->lbColor = RGB(200,200,200);
    hCurrentBrush = CreateBrushIndirect(Brush);

    hHollowBrush = GetStockObject(HOLLOW_BRUSH);
    hBackgroundBrush = GetStockObject(BLACK_BRUSH);
    break;
    
    
    /**********************************************************************\
     *  WM_DESTROY
     *
     * Complement of the WM_CREATE message.  Delete the pens that were
     *  created and then call postquitmessage.
     \**********************************************************************/
  case WM_DESTROY:
    if (hCurrentFont) DeleteObject (hCurrentFont);
    LocalFree((LOCALHANDLE) Plf); 

    if (hCurrentPen) DeleteObject (hCurrentPen);
    LocalFree((LOCALHANDLE) Pen); 

    if (hCurrentBrush) DeleteObject (hCurrentBrush);
    LocalFree((LOCALHANDLE) Brush); 

    PostQuitMessage(0);
    break;

    /**********************************************************************\
     *  WM_PAINT
     \**********************************************************************/
  case WM_PAINT: 
    {
      static int been_here = 0;
      HDC hdc;
      PAINTSTRUCT ps;
      static RECT  rect, oldrect;

      hdc = BeginPaint(hwnd, &ps);
      GetClientRect (hwnd, &rect);
      
      if (!been_here) {
	GetClientRect(hwnd, &oldrect);
	been_here = 1;
      }
      else {
	if (oldrect.right != rect.right || oldrect.bottom != rect.bottom) {
	  oldrect = rect;
	  setresol(rect.right, rect.bottom);
	  if (current_dpc) {
	    current_dpc->width = rect.right;
	    current_dpc->height = rect.bottom;
	  }
	  setfviewport(0, 0, 1, 1);
	}
	gbPlaybackGevents();
      }
      EndPaint (hwnd, &ps);
    } return FALSE;
    break;
  } /* end switch */
  return (DefWindowProc(hwnd, message, wParam, lParam));
}


int NewWindowThread(DPC *inst)
{
  MSG msg;
  HWND hWnd;
  WNDCLASS wndClass;
  HINSTANCE hCurrInst = GetModuleHandle(NULL);
  RECT r;
  
  /* register window class */
  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = WndProc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = 0;
  wndClass.hInstance = hCurrInst;
  wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndClass.hbrBackground = GetStockObject(BLACK_BRUSH);
  wndClass.lpszMenuName = NULL;
  wndClass.lpszClassName = className;
  RegisterClass(&wndClass);
  
  /* create window */

  r.left = 0;
  r.right = inst->width;
  r.top = 0;
  r.bottom = inst->height;
  AdjustWindowRect(&r, 
		   WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		   FALSE);

  hWnd = CreateWindow(
		      className, inst->name,
		      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		      CW_USEDEFAULT, CW_USEDEFAULT, 
		      r.right-r.left+8, r.bottom-r.top+27,
		      NULL, NULL, 
		      hCurrInst, inst);

  inst->hWnd = hWnd;

  ShowWindow(hWnd, SW_SHOWNORMAL);

  /* This wakes up the calling thread to say we're ready to roll */
  SetEvent(inst->createEvent);

  /* process messages */
  while (1) {
    if (GetMessage(&msg, NULL, 0, 0) != TRUE) {
      return msg.wParam;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}


HANDLE NewWindow(DPC *inst)
{
  HANDLE handles[2];
  int thread_id;


  /* Create a synchronizing event */
  inst->createEvent = CreateEvent( 
				  NULL,         /* no security attributes    */
				  FALSE,        /* manual-reset event        */
				  FALSE,        /* initial state is off      */
				  NULL          /* object name               */
				  ); 

  inst->hThread = CreateThread(NULL, 0, 
			       (LPTHREAD_START_ROUTINE) NewWindowThread,
			       inst, 0, &thread_id);

  handles[0] = inst->hThread;
  handles[1] = inst->createEvent;

  WaitForMultipleObjects(2, handles, FALSE, INFINITE);
  CloseHandle(inst->createEvent);

  return inst->hThread;
}


static void GS_Clearwin(void)
{
  HDC hdc;
  RECT  rect;

  if (current_dpc) {
    GetClientRect(current_dpc->hWnd, &rect);
    hdc = GetDC (current_dpc->hWnd);
    SelectObject(hdc, hBackgroundBrush);
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    ReleaseDC(current_dpc->hWnd, hdc);
  }
}


static void GS_Setcolor(int index)
{
  HDC hdc;
  unsigned int shifted;

  if (!current_dpc) return;
  shifted = index >> 5;	/* RGB colors are store in bits 6-30 */
  hdc = GetDC (current_dpc->hWnd);

  if (index < MAX_COLOR_INDICES) {
    CurColor[0] = ColorTableVals[index][0];
    CurColor[1] = ColorTableVals[index][1];
    CurColor[2] = ColorTableVals[index][2];
  }
  else {
    CurColor[0] = ((shifted & 0xff0000) >> 16);
    CurColor[1] = ((shifted & 0xff00) >> 8);
    CurColor[2] = (shifted & 0xff);
  }

  Pen->lopnColor = RGB(CurColor[0], CurColor[1], CurColor[2]);
  if (hCurrentPen) DeleteObject(hCurrentPen);
  hCurrentPen = CreatePenIndirect(Pen);

  Brush->lbColor = RGB(CurColor[0], CurColor[1], CurColor[2]);
  if (hCurrentBrush) DeleteObject(hCurrentBrush);
  hCurrentBrush = CreateBrushIndirect(Brush);

  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Linewidth(int width)
{
  HDC hdc;
  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  Pen->lopnWidth.x = width/100;
  if (hCurrentPen) DeleteObject(hCurrentPen);
  hCurrentPen = CreatePenIndirect(Pen);

  ReleaseDC (current_dpc->hWnd, hdc);
}


static void GS_Point(float x, float y) 
{
  HDC hdc;

  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);
  SelectObject(hdc, hCurrentPen);
  MoveToEx (hdc, (int) x, (int) y, NULL);
  LineTo(hdc, (int) x, (int) current_dpc->height-y);
  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Line(float x0, float y0, float x1, float y1) 
{
  HDC hdc;

  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);
  SelectObject(hdc, hCurrentPen);
  MoveToEx (hdc, (int) x0, (int) current_dpc->height-y0, NULL);
  LineTo(hdc, (int) x1, (int) current_dpc->height-y1);
  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Circle(float x, float y, float width, int filled) 
{
  HDC hdc;
  
  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  SelectObject(hdc, hCurrentPen);
  if (filled) SelectObject(hdc, hCurrentBrush);
  else SelectObject(hdc, hHollowBrush);
  
  Ellipse (hdc, (int) x, current_dpc->height-y+width, 
	   (int) (x+width), (int) current_dpc->height-y);
  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Poly(float * verts, int nverts) 
{
  int i;
  HDC hdc;
  POINT *svPoints;
  
  if (nverts < 2) return;

  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  SelectObject(hdc, hCurrentPen);
  SelectObject(hdc, hCurrentBrush);
  
  if (!(svPoints = (POINT *) calloc(nverts, sizeof(POINT)))) return;
  
  /* flip all the y positions into device coords */
  
  for (i = 0; i < nverts; i++) { 
    svPoints[i].x = verts[2*i];
    svPoints[i].y = current_dpc->height - verts[2*i+1];
  }

  Polygon (hdc, &svPoints[0], nverts);
  
  free((void *) svPoints);
}


static void GS_Text(float x, float y, char *str)
{
  static int GS_Strheight(char *str);
  static int GS_Strwidth(char *str);
  HDC hdc;
  float voff = 0, hoff = 0;
  int ori;

  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  ori = getframe()->orientation;
  if (ori == 0 || ori == 2) {	/* horizontal */
    switch (getframe()->just) {
    case LEFT_JUST:     SetTextAlign(hdc, TA_LEFT | TA_BOTTOM); break;
    case RIGHT_JUST:    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM); break;
    case CENTER_JUST :  SetTextAlign(hdc, TA_CENTER | TA_BOTTOM); break;
    }
    voff = GS_Strheight(str)/2.0;
  }
  else {
    switch (getframe()->just) {
    case LEFT_JUST:     SetTextAlign(hdc, TA_LEFT | TA_BOTTOM); break;
    case RIGHT_JUST:    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM); break;
    case CENTER_JUST :  SetTextAlign(hdc, TA_CENTER | TA_BOTTOM); break;
    }
    hoff = GS_Strheight(str)/2.0;
  }
  
  SelectObject(hdc, hCurrentFont);
  SetTextColor(hdc, RGB(CurColor[0],CurColor[1],CurColor[2]));
  SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, (int) x+hoff, (int) current_dpc->height-y+voff, 
	  str, strlen(str));
  SetBkMode(hdc, OPAQUE);
  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Char(float x, float y, char *str)
{
  HDC hdc;

  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  SelectObject(hdc, hCurrentFont);
  SetTextColor(hdc, RGB(CurColor[0],CurColor[1],CurColor[2]));
  SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, (int) x, (int) current_dpc->height-y, str, strlen(str));
  SetBkMode(hdc, OPAQUE);

  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Setfont(char *name, float size)
{
  float x, y;
  float scale;			/* For scaling according to window size */
  HDC hdc;
  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);

  getresol(&x, &y);

  scale = x/850.;
  if (scale < .6) scale = .6;
  if (scale > 1) scale = 1;

  lstrcpy(Plf->lfFaceName, name); 
  Plf->lfHeight = -MulDiv((int) (scale*size), 
			  GetDeviceCaps(hdc, LOGPIXELSY), 72);
  if (hCurrentFont) DeleteObject(hCurrentFont); 
  hCurrentFont = CreateFontIndirect(Plf); 
  ReleaseDC (current_dpc->hWnd, hdc);
}

static void GS_Setorient(int ori)
{
  HDC hdc;
  if (!current_dpc) return;
  hdc = GetDC (current_dpc->hWnd);
  Plf->lfEscapement = ori*90*10;
  if (hCurrentFont) DeleteObject(hCurrentFont); 
  hCurrentFont = CreateFontIndirect(Plf); 
  ReleaseDC (current_dpc->hWnd, hdc);
}

static int GS_Strwidth(char *str)
{
  HDC hdc;
  SIZE sz;
  if (!current_dpc) return 1;
  hdc = GetDC (current_dpc->hWnd);
  GetTextExtentPoint32(hdc, str, strlen(str), &sz);
  ReleaseDC (current_dpc->hWnd, hdc);
  return sz.cx;
}

static int GS_Strheight(char *str)
{
  HDC hdc;
  SIZE sz;
  if (!current_dpc) return 1;
  hdc = GetDC (current_dpc->hWnd);

  GetTextExtentPoint32(hdc, str, strlen(str), &sz);
  ReleaseDC (current_dpc->hWnd, hdc);
  return sz.cy;
}

void dl_win_init_frame(DPC *inst, int w, int h)
{
  RECT size;

  setclearfunc((HANDLER) GS_Clearwin);
  setpoint((PHANDLER) GS_Point) ;
  setline((LHANDLER) GS_Line) ;
  setfilledpoly((FHANDLER) GS_Poly) ;
  setcircfunc((CHANDLER) GS_Circle) ;

  /* This is more "powerful" than setchar, and will override */
  settext((THANDLER) GS_Text);

  setchar((THANDLER) GS_Char);
  strwidthfunc((SWHANDLER) GS_Strwidth);
  strheightfunc((SHHANDLER) GS_Strheight);

  setfontfunc((SFHANDLER) GS_Setfont);
  setorientfunc((SOHANDLER) GS_Setorient);
  setcolorfunc((COHANDLER) GS_Setcolor);

  setlwidthfunc((LWHANDLER) GS_Linewidth);


  GetClientRect (inst->hWnd, &size);
  setresol((float) size.right, (float) size.bottom) ;

  gbInitGevents();

  setjust(CENTER_JUST) ;
  setfont("Helvetica", 10) ;
  setcolor(7) ;
  setlstyle(1) ;
  setlwidth(100) ;
  setorientation(0);
  setwindow(0, 0, size.right, size.bottom) ;
  setfviewport(0, 0, 1, 1) ;
}

void dl_win_init_dpc(DPC *inst, int w, int h, char *name)
{
  GBUF_DATA * gb = gbGetGeventBuffer() ;
  
  inst->width =  w;
  inst->height = h;

  gbDisableGevents() ;				  /* valid data */
  gbInitGeventBuffer(&inst->gb) ;
  gbSetGeventBuffer(&inst->gb) ;
  gbEnableGevents() ;

  setframe(&inst->fr) ;

  strncpy(inst->name, name, 63);
  NewWindow(inst);
  dl_win_init_frame(inst, w, h) ;
  inst->next = first_dpc;
  first_dpc = inst;
}


int dl_win_select_dpc(DPC *inst)
{
  DPC *d ;
  
  if (inst == NULL) d = default_dpc ;
  else for (d = first_dpc ; d != inst && d != NULL ; d = d->next) ;
  current_dpc = d ;
  if (d == NULL) return(0) ;
  setstatus(&d->fr);
  gbSetGeventBuffer(& d->gb) ;
  gbEnableGevents() ;
  user() ;
  return(1) ;
}


DPC *dl_win_create(int width, int height, char *name)
{
  DPC *new_dpc;
  
  if (!(new_dpc = calloc(1, sizeof(DPC)))) return(NULL);
  dl_win_init_dpc(new_dpc, width, height, name);
  current_dpc = new_dpc;
  dl_win_select_dpc(new_dpc);
  return(new_dpc);
}


int dl_win_destroy(DPC * inst)
{
  GBUF_DATA * gb ;
  DPC *d;
  
  if (first_dpc == NULL) return(0) ;
  if (inst == first_dpc) first_dpc = inst->next ;
  else {
    for (d = first_dpc ; d->next != inst && d->next != NULL ;
	 d = d->next) ;
    if (d->next == NULL) return(0) ;
    d->next = d->next->next ;
  }
  gbDisableGevents() ;
  if (inst == current_dpc)
    dl_win_select_dpc(NULL) ;
  gb = gbGetGeventBuffer() ;
  gbSetGeventBuffer(& inst->gb) ;
  gbCloseGevents() ;
  gbSetGeventBuffer(gb) ;
  
  PostMessage(inst->hWnd, WM_QUIT, 0, 0);
  free(d) ;

  return(1) ;
}


void dl_win_resize(int w, int h)
{

}


static int tclWinCreate(ClientData clientData, Tcl_Interp * interp, int argc,
		 char * argv[])
{
  int width = 640, height = 480;
  char *name = "dlsh";

  if (current_dpc) {
    Tcl_AppendResult(interp, argv[0], ": graphics window already exists",
		     NULL);
    return TCL_ERROR;
  }

  if (argc > 1) {
    if (Tcl_GetInt(interp, argv[1], &width) != TCL_OK) return TCL_ERROR;
  }
  if (argc > 2) {
    if (Tcl_GetInt(interp, argv[2], &height) != TCL_OK) return TCL_ERROR;
  }
  if (argc > 3) {
    name = argv[3];
  }

  sprintf(interp->result, "%d", (int) dl_win_create(width, height, name)) ;
  return(TCL_OK);
}


static int tclWinDelete(ClientData clientData, Tcl_Interp * interp, int argc,
		 char * argv[])
{
  DPC *inst;

  if (argc < 2) {
    Tcl_AppendResult(interp, "Usage: ", argv[0],
		     " id", (char *) NULL) ;
    return(TCL_ERROR) ;
  }
  if (Tcl_GetInt(interp, argv[1], (int *) &inst) != TCL_OK) {
    return(TCL_ERROR) ;
  }
  if (!dl_win_destroy(inst))
    {
      Tcl_AppendResult(interp, argv[0], " : id is not valid",
		       (char *) NULL) ;
      return(TCL_ERROR) ;
    }
  return(TCL_OK) ;
}


static int tclWinSelect(ClientData clientData, Tcl_Interp * interp, int argc,
		 char * argv[])
{
  DPC *inst;
  
  if (argc < 2) {
    Tcl_AppendResult(interp, "Usage: ", argv[0],
		     " id", (char *) NULL) ;
    return(TCL_ERROR) ;
  }
  if (Tcl_GetInt(interp, argv[1], (int *) &inst) != TCL_OK) {
    return(TCL_ERROR) ;
  }
  if (!dl_win_select_dpc(inst))
    {
      Tcl_AppendResult(interp, argv[0], " : id is not valid",
		       (char *) NULL) ;
      return(TCL_ERROR) ;
    }
  return(TCL_OK) ;
}


static int tclWinResize(ClientData clientData, Tcl_Interp * interp, int argc,
		 char * argv[])
{
  int w, h ;
  
  if (argc < 3) {
    Tcl_AppendResult(interp, "usage: ", argv[0],
		     " width height", (char *) NULL) ;
    return(TCL_ERROR) ;
  }
  if (Tcl_GetInt(interp, argv[1], &w) != TCL_OK) return TCL_ERROR;
  if (Tcl_GetInt(interp, argv[2], &h) != TCL_OK) return TCL_ERROR;
  dl_win_resize(w, h) ;
  return(TCL_OK) ;
}

static int tclFlushWin(ClientData clientData, Tcl_Interp * interp, int argc,
		char * argv[])
{
  return(TCL_OK) ;
}

#ifdef WIN32
extern _declspec(dllexport) int Cg_mswin_Init(Tcl_Interp *interp)
#else
int Cg_mswin_Init(Tcl_Interp * interp)
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

  Tcl_CreateCommand(interp, "flushwin", tclFlushWin, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL) ;
  Tcl_CreateCommand(interp, "createwin", tclWinCreate, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL) ;
  Tcl_CreateCommand(interp, "deletewin", tclWinDelete, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL) ;
  Tcl_CreateCommand(interp, "selectwin", tclWinSelect, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL) ;
  Tcl_CreateCommand(interp, "resizewin", tclWinResize, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL) ;

  /* This is a way to guarantee the dlsh.dll is loaded */
  /*  Tcl_VarEval(interp, "dl_noOp", NULL); */

  return(TCL_OK) ;
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

