/**************************************************************************/
/*      cgraph.c - graphics package for c process control system          */
/*      Created: 20-Aug-82                                                */
/*      By: Sean D. True and Doug Whittington                             */
/*      Modified to run under DOS on IBM-AT by: Nikos Logothetis,20/11/88 */
/*      Modified by Nikos K. Logothetis, 22-MAY-90: Modules CGRAPH.C and  */
/*      GUTIL1.C are squeezed in one module with the name CGRAPH.C.       */
/*      Modified by Sheinberg & Leopold, 19-DEC-93: the character handler */
/*                                                  now takes ! strings ! */
/*      Modified by DLS,  APR-94 to record graphics events for later dump */
/*      Overhauled by DLS, APR-94 to use floats instead of ints           */
/*      Adapted for LYNX by DLS & DAL 17-APR-94                           */
/*	Modified by Michael Thayer, DEC-97: added extra handlers to       */
/*                                          ``FRAME'' structure           */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <cgraph.h>
#include <math.h>

#ifdef WIN32
#pragma warning (disable:4244)
#pragma warning (disable:4305)
#pragma warning (disable:4761)
#endif

#include "gbuf.h"

#define CR 015
#define LF 012
#define TAB 011
#define FF 014
#define EOT 04


static FRAME cntxt = {0.0,0.0,640.0,480.0,0.0,0.0,1000.0,1000.0,
		      1.0,1.0,1.0,1.0,7.0,9.0,NULL,10.0, 0.0,0.0,8.0,8.0,
		      1,100,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		      NULL,NULL,NULL,NULL,
		      NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		      0.0,0.0,0.0,0.0,
		      0.0,0.0,0.0,0.0,NULL};

static int GetNorm (float *, float *);

static float temp_float;	/* used in SWAP */

static VW_STACK *ViewportStack = NULL;

FRAME *contexp = &cntxt;

#define MULDIV(x,y,z) (((x)*(y))/(z))
#define SWAP(x,y) {temp_float = x; x = y; y = temp_float;}
#define WINDOW(f,x,y) {x=f->xl+MULDIV(x-f->xul,f->xs,f->xus); \
                       y=f->yb+MULDIV(y-f->yub,f->ys,f->yus);}
#define SCREEN(f,x,y) {x=f->xul + MULDIV(x-f->xl,f->xus,f->xs); \
                       y=f->yub + MULDIV(y-f->yb,f->yus,f->ys);}
#define SCALEXY(f,x,y){x=MULDIV((x),(f->xs),(f->xus));\
                       y=MULDIV((y),(f->ys),(f->yus));}
#define CHECKMIN(x) {if (x < 0.0) x=0.0;}
#define CHECKXMX(f,x) {if (x>((f->xsres)-1.0)) x=((f->xsres)-1.0);}
#define CHECKYMX(f,x) {if (x>((f->ysres)-1.0)) x=((f->ysres)-1.0);}

HANDLER bframe = NULL;
HANDLER eframe = NULL;
static float barwidth = 10.0;
static void setvw(void);
static void linutl(float x, float y);
static int dclip(void);
static int fclip(void);		/* filled version of clip checker */

static int img_preview = 0;	/* preview or keyline ps images   */

/*********************************************************************
 *          Allow pushing & popping of frames and viewports
 *********************************************************************/

FRAME *gsave()
{
   FRAME *newframe = (FRAME *) malloc(sizeof(FRAME));
   copyframe(contexp, newframe);
   newframe->parent = contexp;
   setstatus(newframe);
   if (RecordGEvents) record_gattr(G_SAVE, 1);
   return(newframe);
}

FRAME *grestore()
{
   FRAME *oldframe = contexp;
   if (!contexp->parent) return NULL;
   setstatus(contexp->parent);
   if (oldframe->fontname) free((void *) oldframe->fontname);
   free((void *) oldframe);
   if (contexp->dsetfont)
   	contexp->dsetfont(contexp->fontname, contexp->fontsize);
   if (RecordGEvents) record_gattr(G_SAVE, -1);
   return(contexp);
}

FRAME *setstatus(FRAME *newframe)
{
  FRAME *f = contexp;
  contexp = newframe;
  return(f);
}

FRAME *setframe(FRAME *newframe)
{
  FRAME *f = contexp;
  contexp = newframe;
  return(f);
}

FRAME *getframe(void)
{
  return(contexp);
}


void pushviewport(void)
{
  float *vw;

/* if haven't created the stack yet, allocate */
  if (!ViewportStack) {
    ViewportStack = (VW_STACK *) calloc(1, sizeof(VW_STACK));
    VW_SIZE(ViewportStack) = 0;
    VW_INDEX(ViewportStack) = -1;
    VW_VIEWPORTS(ViewportStack) = NULL;
    VW_INC(ViewportStack) = 4;
  }

/* if there isn't space to add the current viewport, allocate */  
  if (VW_INDEX(ViewportStack) == (VW_SIZE(ViewportStack)-1)) {
    VW_SIZE(ViewportStack) += VW_INC(ViewportStack);
    if (VW_VIEWPORTS(ViewportStack))
      VW_VIEWPORTS(ViewportStack) = 
	(float *) realloc(VW_VIEWPORTS(ViewportStack), 
			  VW_SIZE(ViewportStack)*4*sizeof(float));
    else VW_VIEWPORTS(ViewportStack) = 
      (float *) calloc(VW_SIZE(ViewportStack)*4, sizeof(float));
  }
  VW_INDEX(ViewportStack)++;
  
/* set vw to the top of the stack */
  vw = &VW_VIEWPORTS(ViewportStack)[4*VW_INDEX(ViewportStack)];
  
  getviewport(&vw[0], &vw[1], &vw[2], &vw[3]);
}

void poppushviewport(void)
{
  popviewport();
  pushviewport();
}
    
int popviewport(void)
{
  float *vw;
  if (!ViewportStack || VW_INDEX(ViewportStack) < 0) {
    return(0);
  }
  vw = &VW_VIEWPORTS(ViewportStack)[4*VW_INDEX(ViewportStack)];
  setviewport(vw[0], vw[1], vw[2], vw[3]);
  VW_INDEX(ViewportStack)--;
  return(1);
}


void beginframe(void)
{
   if (bframe)
      (*bframe)();
}

void endframe(void)
{
   if (eframe)
      (*eframe)();
}

void clearscreen()
{
  if (contexp->dclearfunc)
  	contexp->dclearfunc();
  else if (bframe) (*bframe)();

  if (RecordGEvents) gbResetGevents();
}

void noplot(void)
{
}


/* sets the ``clearfunc'' handler function */

HANDLER setclearfunc(HANDLER hnew)
{
	HANDLER hold = contexp->dclearfunc ;

	contexp->dclearfunc = hnew ;
	return(hold) ;
}

PHANDLER getpoint(void)
{
	return (contexp->dpoint);
}

PHANDLER setpoint(PHANDLER pointp)
{
   PHANDLER oldfunc = contexp->dpoint;
   contexp->dpoint = pointp;
   return(oldfunc);
}

PHANDLER setclrpnt(PHANDLER clrpntp)
{
   PHANDLER oldfunc = contexp->dclrpnt;
   contexp->dclrpnt = clrpntp;
   return(oldfunc);
}

THANDLER setchar(THANDLER charp)
{
   THANDLER oldfunc = contexp->dchar;
   contexp->dchar = charp;
   if (charp == NULL) setchrsize(6.0, 8.0);    /* for point driven char hand */
   return(oldfunc);
}

THANDLER settext(THANDLER textp)
{
   THANDLER oldfunc = contexp->dtext;
   contexp->dtext = textp;
   return(oldfunc);
}

LHANDLER setline(LHANDLER linep)
{
   LHANDLER oldfunc = contexp->dline;
   contexp->dline = linep;
   return(oldfunc);
}

FHANDLER setfilledpoly(FHANDLER fillp)
{
   FHANDLER oldfunc = contexp->dfilledpoly;
   contexp->dfilledpoly = fillp;
   return(oldfunc);
}

FHANDLER setpolyline(FHANDLER polyl)
{
   FHANDLER oldfunc = contexp->dpolyline;
   contexp->dpolyline = polyl;
   return(oldfunc);
}

LHANDLER setclipfunc(LHANDLER clipf)
{
   LHANDLER oldfunc = contexp->dclip;
   contexp->dclip = clipf;
   return(oldfunc);
}

CHANDLER setcircfunc(CHANDLER circfunc)
{
  CHANDLER oldfunc = contexp->dcircfunc;
  contexp->dcircfunc = circfunc;
  return(oldfunc);
}


/* sets the ``set line style'' handler function */

LSHANDLER setlstylefunc(LSHANDLER hnew)
{
	LSHANDLER hold = contexp->dlinestyle ;

	contexp->dlinestyle = hnew ;
	return(hold) ;
}


/* sets the ``set line width'' handler function */

LWHANDLER setlwidthfunc(LWHANDLER hnew)
{
	LWHANDLER hold = contexp->dlinewidth ;

	contexp->dlinewidth = hnew ;
	return(hold) ;
}


/* sets the ``set drawing color'' handler function */

COHANDLER setcolorfunc(COHANDLER hnew)
{
	COHANDLER hold = contexp->dsetcolor ;

	contexp->dsetcolor = hnew ;
	return(hold) ;
}


/* sets the ``set background color'' handler function */

COHANDLER setbgfunc(COHANDLER hnew)
{
	COHANDLER hold = contexp->dsetbg ;

	contexp->dsetbg = hnew ;
	return(hold) ;
}


/* sets the ``get string width'' handler function */

SWHANDLER strwidthfunc(SWHANDLER hnew)
{
	SWHANDLER hold = contexp->dstrwidth ;

	contexp->dstrwidth = hnew ;
	return(hold) ;
}

/* sets the ``get string height'' handler function */

SHHANDLER strheightfunc(SHHANDLER hnew)
{
	SHHANDLER hold = contexp->dstrheight ;

	contexp->dstrheight = hnew ;
	return(hold) ;
}


/* sets the ``set text orientation'' handler function */

SOHANDLER setorientfunc(SOHANDLER hnew)
{
	SOHANDLER hold = contexp->dsetorient ;

	contexp->dsetorient = hnew ;
	return(hold) ;
}


/* sets the ``set text font'' handler function */

SFHANDLER setfontfunc(SFHANDLER hnew)
{
	SFHANDLER hold = contexp->dsetfont ;

	contexp->dsetfont = hnew ;
	return(hold) ;
}

/* sets the ``file based image drawing'' handler function */

IMHANDLER setimagefunc(IMHANDLER hnew)
{
  IMHANDLER hold = contexp->dimage;
  contexp->dimage = hnew;
  return(hold);
}

/* sets the ``in memory image drawing'' handler function */

MIMHANDLER setmemimagefunc(MIMHANDLER hnew)
{
  MIMHANDLER hold = contexp->dmimage;
  contexp->dmimage = hnew;
  return(hold);
}


char *setfont(char *fontname, float size)
{
  static char oldfontname[64];
  if (contexp->fontname && contexp->fontname != oldfontname) 
    strncpy(oldfontname,contexp->fontname,63);
  else strcpy(oldfontname,"");
  
  if (contexp->fontname) free((void *) contexp->fontname);
  contexp->fontname = (char *) malloc(strlen(fontname)+1);
  strcpy(contexp->fontname, fontname);
  contexp->fontsize = size;
  if (contexp->dsetfont)
  	contexp->dsetfont(contexp->fontname, size);
  if (RecordGEvents) record_gtext(G_FONT, size, 0.0, fontname);
  
  if (strcmp(oldfontname,"")) return(oldfontname);
  else return (NULL);
}


/*
 * setsfont - set scaled font based on current viewport
 */

float setsfont(char *fontname, float ssize)
{
  float size = ssize * ((contexp->xr-contexp->xl)/contexp->xsres);
  setfont(fontname, size);
  return(size);
}


float getxscale(void)
{
  return((contexp->xr-contexp->xl)/contexp->xsres);
}

float getyscale(void)
{
  return((contexp->yt-contexp->yb)/contexp->ysres);
}

char *getfontname()
{
   return(contexp->fontname);
}

float getfontsize()
{
   return(contexp->fontsize);
}

void setbframe(HANDLER clearfunc)
{
   bframe = clearfunc;
}

void seteframe(HANDLER eoffunc)
{
   eframe = eoffunc;
}

void group(void)
{
   if (RecordGEvents) record_gattr(G_GROUP, 1); 
}

void ungroup(void)
{
  if (RecordGEvents) record_gattr(G_GROUP, 0); 
}

int setimgpreview(int val)
{
  int old = img_preview;
  img_preview = val;
  return old;
}

void postscript(char *filename, float xsize, float ysize)
{
  FRAME *f = contexp;
  int recording, oldmode, oldclip;
  float x = f->xpos;
  float y = f->ypos;
  float xs = xsize;
  float ys = ysize;

  if((oldmode = f->mode))
    SCALEXY(f,xs,ys);
  
  oldclip = setclip(0);
  if ((recording = RecordGEvents)) {
    record_gpoint(G_MOVETO, f->xpos, f->ypos);
    record_gtext(G_POSTSCRIPT, xs, ys, filename); 
  }
  if (!img_preview || !f->dimage) { /* Put a frame where image will appear */
  keyline:
    if (recording) gbDisableGevents();
    screen();
    rect(x, y, x+xs, y+ys);
    moveto(x, y);
    lineto(x+xs, y+ys);
    moveto(x+xs, y);
    lineto(x, y+ys);
    if (oldmode) user();
    if (recording) gbEnableGevents();
    setclip(oldclip);
    return;
  }
  else {
    if (!(f->dimage(x, y, x+xs, y+ys, filename))) 
      goto keyline;
    setclip(oldclip);
  }
}

int place_image(int w, int h, int d, unsigned char *data, 
		 float xsize, float ysize)
{
  FRAME *f = contexp;
  int recording, oldmode, ref = -1;
  int oldclip;
  float x = f->xpos;
  float y = f->ypos;
  float xs = xsize;
  float ys = ysize;

  if((oldmode = f->mode))
    SCALEXY(f,xs,ys);
  
  oldclip = setclip(0);

  if ((recording = RecordGEvents)) {
    ref = gbAddImage(w, h, d, data, x, y, x+xs, y+ys);
    record_gpoint(G_MOVETO, f->xpos, f->ypos);
    record_gline(G_IMAGE, xs, ys, (float) ref, 0);
  }

  if (!img_preview || !f->dmimage) { /* Put a frame where image will appear */
  keyline:
    if (recording) gbDisableGevents();
    screen();
    rect(x, y, x+xs, y+ys);
    moveto(x, y);
    lineto(x+xs, y+ys);
    moveto(x+xs, y);
    lineto(x, y+ys);
    if (oldmode) user();
    if (recording) gbEnableGevents();
    setclip(oldclip);
    return 0;
  }
  else {
    if (!(f->dmimage(x, y, x+xs, y+ys, w, h, d, data))) 
      goto keyline;
  }
  setclip(oldclip);
  return ref;
}

int replace_image(int ref, int w, int h, int d, unsigned char *data)
{
  FRAME *f = contexp;
  GBUF_IMAGE *gimg;
  gimg = gbFindImage(ref);
  if (!gimg) return 0;
  gbReplaceImage(ref, w, h, d, data);
  if (f->dmimage) {
    f->dmimage(gimg->x0, gimg->y0, gimg->x1, gimg->y1, w, h, d, data);
  }
  return 1;
}

int setcolor(int color)
{
   int oldcolor = contexp->color;
   if (RecordGEvents) record_gattr(G_COLOR, color); 
   contexp->color = color;
   if (contexp->dsetcolor)
   	contexp->dsetcolor(color);
   return(oldcolor);
}

int getcolor()
{
   return(contexp->color);
}

int setbackgroundcolor(int color)
{
   int oldcolor = contexp->background_color;
   contexp->background_color = color;
   if (contexp->dsetbg)
   	contexp->dsetbg(color) ;
   return(oldcolor);
}

int getbackgroundcolor()
{
  return(contexp->background_color);
}


int setuser(int modearg)
{
   int olduser = contexp->mode;
   contexp->mode = modearg;
   return(olduser);
}

int setclip(int cliparg)
{
  FRAME *f = contexp;
  LHANDLER dclip = f->dclip;
  int oldclip = f->clipf;
  if (oldclip == cliparg) return oldclip;

  if (cliparg) {
    if (dclip) (*dclip)(f->xl, f->yb, f->xr, f->yt);
    if (RecordGEvents)
      record_gline(G_CLIP, f->xl, f->yb, f->xr, f->yt);
  }
  else {
    if (dclip) (*dclip)(0, 0, f->xsres, f->ysres);
    if (RecordGEvents)
      record_gline(G_CLIP, 0, 0, f->xsres, f->ysres);
  }
  f->clipf = cliparg;
  return oldclip;
}

int getclip(void)
{
  FRAME *f = contexp;
  return f->clipf;
}

void setchrsize(float chrwarg, float chrharg)
{
   FRAME *f = contexp;
   
   if(chrharg < 9.0) f->linsiz = 8.0;
   else f->linsiz = chrharg;
   f->yinc = 1.0;
   
   if(chrwarg < 7.0) f->colsiz = 7.0;
   else f->colsiz = chrwarg;
   f->xinc = 1.0;
}

static void setvw(void)
{
   FRAME *f = contexp;
   f->xus = f->xur - f->xul;
   f->yus = f->yut - f->yub;
   f->xs = f->xr - f->xl;
   f->ys = f->yt - f->yb;
}

void setclipregion(float xlarg, float ybarg, float xrarg, float ytarg)
{
  FRAME *f = contexp;

  if (f->mode) WINDOW(f, xlarg, ybarg);
  if (f->mode) WINDOW(f, xrarg, ytarg);

  if (f->dclip) (*(f->dclip))(xlarg, ybarg, xrarg, ytarg);
  if (RecordGEvents)
    record_gline(G_CLIP, xlarg, ybarg, xrarg, ytarg);
}

void setviewport(float xlarg,float ybarg,float xrarg,float ytarg)
{
	FRAME *f = contexp;
	LHANDLER dclip = f->dclip;

	CHECKMIN(xlarg);        /* check for lower than minimum values */
	CHECKMIN(xrarg);
	CHECKMIN(ybarg);
	CHECKMIN(ytarg);

	CHECKXMX(f,xlarg);      /* check for greater than maximum values */
	CHECKXMX(f,xrarg);
	CHECKYMX(f,ybarg);
	CHECKYMX(f,ytarg);

	if (xlarg>xrarg)        /* swap x-arguments if needed */
	   SWAP(xlarg,xrarg);
	if (ybarg>ytarg)        /* swap y-arguments if needed */
	   SWAP(ybarg,ytarg);

	if (xrarg - xlarg < 1.0) xrarg += 1.0;
	if (ytarg - ybarg < 1.0) ytarg += 1.0;

	f->xl = xlarg;
	f->yb = ybarg;
	f->xr = xrarg;
	f->yt = ytarg;

	if (dclip) (*dclip)(xlarg, ybarg, xrarg, ytarg);
	if (RecordGEvents)
	   record_gline(G_CLIP, xlarg, ybarg, xrarg, ytarg);

	setvw();
}

void setpviewport(float xlarg,float ybarg,float xrarg,float ytarg)
{
  FRAME *f = contexp;
  float x0, x1, y0, y1;
  
  x0 = f->xl+xlarg*f->xs;
  x1 = f->xl+xrarg*f->xs;
  y0 = f->yb+ybarg*f->ys;
  y1 = f->yb+ytarg*f->ys;
  setviewport(x0, y0, x1, y1);
}

float getuaspect()
{
    float wlx, wly, wux, wuy;
    float vlx, vly, vux, vuy;

    /* Get viewport/window extents to calc user coord aspect ratio */
    getviewport(&vlx, &vly, &vux, &vuy);
    getwindow(&wlx, &wly, &wux, &wuy);

    /* Make upper bounds "correct" */
    vux+=1.0;
    vuy+=1.0;

    return (((vux-vlx)/(vuy-vly))*((wuy-wly)/(wux-wlx)));
}

void getviewport(float *xlarg, float *ybarg, float *xrarg, float *ytarg)
{
  FRAME *f = contexp;
  *xlarg = f->xl;
  *ybarg = f->yb;
  *xrarg = f->xr;
  *ytarg = f->yt;
}

void setfviewport(float fxlarg, float fybarg, float fxrarg, float fytarg)
{
  FRAME *f = contexp;
  setviewport(fxlarg*f->xsres, fybarg*f->ysres,
	      fxrarg*f->xsres, fytarg * f->ysres);
}    

void getwindow(float  *xul, float *yub, float *xur, float *yut)
{
  FRAME *f = contexp;
  *xul = f->xul;
  *yub = f->yub;
  *xur = f->xur;
  *yut = f->yut;
}

void setwindow(float xularg, float yubarg, float xurarg, float yutarg)
{
  FRAME *f = contexp;
  f->xul = xularg;
  f->yub = yubarg;
  f->xur = xurarg;
  f->yut = yutarg;
  setvw();
}

int setlstyle(int grainarg)
{
   FRAME *f = contexp;
   int oldlstyle = f->grain;
   if(grainarg <= 0) grainarg = 1;
   f->grain = grainarg;
   if (contexp->dlinestyle)
   	(*contexp->dlinestyle)(grainarg);
   if (RecordGEvents) record_gattr(G_LSTYLE, grainarg);
   return(oldlstyle);
}

int setlwidth(int width)
{
   FRAME *f = contexp;
   int oldwidth = f->lwidth;
   if (width < 0) width = 1;
   f->lwidth = width;
   if (contexp->dlinewidth)
   	(*contexp->dlinewidth)(width);
   if (RecordGEvents) record_gattr(G_LWIDTH, width);
   return(oldwidth);
}

int setgrain(int grainarg)		/* for historical reasons... */
{
   return(setlstyle(grainarg));
}

int setorientation(int path)
{
  int oldorient = contexp->orientation;
  
  if (RecordGEvents) record_gattr(G_ORIENTATION, path);
  contexp->orientation = path;
  if (contexp->dsetorient)
  	contexp->dsetorient(path);
  return(oldorient);
}


/* returns the current text orientation */

int getorientation(void)
{
	return(contexp->orientation) ;
}


int setjust(int just)
{
  int oldjust = contexp->just;

  if (RecordGEvents) record_gattr(G_JUSTIFICATION, just);
   contexp->just = just;
  return(oldjust);
}

void setresol(float xres, float yres)
{
   FRAME *f = contexp;
   f->xsres = xres;                             /* set new x-resolution */
   f->ysres = yres;                             /* set new y-resolution */
}


void getresol(float *xres, float *yres)
{
   FRAME *f = contexp;
   *xres = f->xsres;                             /* get x-resolution */
   *yres = f->ysres;                             /* get y-resolution */
}


static int GetNorm (float *x, float *y)
{
	FRAME *f = contexp;
	float argx = *x;
	float argy = *y;

	if(f->mode)
		WINDOW(f,argx,argy);
	f->xpos = argx;
	f->ypos = argy;

	if(f->clipf){
		if(code(f,argx,argy))
			return (0);
	}
	*x = argx;
	*y = argy;
	return (1);
}

/**************************************************************************/
/* Routines for drawing points, lines, shapes, etc.                       */
/**************************************************************************/

void dotat(float xarg, float yarg)
{
	FRAME *f = contexp;
	PHANDLER dp = f->dpoint;

	if(f->mode)
		WINDOW(f,xarg,yarg);
	f->xpos = xarg;
	f->ypos = yarg;
	if (dp) (*dp)(xarg,yarg);
	if (RecordGEvents)
	   record_gpoint(G_POINT, xarg, yarg);
}

void BigDotAt(float x, float y)                 /* make a big dot         */
{
	FRAME *f = contexp;
	PHANDLER pfunc;

	pfunc = getpoint();

	if (!GetNorm(&x, &y)) return;

	(*pfunc)(x, y);
	(*pfunc)((x+=XUNIT(f)), y);
	(*pfunc)(x, (y+=YUNIT(f)));
	(*pfunc)((x-=XUNIT(f)), y);
	if (RecordGEvents)
	   record_gpoint(G_POINT, x, y);
}

void SmallSquareAt(float x, float y)             /* make a filled square */
{
   FRAME *f = contexp;
   PHANDLER pfunc;
   register float i, j;

   pfunc = getpoint();

   if (!GetNorm(&x, &y)) return;
   
   for (j=y-2.0*YUNIT(f); j<y+2.0*YUNIT(f); j+=YUNIT(f))
	  for (i=x-2.0*XUNIT(f); i<x+2.0*XUNIT(f); i+=XUNIT(f))
      (*pfunc)(i,j);
}

void SquareAt(float x, float y)              /* make a filled square */
{
   FRAME *f = contexp;
   filledrect(x-3.0*XUNIT(f),y-3.0*YUNIT(f),
	      x+3.0*XUNIT(f),y+3.0*YUNIT(f));
}


void triangle(float x, float y, float scale) /* make a hollow square */
{
   FRAME *f = contexp;
   float root2 = sqrt(2.);
   float xoff = root2*0.5*scale*XUNIT(f);
   float yoff = root2*0.5*scale*YUNIT(f);
   float yoff2 = .75*yoff;
   moveto(x-xoff,y-yoff2);
   lineto(x+xoff,y-yoff2);
   lineto(x,y+yoff);
   lineto(x-xoff,y-yoff2);
}

void diamond(float x, float y, float scale) /* make a hollow square */
{
   FRAME *f = contexp;
   float xoff = 0.3*scale*XUNIT(f);
   float yoff = 0.5*scale*YUNIT(f);
   moveto(x-xoff,y);
   lineto(x,y+yoff);
   lineto(x+xoff,y);
   lineto(x,y-yoff);
   lineto(x-xoff,y);
}


void square(float x, float y, float scale) /* make a hollow square */
{
   FRAME *f = contexp;
   float xoff = 0.5*scale*XUNIT(f);
   float yoff = 0.5*scale*YUNIT(f);
   moveto(x-xoff,y-yoff);
   lineto(x+xoff,y-yoff);
   lineto(x+xoff,y+yoff);
   lineto(x-xoff,y+yoff);
   lineto(x-xoff,y-yoff);
}

void fsquare(float x, float y, float scale) /* make a filled square */
{
   FRAME *f = contexp;
   filledrect(x-0.5*scale*XUNIT(f),y-0.5*scale*YUNIT(f),
	      x+0.5*scale*XUNIT(f),y+0.5*scale*YUNIT(f));
}

void circle(float xarg, float yarg, float scale)
{
  FRAME *f = contexp;
  CHANDLER circfunc = f->dcircfunc;
  float xsize = scale/XUNIT(f);
  xsize = scale;
  
  if(f->mode) {
    WINDOW(f,xarg,yarg);
  }

  if (f->clipf) {
    f->wx1 = xarg-(xsize/2);
    f->wy1 = yarg-(xsize/2);
    f->wx2 = xarg+(xsize/2);
    f->wy2 = yarg+(xsize/2);
    if (dclip())
      return;
  }

  if (circfunc) {
    (*circfunc)(xarg-(xsize/2), yarg+(xsize/2), xsize, 0);
  }
  
  if (RecordGEvents)
    record_gline(G_CIRCLE, xarg, yarg, xsize, 0.0);
}

void fcircle(float xarg, float yarg, float scale)
{
  FRAME *f = contexp;
  CHANDLER circfunc = f->dcircfunc;
  float xsize = scale/XUNIT(f);
  xsize = scale;

  if(f->mode) {
    WINDOW(f,xarg,yarg);
  }
  
  if (f->clipf) {
    f->wx1 = xarg-(xsize/2);
    f->wy1 = yarg-(xsize/2);
    f->wx2 = xarg+(xsize/2);
    f->wy2 = yarg+(xsize/2);
    if (dclip())
      return;
  }

  if (circfunc) {
    (*circfunc)(xarg-(xsize/2), yarg+(xsize/2), xsize, 1);
  }
  
  if (RecordGEvents)
    record_gline(G_CIRCLE, xarg, yarg, xsize, 1.0);
}

void vtick(float x, float y, float scale) /* make a vertical tick */
{
  FRAME *f = contexp;
  float yoff = scale*YUNIT(f);
  moveto(x, y-yoff/2.0);
  lineto(x, y+yoff/2.0);
}

void htick(float x, float y, float scale) /* make a horiztonal tick */
{
  FRAME *f = contexp;
  float xoff = scale*XUNIT(f);
  moveto(x-xoff/2.0, y);
  lineto(x+xoff/2.0, y);
}

void vtick_up(float x, float y, float scale) /* make a vertical tick */
{
  FRAME *f = contexp;
  float yoff = scale*YUNIT(f);
  moveto(x, y);
  lineto(x, y+yoff/2.0);
}

void vtick_down(float x, float y, float scale) /* make a vertical tick */
{
  FRAME *f = contexp;
  float yoff = scale*YUNIT(f);
  moveto(x, y);
  lineto(x, y-yoff/2.0);
}

void htick_left(float x, float y, float scale) /* make a horiztonal tick */
{
  FRAME *f = contexp;
  float xoff = scale*XUNIT(f);
  moveto(x, y);
  lineto(x-xoff/2.0, y);
}

void htick_right(float x, float y, float scale) /* make a horiztonal tick */
{
  FRAME *f = contexp;
  float xoff = scale*XUNIT(f);
  moveto(x, y);
  lineto(x+xoff/2.0, y);
}

void plus(float x, float y, float scale) /* make a plus sign tick */
{
  htick(x, y, scale);
  vtick(x, y, scale);
}

void TriangleAt(float x, float y)             /* make a filled triangle */
{
   FRAME *f = contexp;
   PHANDLER pfunc;
   float i, j, t;
   
   pfunc = getpoint();
   
   if (GetNorm(&x, &y) == 0.0) return;
   
   for (t=0.0, j=y+3.0*YUNIT(f); j>y-3.0*YUNIT(f); j--, t+=XUNIT(f))
      for (i=x-t; i<x+t; i+=XUNIT(f))
	 (*pfunc)(i,j);
}

float setwidth(float w)
{
	float oldw = barwidth;

	barwidth = w;
	return(oldw);
}

void VbarsAt (float x, float y)              /* draw vertical bars */
{
	PHANDLER pfunc;
	float i, j;

	pfunc = getpoint();

	if (GetNorm(&x, &y) == 0.0) return;

	for (j=contexp->yb; j<y; j++)
	   for (i=x-barwidth/2.0; i<x+barwidth/2.0; i++)
	      (*pfunc)(i,j);
}

void HbarsAt (float x, float y)              /* draw horizontal bars */
{
   PHANDLER pfunc;
   float i, j;
   
   pfunc = getpoint();
   
   if (GetNorm(&x, &y) == 0.0) return;
   
   for (j=y+barwidth/2.0; j>y-barwidth/2.0; j--)
      for (i=contexp->xl; i<x; i++)
	 (*pfunc)(i,j);
}

int code(FRAME *f, float x, float y)
{
   register int c;
   c = 0;
   if(x < f->xl) c |= 1;
   if(y < f->yb) c |= 4;
   if(x > f->xr) c |= 2;
   if(y > f->yt) c |= 8;
   return(c);
}

void moveto(float xarg, float yarg)
{
   FRAME *f = contexp;
   
   if(f->mode)
      WINDOW(f,xarg,yarg);
   f->xpos = xarg;
   f->ypos = yarg;
   if (RecordGEvents) record_gpoint(G_MOVETO, f->xpos, f->ypos);
}

void moverel(float dxarg, float dyarg)
{
   FRAME *f = contexp;
   
   if(f->mode)
      SCALEXY(f,dxarg,dyarg);
   f->xpos += dxarg;
   f->ypos += dyarg;
   if (RecordGEvents) record_gpoint(G_MOVETO, f->xpos, f->ypos);
}

static int dclip(void)
{
   FRAME *f = contexp;
   float x1 = f->wx1;
   float y1 = f->wy1;
   float x2 = f->wx2;
   float y2 = f->wy2;
   int c1,c2;
   
   c2 = code(f,x2,y2);
   for(;;){
      c1 = code(f,x1,y1);
      if((c1 == 0) && (c2 == 0))
      {
	 f->wx1 = x1;
	 f->wy1 = y1;
	 f->wx2 = x2;
	 f->wy2 = y2;
	 f->c1 = c1;
	 f->c2 = c2;
	 return(0);
      }
      if(c1 & c2)
	 return(-1);
      if(!c1){
	 SWAP(c1,c2);
	 SWAP(x1,x2);
	 SWAP(y1,y2);
      }
      if(1 & c1){
	 y1 += MULDIV(y2-y1,f->xl-x1,x2-x1);
	 x1 = f->xl;
	 continue;
      }
      if(2 & c1){
	 y1 += MULDIV(y2-y1,f->xr-x1,x2-x1);
	 x1 = f->xr;
	 continue;
      }
      if(4 & c1){
	 x1 += MULDIV(x2-x1,f->yb-y1,y2-y1);
	 y1 = f->yb;
	 continue;
      }
      if(8 & c1){
	 x1 += MULDIV(x2-x1,f->yt-y1,y2-y1);
	 y1 = f->yt;
	 continue;
      }
   }
}

/*
 * Similar to dclip(), except for filled polygons, so linear
 * interpolation (MULDIV stuff) is not necessary 
 */

static int fclip(void)
{
   FRAME *f = contexp;
   float x1 = f->wx1;
   float y1 = f->wy1;
   float x2 = f->wx2;
   float y2 = f->wy2;
   int c1,c2;
   
   c2 = code(f,x2,y2);
   for(;;){
      c1 = code(f,x1,y1);
      if((c1 == 0) && (c2 == 0))
      {
	 f->wx1 = x1;
	 f->wy1 = y1;
	 f->wx2 = x2;
	 f->wy2 = y2;
	 f->c1 = c1;
	 f->c2 = c2;
	 return(0);
      }
      if(c1 & c2)
	 return(-1);
      if(!c1){
	 SWAP(c1,c2);
	 SWAP(x1,x2);
	 SWAP(y1,y2);
      }
      if(1 & c1){
	 x1 = f->xl;
	 continue;
      }
      if(2 & c1){
	 x1 = f->xr;
	 continue;
      }
      if(4 & c1){
	 y1 = f->yb;
	 continue;
      }
      if(8 & c1){
	 y1 = f->yt;
	 continue;
      }
   }
}

static void linutl(float x, float y)
{
   FRAME *f = contexp;
   PHANDLER dp = f->dpoint;
   LHANDLER dl = f->dline;
   float xx, yy, error;
   float index, incx, incy, step, startx, starty, endx, endy;
   
   if (!dp) return ;
   endx = xx = x;
   endy = yy = y;
   startx = x = f->xpos;
   starty = y = f->ypos;
   f->xpos = xx;
   f->ypos = yy;
   if (f->clipf) {
      f->wx1 = x;
      f->wy1 = y;
      f->wx2 = xx;
      f->wy2 = yy;
      if (dclip()) {
	/* 
	 * Even if the entire line is clipped, we need to moveto the
	 * destination for following lineto's...
	 */
	if (RecordGEvents) {
	  record_gpoint(G_MOVETO, f->xpos, f->ypos);
	}
	return;
      }
      else {
	 x = f->wx1;
	 y = f->wy1;
	 xx = f->wx2;
	 yy = f->wy2;
      }
   }
   if (dl){
      (*dl)(xx, yy, x, y);
      if (RecordGEvents) {

	 /*******************************************************************
	  *  Recording the lineto as a "lineto" requires that the correct
	  *  x,y pair be used.  Unfortunately, if clipping occurs, the
	  *  start and finish x,y pairs can be swapped in the computation.
	  *  For real lines this isn't a problem, because there's no
	  *  detectable difference between the "start" and "end" of a line. 
	  *  For lineto's, however, the line must continue to the new, possibly
	  *  clipped point, so a small calculation is performed to figure
	  *  out which pair of points is the actual "lineto" pair.  
	  *  We just figure out which pair is farther from the start pair
	  *  and use that as the new lineto pair.
	  ********************************************************************/
	 
 	 if (((startx-x)*(startx-x))+((starty-y)*(starty-y)) >
	     ((startx-xx)*(startx-xx))+((starty-yy)*(starty-yy))) {
	    SWAP(x, xx);
	    SWAP(y, yy);
	 }
	 /*
	  * If the starting coord was clipped, then we need a moveto
	  */
	 if (startx != x || starty != y) record_gpoint(G_MOVETO, x, y);
	 record_gpoint(G_LINETO, xx, yy);
	 if (xx != endx || yy != endy) record_gpoint(G_MOVETO, endx, endy);
      }
      return;
   }
   step = f->grain;
   incx = XUNIT(f);
   if ((xx -= x) < 0) {
      incx = -incx;
      xx = -xx;
   }
   incy = YUNIT(f);
   if ((yy -= y) < 0){
      incy = -incy;
      yy = -yy;
   }
   if (xx > yy){
      error = xx;
      error = error / 2.0;
      index = xx + XUNIT(f);
      for (;;) {
	 if (dp) (*dp)(x,y);
	 while (step-- > 0.0) {
	    error += yy;
	    if (error > xx){
	       error -= xx;
	       y += incy;
	    }
	    x += incx;
	    if(--index == 0.0)
	       return;
	 }
	 step = f->grain;
      }
   }
   else {
      error = yy / 2.0;
      index = yy + YUNIT(f);
      for (;;) {
	 if (dp) (*dp)(x,y);
	 while (step-- > 0.0){
	    error += xx;
	    if (error > yy) {
	       error -= yy;
	       x += incx;
	    }
	    y += incy;
	    if (--index <= 0.0)
	       return;
	 }
	 step = f->grain;
      }
   }
}

void linerel(float dxarg, float dyarg)
{
   FRAME *f = contexp;
   
   if (f->mode)
      SCALEXY(f, dxarg, dyarg);
   linutl(dxarg + f->xpos, dyarg + f->ypos);
}

void lineto(float xarg, float yarg)
{
   FRAME *f = contexp;
	
   if (f->mode)
      WINDOW(f, xarg, yarg);
   linutl(xarg, yarg);
}

void polyline(int nverts, float *verts)
{
  register int i;
  FRAME *f = contexp;
  FHANDLER polylinefunc = f->dpolyline;
  
  /*
    This is not very good, but we're still battling with
     the clipping problem.  The basic issue is whether
     to record the line a G_POLY or as a series of G_LINETOs
  */
  if (f->dpolyline) {
    if (f->mode) {
      for(i = 0;i < nverts; i++)
	WINDOW(f, verts[2*i], verts[2*i+1]);
    }
  
    if (polylinefunc) {
      (*polylinefunc)(&verts[0], nverts);
    }
    if (RecordGEvents) record_gpoly(G_POLY, nverts*2, verts);
  }
  else {
    int i, j;
    moveto(verts[0], verts[1]);
    for (i = 1, j = 2; i < nverts; i++, j+=2) {
      lineto(verts[j], verts[j+1]);
    }
  }
}

void filledpoly(int nverts, float *verts)
{
  register int i;
  FRAME *f = contexp;
  FHANDLER fillfunc = f->dfilledpoly;
  
  
  if (f->mode) {
    for(i = 0;i < nverts; i++)
      WINDOW(f, verts[2*i], verts[2*i+1]);
  }
  
  if (fillfunc) {
    (*fillfunc)(&verts[0], nverts);
  }

  if (RecordGEvents) record_gpoly(G_FILLEDPOLY, nverts*2, verts);
}

void filledrect(float x1, float y1, float x2, float y2)
{
  FRAME *f = contexp;
  FHANDLER fillfunc = f->dfilledpoly;
  float verts[8];

  if (f->mode) {
    WINDOW(f, x1, y1);
    WINDOW(f, x2, y2);
  }
  
  /* NOTE:  This is preliminary clipping code!  */
  if (f->clipf) {
    f->wx1 = x1;
    f->wy1 = y1;
    f->wx2 = x2;
    f->wy2 = y2;
    if (fclip())
      return;
  }

  if (fillfunc) {
    verts[0] = f->wx1;   verts[1] = f->wy1;
    verts[2] = f->wx2;   verts[3] = f->wy1;
    verts[4] = f->wx2;   verts[5] = f->wy2;
    verts[6] = f->wx1;   verts[7] = f->wy2;

    (*fillfunc)(&verts[0], 4);
  }

  if (RecordGEvents) record_gline(G_FILLEDRECT, f->wx1, f->wy1, f->wx2, f->wy2);
}

/* make a hollow rectangle */
void rect(float x1, float y1, float x2, float y2) 
{
  float verts[10];
  verts[0] = x1; verts[1] = y1;
  verts[2] = x1; verts[3] = y2;
  verts[4] = x2; verts[5] = y2;
  verts[6] = x2; verts[7] = y1;
  verts[8] = x1; verts[9] = y1;
  polyline(5, verts);
}

void cleararea(float x1, float y1, float x2, float y2)
{
  int oldcolor;
  
  oldcolor = setcolor(getbackgroundcolor());
  filledrect(x1, y1, x2, y2);
  setcolor(oldcolor);
}

void clearline(float x1, float y1, float x2, float y2)
{
  int oldcolor;
  
  oldcolor = setcolor(getbackgroundcolor());
  moveto(x1, y1);
  lineto(x2, y2);
  setcolor(oldcolor);
}


void drawtextf(char *str, ...)
{
  static char msg[1024];
  va_list arglist;
  
  va_start(arglist, str);
  vsprintf(msg, str, arglist);
  va_end(arglist);
  
  drawtext(msg);
}

 
void cleartextf(char *str, ...)
{
  int oldcolor = setcolor(getbackgroundcolor());
  static char msg[1024];
  va_list arglist;

  va_start(arglist, str);
  vsprintf(msg, str, arglist);
  va_end(arglist);
  drawtext(msg);
  setcolor(oldcolor);
}



int strwidth(char *str)
{
   if (contexp->dchar && contexp->dstrwidth)
   	return((*contexp->dstrwidth)(str));
   else return(strlen(str)*contexp->colsiz);
}

int strheight(char *str)
{
   if (contexp->dchar && contexp->dstrheight)
   	return((*contexp->dstrheight)(str));
   else return(contexp->linsiz);
}

void cleartext(char *str)
{
  int oldcolor = setcolor(getbackgroundcolor());
  drawtext(str);
  setcolor(oldcolor);
}


void drawtext(char *string)
{
  FRAME *f = contexp;
  int c, add_newline = 0;
  THANDLER dc = f->dchar;
  THANDLER dt = f->dtext;
  float xoffset = 0.0, yoffset = 0.0;

  if (strstr(string, "\n\r")) {
     string[strlen(string)-2] = 0;
     add_newline = 1;
  }

  else if (strstr(string, "\n")) {
     string[strlen(string)-1] = 0;
     add_newline = 1;
  }
  
  if (RecordGEvents) record_gtext(G_TEXT, f->xpos, f->ypos, string);

  /*
   * now figure where text should begin based on current orientation
   * and justification
   */

  if (dt) {
    (*dt)(f->xpos,f->ypos,string);
    return;
  }
  
  switch (f->orientation) {
     case TXT_HORIZONTAL:
	switch(f->just) {
	   case LEFT_JUST:
	      yoffset -= strheight(string)/2.0;
	      break;
	   case RIGHT_JUST:
	      xoffset -= strwidth(string);
	      yoffset -= strheight(string)/2.0;
	      break;
	   case CENTER_JUST:
	      xoffset -= strwidth(string) / 2.0;
	      yoffset -= strheight(string)/2.0;
	      break;
	}
	break;
     case TXT_VERTICAL:
	 switch(f->just) {
	    case CENTER_JUST:
	       /*
		* presumably, the char handler can write rotated text so
		* we move DOWN to offset the text
		*/
	       if (dc) {
		  xoffset -= strheight(string) / 2.0;
		  yoffset -= strwidth(string) / 2.0;
	       }
	       /*
	        * the char handler using the point hander, however, must 
		* back up, so we move UP half of the width of the string
		*/
	       else {
		  xoffset -= contexp->colsiz / 2.0;
		  yoffset += (contexp->linsiz*strlen(string)) / 2.0;
	       }
	       break;
	    case LEFT_JUST:
	       if (dc) {
		  xoffset -= strheight(string) / 2.0;
	       }
	       else {
		  yoffset += contexp->linsiz*strlen(string);
		  xoffset -= contexp->colsiz / 2.0;
	       }
	       break;
	    case RIGHT_JUST:
	       if (dc) {
		  xoffset -= strheight(string) / 2.0;
		  yoffset -= strwidth(string);
	       }
	       else {
		  xoffset -= contexp->colsiz / 2.0;
	       }
	       break;
	 }
	 break;
	    case 2:
	    case 3:
	       break;
  }

  /* based on new position, now call drawing routines */

  if (dc) {
     (*dc)(f->xpos+xoffset,f->ypos+yoffset,string);
     if (add_newline) NXTLIN(f);
  }

  else {
     moverel(xoffset, yoffset);
     switch (f->orientation) {
	case TXT_HORIZONTAL:
	   while((c = *string++)) drawchar(c);
	   if (add_newline) {
	      NXTLIN(f); LEFTMARG(f);
	   }
	   break;
	case TXT_VERTICAL:
	   while((c = *string++)) {
	      drawchar(c);
	      moverel(-(f->colsiz), -(f->linsiz));
	   }
     }
  }
}

static char chartab[90][5] = {
	{0000,0000,0276,0000,0000},	/* ! */
	{0000,0016,0000,0016,0000},	/* " */
	{0050,0356,0000,0356,0050},	/* # */
	{0110,0124,0326,0124,0044},	/* $ */
	{0306,0046,0026,0310,0306},	/* % */
	{0154,0222,0254,0100,0240},	/* & */
	{0000,0000,0016,0016,0000},	/* ' */
	{0000,0070,0104,0202,0000},	/* ( */
	{0000,0202,0104,0070,0000},	/* ) */
	{0114,0070,0174,0070,0114},	/* * */
	{0020,0020,0376,0020,0020},	/* + */
	{0000,0000,0260,0160,0000},	/* , */
	{0040,0040,0040,0040,0040},	/* - */
	{0000,0000,0300,0300,0000},	/* . */
	{0100,0040,0020,0010,0004},	/* / */
	{0000,0174,0202,0202,0174},	/* 0 */
	{0000,0204,0376,0200,0000},	/* 1 */
	{0344,0222,0222,0222,0214},	/* 2 */
	{0104,0202,0222,0222,0154},	/* 3 */
	{0060,0050,0044,0376,0040},	/* 4 */
	{0116,0212,0212,0212,0162},	/* 5 */
	{0170,0224,0222,0222,0140},	/* 6 */
	{0002,0342,0022,0012,0006},	/* 7 */
	{0154,0222,0222,0222,0154},	/* 8 */
	{0014,0222,0222,0122,0074},	/* 9 */
	{0000,0000,0154,0154,0000},	/* : */
	{0000,0200,0166,0066,0000},	/* ; */
	{0020,0050,0104,0202,0000},	/* < */
	{0050,0050,0050,0050,0050},	/* = */
	{0000,0202,0104,0050,0020},	/* > */
	{0000,0004,0242,0022,0014},	/* ? */
	{0144,0222,0362,0202,0174},	/* @ */
	{0370,0044,0042,0044,0370},	/* A */
	{0376,0222,0222,0222,0154},	/* B */
	{0174,0202,0202,0202,0104},	/* C */
	{0202,0376,0202,0202,0174},	/* D */
	{0376,0222,0222,0202,0202},	/* E */
	{0376,0022,0022,0002,0002},	/* F */
	{0174,0202,0202,0222,0362},	/* G */
	{0376,0020,0020,0020,0376},	/* H */
	{0000,0202,0376,0202,0000},	/* I */
	{0100,0200,0200,0200,0176},	/* J */
	{0376,0020,0050,0104,0202},	/* K */
	{0376,0200,0200,0200,0200},	/* L */
	{0376,0004,0030,0004,0376},	/* M */
	{0376,0004,0010,0020,0376},	/* N */
	{0376,0202,0202,0202,0376},	/* O */
	{0376,0022,0022,0022,0014},	/* P */
	{0174,0202,0242,0102,0274},	/* Q */
	{0376,0022,0062,0122,0214},	/* R */
	{0104,0212,0222,0242,0104},	/* S */
	{0002,0002,0376,0002,0002},	/* T */
	{0176,0200,0200,0200,0176},	/* U */
	{0016,0060,0300,0060,0016},	/* V */
	{0376,0100,0040,0100,0376},	/* W */
	{0306,0050,0020,0050,0306},	/* X */
	{0006,0010,0360,0010,0006},	/* Y */
	{0302,0242,0222,0212,0206},     /* Z */
	{0000,0376,0202,0202,0000},	/* [ */
	{0002,0004,0010,0020,0040},	/* \ */
	{0000,0202,0202,0376,0000},	/* ] */
	{0010,0004,0376,0004,0010},	/* ^ */
	{0020,0070,0124,0020,0020},	/* _ */
	{0000,0000,0000,0000,0000},	/*  */
	{0100,0250,0250,0250,0360},	/* a */
	{0366,0210,0210,0210,0160},	/* b */
	{0160,0210,0210,0210,0020},	/* c */
	{0160,0210,0210,0210,0366},	/* d */
	{0160,0250,0250,0250,0060},	/* e */
	{0010,0374,0012,0002,0004},	/* f */
	{0220,0250,0250,0360,0010},	/* g */
	{0366,0010,0010,0010,0360},	/* h */
	{0000,0210,0372,0200,0000},	/* i */
	{0000,0140,0200,0232,0140},	/* j */
	{0366,0050,0150,0250,0220},	/* k */
	{0000,0202,0376,0200,0000},	/* l */
	{0370,0010,0360,0010,0360},	/* m */
	{0370,0020,0010,0010,0360},	/* n */
	{0160,0210,0210,0210,0160},	/* o */
	{0370,0050,0050,0050,0020},	/* p */
	{0060,0110,0110,0310,0260},	/* q */
	{0370,0020,0010,0010,0020},	/* r */
	{0020,0250,0250,0250,0100},	/* s */
	{0000,0010,0176,0210,0100},	/* t */
	{0170,0200,0200,0100,0370},	/* u */
	{0070,0100,0200,0100,0070},	/* v */
	{0170,0200,0160,0200,0170},	/* w */
	{0210,0120,0040,0120,0210},	/* x */
	{0030,0240,0240,0240,0170},	/* y */
	{0210,0310,0250,0230,0210}	/* z */
	};


void drawchar(int c)
{
   FRAME *f = contexp;
   PHANDLER dcp = f->dclrpnt;
   PHANDLER dp = f->dpoint;
   THANDLER dc = f->dchar;
   if(f->clipf)
   {
      float x, y;
      x = f->xpos;
      y = f->ypos;
      if(code(f,x,y))
      {
	 NXTCOL(f);
	 return;
      }
      x += f->colsiz;
      y += f->linsiz;
      if(code(f,x,y))
      {
	 NXTCOL(f);
	 return;
      }
   }
   c &= 0377;
   
   if(c == 040 && !dcp){
      NXTCOL(f);
      return;
   }
   
   if(c < 040){
      switch(c){
	 case CR:        LEFTMARG(f);
			 return;
	 case LF:        NXTLIN(f);
			 return;
	 case TAB:       NXTCOL(f);
			 return;
	 case FF:        HOME(f);
			 return;
	 default:        NXTCOL(f);
			 return;
      }
   }
   if(dc)
   {
      char text[2];
      text[0] = c;
      text[1] = '\0';
      if (dc) (*dc)(f->xpos,f->ypos,text);
      NXTCOL(f);
   }
   else
   {
      register float yp, yinc;
      float xp,xinc;
      char *table;
      int icol, irow, column;

      yp = f->ypos;
      xp = f->xpos;
      xinc = f->xinc;
      yinc = f->yinc;
      table = &(chartab[c-33][0]);
      
      if (dcp) {
	 for (icol = 0; icol < 5; icol++)
	 {
	    for (irow = 0; irow < 7; irow++) {
	       (*dcp)(xp,yp);
	       yp += yinc;
	    }
	    xp += xinc;
	    yp = f->ypos;
	 }
	 yp = f->ypos;
	 xp = f->xpos;
	 
	 if(c == 040) {
	    NXTCOL(f);
	    return;
	 }
      }
      
      for (icol = 0; icol < 5; icol++)
      {
	 column = table[icol];
	 if(column)
	 {
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	    if(column & 0200)
	       if (dp) (*dp)(xp,yp);
	    column <<= 1;
	    yp += yinc;
	 }
	 xp += xinc;
	 yp = f->ypos;
      }
      NXTCOL(f);
      return;
   }
}

#define MAX_STR_LEN 80
static char s[MAX_STR_LEN];

void drawnum (char *fmt, float n)
{
   sprintf (s, fmt, n);
   drawtext (s);
}

void drawfnum (int dpoints, float n)
{
   char fmt[12];

   /* first make format string for number */
   
   sprintf(fmt, "%%.%df", dpoints); 
   sprintf (s, fmt, n);
   drawtext (s);
}


void drawf(char *fmt, double n)         /* draw floating number on screen */
{
   sprintf (s, fmt, n);
   drawtext(s);
}

void HitRetKey(void)
{
   printf("Hit return to continue: ");
   getchar();
}

void copyframe(FRAME *from, FRAME *to)
{
   memcpy(to, from, sizeof(FRAME));
   to->fontname = (char *) malloc(strlen(from->fontname)+1);
   strcpy(to->fontname, from->fontname);
}

void frame(void)         /* draw a frame around current world window (???) */
{
   register int olduser;

   olduser = contexp->mode;               /* save user/device stat */
   user();
   moveto(contexp->xul,contexp->yub);
   lineto(contexp->xul,contexp->yut);
   lineto(contexp->xur,contexp->yut);
   lineto(contexp->xur,contexp->yub);
   lineto(contexp->xul,contexp->yub);
   setuser(olduser);                      /* restore user/device stat */
}

void frameport(void)    /* draw a frame around current viewport */
{
   register int olduser;
   
   olduser = contexp->mode;               /* save user/device stat */
   user();
   moveto(contexp->xl,contexp->yb);
   lineto(contexp->xl,contexp->yt);
   lineto(contexp->xr,contexp->yt);
   lineto(contexp->xr,contexp->yb);
   lineto(contexp->xl,contexp->yb);
   setuser(olduser);                      /* restore user/device stat */
}

void gfill(float xl, float yl, float xh, float yh)     /* fill a screen region */
{
   FRAME *f = contexp;
   register float x, y;
   register float xsl, xsh, ysl, ysh;
   int saveuser;

   moveto(xl, yl);                 /* get screen coordinates       */
   xsl = f->xpos;
   ysl = f->ypos;
   moveto(xh, yh);
   xsh = f->xpos;
   ysh = f->ypos;
   saveuser = f->mode;
   f->mode = 0;
   for (x = xsl; x <= xsh; x++)    /* fill region                  */
      for (y = ysl; y <= ysh; y++)
	 dotat(x, y);
   f->mode = saveuser;
}

int roundiv(int x, int y)
{
   x += y >> 1;
   return(x / y);
}

void tck(char *title)
{
   FRAME *fp= contexp;
   static int labeltick = 1;       /* defaults to tick alone */
   
   if (labeltick != 0) {
      setuser(0);
      linerel(0.0,-fp->linsiz/2.0);
      moverel(fp->colsiz/2.0,0.0);
      if (labeltick < 0)
	 drawtext(title);
      setuser(1);
   }
}

void tickat(float x, float y, char *title)
{
   moveto(x,y);
   tck(title);
}

void screen(void)
{
   setuser(0);
   setclip(0);
}

void user(void)
{
   setuser(1);
   setclip(1);
}

void cross(void)                     /* draw a cross at current pos  */
{
   FRAME *f= contexp;

   screen();
   moverel(-3.0*XUNIT(f),0.0);
   linerel(6*XUNIT(f),0.0);
   moverel(-3*XUNIT(f),-3.0*YUNIT(f));
   linerel(0.0,6.0*YUNIT(f));
   user();
}

void drawbox(float xl, float yl, float xh, float yh)        /* draw a box */
{
   moveto(xl,yl);
   lineto(xl,yh);
   lineto(xh,yh);
   lineto(xh,yl);
   lineto(xl,yl);
}

void screen2window(int x, int y, float *px, float *py)
{
  FRAME *f = contexp;
  float argx, argy;
  
  argx = (float)x;
  argy = (float)y;
  SCREEN(f,argx,argy);
  *px = argx;
  *py = argy;
}

void window2screen(int *px, int *py, float x, float y)
{
  FRAME *f = contexp;
  float argx, argy;

  argx = x;
  argy = y;
  WINDOW(f,argx,argy);

  *px = (int)argx;
  *py = (int)argy;
}


void window_to_screen(float x, float y, int *px, int *py)
{
  FRAME *f = contexp;
  WINDOW(f, x, y);
  *px = (int) x;
  *py = (int) ((f->ysres-1) - y);
}

void screen_to_window(int x, int y, float *px, float *py)
{
  FRAME *f = contexp;
  float argx = x;
  float argy = ((f->ysres-1) - y);

  SCREEN(f,argx,argy);
  *px = argx;
  *py = argy;
}

void maketitle(char *s, float x, float y)
{
  int olduser = setuser(0);
  int oldj = setjust(0);
  moveto(x,y);
  drawtext(s);
  setjust(oldj);
  setuser(olduser);
} 

void makeftitle(char *s, float x, float y)
{
  FRAME *f = contexp;
  int olduser = setuser(0);
  int oldj = setjust(0);
  float x1 = f->xl + x*(f->xr - f->xl);
  float y1 = f->yb + y*(f->yt - f->yb);

  moveto(x1, y1);
  drawtext(s);
  setjust(oldj);
  setuser(olduser);
} 


