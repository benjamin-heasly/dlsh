/************************************************************************
      axes.c - axis drawing utilities                                        
      Nikos K. Logothetis
      Modified David A. Leopold 14-APR-94
      NOTE: lxaxis() and lyaxis() are now changed in that they no longer
            include their fifth argument, the justification. Instead, to
            make things more compatible with the postscript drawing, the
            text is drawn at the center of the axes, and the specific 
            centering work is done by setting a justification flag.
*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cgraph.h>
#include <lablib.h>

extern FRAME *contexp;

void x_tic_label(FRAME *, float, float, float, int);
void drawxtic(FRAME *, float , float , float);
void y_tic_label(FRAME *, float, float, float, int);
void drawytic(FRAME *, float , float , float);

void axes (char *xlabel, char *ylabel)
{
   xaxis (xlabel);
   yaxis (ylabel);
}

void uboxaxes (void)
{
   float tic;

   tic = contexp->xus / 10.0;
   lxaxis (contexp->yub, -tic, 0, NULL);
   tic = contexp->yus / 10.0;
   lyaxis (contexp->xul, -tic, 0, NULL);
   up_xaxis (NULL);
   right_yaxis (NULL);
}

void boxaxes (char *xlabel, char *ylabel)
{
   xaxis (xlabel);
   yaxis (ylabel);
   up_xaxis (NULL);
   right_yaxis (NULL);
}

void xaxis (char *label)
{
   float tic;
   
   tic = contexp->xus / 10.0;
   lxaxis (contexp->yub, -tic, 2, label);
}

void yaxis (char *label)
{
   float tic;
   
   tic = contexp->yus / 10.0;
   lyaxis (contexp->xul, -tic, 2, label);
}

void up_xaxis (char *label)
{
   float tic;
   
   tic = contexp->xus / 10.0;
   lxaxis (contexp->yut, -tic, 0, label);
}

void right_yaxis (char *label)
{
   float tic;

   tic = contexp->yus / 10.0;
   lyaxis (contexp->xur, -tic, 0, label);
}

int lxaxis (float y, float tic, int ltic, char *label)
{
  static FRAME localf;              /* local frame */
  FRAME *fp = &localf;              /* local pointer */
  FRAME *oldframe;
  extern FRAME *setstatus();
  float x, low, high;
  float yoffset;           /* to align text */
  float log_tic;
  int dec_points;
  int oldj;
  
  /*
   * Make estimate of reasonable # of decimal points to use based
   * on tic separation.  The algorithm will take log10(tic) and if
   *    > 1 : No decimal points
   *    > 0 : 1 dp
   *    < -0: -x
   */
  
  if (tic == 0.0) log_tic = 0.0;
  log_tic = (float) log10((double) fabs(tic));
  if (log_tic > 1.0) dec_points = 0;
  else if (log_tic > 0.0) dec_points = 1;
  else dec_points = (int) fabs(log_tic) + 1;
  
  copyframe(contexp, fp);
  oldframe = setstatus(fp);
  user();                           /* draw axis */
  setclip(0);
  moveto(fp->xul,y);
  lineto(fp->xur,y);

  if (label) {                            /* label x axis */
    oldj = setjust(CENTER_JUST);
    moveto((fp->xul + fp->xur) / 2.0, y);
    screen();

    yoffset = 2.0*fp->linsiz; 
    moverel(0.0, -yoffset);

    if (ltic)
      moverel(0.0, -4.0 * fp->linsiz / 2.0);
    drawtext(label);
  }
  if (tic == 0.0) {
    setstatus(oldframe);
    return(1);
  }
  
  low = MIN(fp->xul,fp->xur);   /* make tics */
  high = MAX(fp->xul,fp->xur);

  if((low <= 0.0) && (high >= 0.0)) {
    for (x = 0.0; x >= low; x -= fabs(tic)) drawxtic(fp,x,y,tic);
    for (x = 0.0; x <= high; x += fabs(tic)) drawxtic(fp,x,y,tic);
  }
  else if (high < 0.0 || low > 0.0)  
      for (x = low; x <= high; x += fabs(tic)) drawxtic(fp,x,y,tic);
	
  if (ltic) {  
    tic*=abs(ltic);
    if(low <= 0.0 && high >= 0.0) {
      if(low) {
	for (x = 0; x >= low; x -= fabs(tic)) 
	  x_tic_label(fp,x,y,tic,dec_points);
	for (x = 0; x <= high; x += fabs(tic)) 
	  x_tic_label(fp,x,y,tic,dec_points);
      } else {
	for (x = 0.0; x <= high; x += fabs(tic)) 
	  x_tic_label(fp,x,y,tic,dec_points);
      } 
    } else {
	for (x = low; x <= high; x += fabs(tic)) 
	  x_tic_label(fp,x,y,tic,dec_points);
    }
  }
  setstatus(oldframe);
  return(1);
}


int lyaxis (float x, float tic, int ltic, char *label)
{
  static FRAME localf;		/* local frame */
  FRAME *fp = &localf;		/* local pointer */
  FRAME *oldframe;
  extern FRAME *setstatus();
  float y, low, high;
  float xoffset;
  float log_tic;
  int dec_points;
  int tic_label_chars;		/* Number of chars in tic label */
  int oldj, oldo;
  
  /*
   * Make estimate of reasonable # of decimal points to use based
   * on tic separation.  The algorithm will take log10(tic) and if
   *    > 1 : No decimal points
   *    > 0 : 1 dp
   *    < -0: -x
   */
  
  if (tic == 0.0) log_tic = 0.0;
  else log_tic = (float) log10((double) fabs(tic));
  if (log_tic > 1.0) dec_points = 0;
  else if (log_tic > 0.0) dec_points = 1;
  else dec_points = (int) fabs(log_tic) + 1;
  
  copyframe(contexp, fp);
  oldframe = setstatus(fp);
  user();                           /* draw axis */
  setclip(0);
  moveto(x,fp->yub);
  lineto(x,fp->yut);

  /* figure out the MAX # of chars used for tic labels */
  if (fp->yus < 10.0) tic_label_chars = 2+dec_points;
  else tic_label_chars = (int) log10(fabs(fp->yus)) + (1+dec_points);
  
  if (label) {                            /* label y axis */
    moveto(x, (fp->yut + fp->yub) / 2.0);
    screen();

    /* move the label #chars + space + ticsize over to the left */
    
    xoffset = (tic_label_chars+1.5+1.5)*fp->colsiz;
    moverel(-xoffset, 0.0);

    oldj = setjust(CENTER_JUST);
    oldo = setorientation(1);
    drawtext(label);
    setjust(oldj);
    setorientation(oldo);
  }
  if (tic == 0.0) {
    setstatus(oldframe);
    return(1);
  }
  
  low = MIN(fp->yub,fp->yut);   /* make tics */
  high = MAX(fp->yub,fp->yut);

  if((low <= 0.0) && (high >= 0.0)) {
    for (y = 0.0; y >= low; y -= fabs(tic)) drawytic(fp,x,y,tic);
    for (y = 0.0; y <= high; y += fabs(tic)) drawytic(fp,x,y,tic);
  } else if (high < 0.0 || low > 0.0)  
    for (y = low; y <= high; y += fabs(tic)) drawytic(fp,x,y,tic);


  if (ltic) {  
    tic *= abs(ltic);
    if(low <= 0.0 && high >= 0.0) {
      if(low) {
	for (y = 0; y >= low; y -= fabs(tic)) 
	  y_tic_label(fp,x,y,tic,dec_points);
	for (y = 0; y <= high; y += fabs(tic)) 
	  y_tic_label(fp,x,y,tic,dec_points);
      } else {
	for (y = 0.0; y <= high; y += fabs(tic)) 
	  y_tic_label(fp,x,y,tic,dec_points);
      } 
    } else {
	for (y = low; y <= high; y += fabs(tic)) 
	  y_tic_label(fp,x,y,tic,dec_points);
    }
  }
  setstatus(oldframe);

  return(1);
}




void drawxtic(FRAME *fp, float x, float y, float tic)
{
    user();
    moveto(x,y);
    screen();
    if (tic < 0.0)
      linerel(0.0, -fp->linsiz / 2.0);
    else {
      moverel(0.0, fp->linsiz / 2.0);
      linerel(0.0, -fp->linsiz);
    }
}  

void x_tic_label(FRAME *fp, float x, float y, float tic, int dp)
{
  int oldo, oldj;
  
  user();
  moveto(x,y);
  screen();
  setorientation(0);
  oldj = setjust(CENTER_JUST);
  oldo = setorientation(0);
  if (tic < 0.0)
    linerel(0.0, -fp->linsiz );
  else {
    moverel(0.0, fp->linsiz);
    linerel(0.0, -2.0*fp->linsiz);
  }
  moverel(0.0, fp->linsiz);

  moverel(0.0, -2.0*fp->linsiz);
  drawfnum(dp,x);
  setjust(oldj);
  setorientation(oldo);
} 

void drawytic(FRAME *fp, float x, float y, float tic)
{
    user();
    moveto(x,y);
    screen();
    if (tic < 0.0)
      linerel(-fp->colsiz/2.0, 0.0);
    else {
      moverel(fp->colsiz / 2.0, 0.0);
      linerel(-fp->colsiz, 0.0);
    }
}  

void y_tic_label(FRAME *fp, float x, float y, float tic, int dp)
{
  int oldj = setjust(RIGHT_JUST);
  user();
  moveto(x,y);
  screen();
  if (tic < 0.0)
    linerel(-fp->colsiz,0.0 );
  else {
    moverel(fp->colsiz, 0.0);
    linerel(-2.0*fp->colsiz, 0.0);
  }
  moverel(fp->colsiz, 0.0);

  moverel(-2.0*fp->colsiz, 0.0);
  drawfnum(dp,y);
  setjust(oldj);
} 

