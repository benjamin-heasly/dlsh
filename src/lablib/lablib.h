/****************************************************************************
 *
 * NAME:        LABLIB.H
 *
 * PURPOSE:     Header file for all lab applications.
 *              This file includes all definitions needed
 *              by the data acquisition and analysis program running
 *              under MS-DOS.
 *
 * AUTHOR:      Nikos K. Logothetis, 18-NOV-93
 *
 ***************************************************************************/

/* ---------------------------------------------------------------
 *           Experiment IDs To Distinguish Datafiles                
 * --------------------------------------------------------------- */

#define AVS_EXP         1 
#define RIV_EXP         2
#define ISEG_EXP        3
#define OREC_EXP        4
#define NOREC_EXP       5
#define OBJ_RIV         6       /* phase out...                            */
#define OBJ_RIV_EXP     6       /* object rivalry (dqriv)                  */
#define MORPH_EXP       10
#define PRF_EXP         60
#define OPRF_EXP        61      /* orec prf (pauls et al.)                 */
#define OBJ_PRF_EXP     62      /* object rivalry prf (sheinb et al.)      */

#define YES             1
#define NO              0


#if !defined (MAX) 
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#if !defined (ABS)
#define ABS(a)  ((a) > 0 ? (a) : -(a))
#endif

#define BIT0 0x01               /* Bit masks...                             */
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#if !defined(PI)
#define PI      3.141592654
#endif
#define TWOPI   6.2831853       /* For trigonometrical functions        */

#define TAN1    0.01745506493   /* For trigonometrical functions        */
#define SIGMA   1.0		/* to compute z-scores			*/

#if !defined(MININT)
#define MAXINT  32000            /* start-values to compute minima       */
#define MININT -32000            /* start-values to compute maxima       */
#endif
#define  WORD   short

#if !defined(__FPOINT__)

#define __FPOINT__
typedef struct {
  float x;
  float y;
} FPOINT;

#endif


/*
 * Function prototypes
 */
double arctan (double, double);
double zscore (double);
double cdfnorm (double);
double get_rocpoint (double, double);
double get_d (double);
double get_2afc (double);
char *getflags(int *, char ***, char *, ...);
short prefix(char *, char *);


