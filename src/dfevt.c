/*************************************************************************
 *
 *  NAME
 *    dfevt.c
 *
 *  DESCRIPTION
 *    Functions for extracting events from the df structure.
 *
 *  AUTHOR
 *    DLS
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "df.h"
#include "dfana.h"

#include <utilc.h>
#ifdef WIN32
#define strcasecmp stricmp
#endif

/*
 * These are tags from the DF_FILE that can be found in 
 * df.h, and which are used to access individual events from df files
 */
static struct TableEntry TagTable[] = {
  { "FIXON", E_FIXON_TAG },
  { "FIXOFF", E_FIXOFF_TAG },
  { "STIMON", E_STIMON_TAG, },
  { "STIMOFF", E_STIMOFF_TAG, },
  { "RESP", E_RESP_TAG },
  { "PATON", E_PATON_TAG },
  { "PATOFF", E_PATOFF_TAG, },
  { "STIMTYPE", E_STIMTYPE_TAG },
  { "PATTERN", E_PATTERN_TAG },
  { "REWARD", E_REWARD_TAG },
  { "PROBEON", E_PROBEON_TAG },
  { "PROBEOFF", E_PROBEOFF_TAG },
  { "SAMPON", E_SAMPON_TAG },
  { "SAMPOFF", E_SAMPOFF_TAG, },
  { "FIXATE", E_FIXATE_TAG, },
  { "DECIDE", E_DECIDE_TAG },
  { "STIMULUS", E_STIMULUS_TAG },
  { "DELAY", E_DELAY_TAG },
  { "ISI", E_ISI_TAG },
  { "UNIT", E_UNIT_TAG },
  { "INFO", E_INFO_TAG },
  { "TRIALTYPE", E_TRIALTYPE_TAG },
  { "CUE", E_CUE_TAG },
  { "DISTRACTOR", E_DISTRACTOR_TAG },
  { "TARGET", E_TARGET_TAG },
  { "ABORT", E_ABORT_TAG },
  { "PUNISH", E_PUNISH_TAG },
  { "SACCADE", E_SACCADE_TAG },
  { "WRONG", E_WRONG_TAG },
  { "BLANKING", E_BLANKING_TAG }
};


/************************************************************************/
/*                     Event Retrieval Functions                        */
/************************************************************************/

int evGetTagID(char *tagname, int *tag)
{
  int i, ntags = sizeof(TagTable)/sizeof(struct TableEntry);

  for (i = 0; i < ntags; i++) {
    if (!strcasecmp(tagname, TagTable[i].name)) {
      *tag = TagTable[i].id;
      return(1);
    }
  }
  return(0);
}

EV_LIST *evGetList(OBS_P *obsp, int tag, int *np)
{
  EV_LIST *evlist = NULL;
  int nparams = 1;

  switch(tag) {
  case E_INFO_TAG:
    evlist = EV_INFO(OBSP_EVDATA(obsp));
    nparams = 5;
    break;
  case E_TRIALTYPE_TAG:
    evlist = EV_TRIALTYPE(OBSP_EVDATA(obsp));
    nparams = 5;
    break;
  case E_STIMULUS_TAG:
    evlist = EV_STIMULUS(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_STIMON_TAG:
    evlist = EV_STIMON(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_STIMOFF_TAG:
    evlist = EV_STIMOFF(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_RESP_TAG:
    evlist = EV_RESP(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_REWARD_TAG:
    evlist = EV_REWARD(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PATTERN_TAG:
    evlist = EV_PATTERN(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PATON_TAG:
    evlist = EV_PATON(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PATOFF_TAG:
    evlist = EV_PATOFF(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PROBEON_TAG:
    evlist = EV_PROBEON(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PROBEOFF_TAG:
    evlist = EV_PROBEOFF(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_SAMPON_TAG:
    evlist = EV_SAMPON(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_SAMPOFF_TAG:
    evlist = EV_SAMPOFF(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_FIXON_TAG:
    evlist = EV_FIXON(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_FIXOFF_TAG:
    evlist = EV_FIXOFF(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_STIMTYPE_TAG:
    evlist = EV_STIMTYPE(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_CUE_TAG:
    evlist = EV_CUE(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_CORRECT_TAG:
    evlist = EV_CORRECT(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_TARGET_TAG:
    evlist = EV_TARGET(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_DISTRACTOR_TAG:
    evlist = EV_DISTRACTOR(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_DECIDE_TAG:
    evlist = EV_DECIDE(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_FIXATE_TAG:
    evlist = EV_FIXATE(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_DELAY_TAG:
    evlist = EV_DELAY(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_ISI_TAG:
    evlist = EV_ISI(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_UNIT_TAG:
    evlist = EV_UNIT(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_ABORT_TAG:
    evlist = EV_ABORT(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_WRONG_TAG:
    evlist = EV_WRONG(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_PUNISH_TAG:
    evlist = EV_PUNISH(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_SACCADE_TAG:
    evlist = EV_SACCADE(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  case E_BLANKING_TAG:
    evlist = EV_BLANKING(OBSP_EVDATA(obsp));
    nparams = 1;
    break;
  }

  *np = nparams;
  return(evlist);
}


/*
 *  The following functions all take a similar set of arguments, which
 * include the observation period, the event of interest, and pertinent
 * parameters for finding the data.
 *
 *  All depend of the evGetList function (see above) which finds a list
 * of events associated with the supplied tag (found in the table above).
 * Each event has an associated number of params, providing a way of dealing
 * with events that include more than one data param.  If no data for an 
 * event is stored in the observation period, 0 is returned.  Otherwise,
 * the number of parameters is returned (always one or more).
 */

int evGetTimeWithData(OBS_P *obsp, int tag, int data, int *evdata, int *time)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);

  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_VAL(evlist, i*nparams) == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (time) *time = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  if (time) *time = 0;
  return(0);
}

int evGetDataAtTime(OBS_P *obsp, int tag, int time, int *evdata)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) == time) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      return(nparams);
    }
  }
  return(0);
}

int evGetDataAtOrAfterTime(OBS_P *obsp, int tag, int time, int *evdata, 
			   int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) >= time) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist, i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}

int evGetDataAtOrBeforeTime(OBS_P *obsp, int tag, int time, int *evdata, 
			    int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) < time) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }

  if (rt) *rt = 0;
  return(0);
}

int evGetDataBeforeTime(OBS_P *obsp, int tag, int time, int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) < time) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  
  if (rt) *rt = 0;
  return(0);
}

int evGetDataAfterTime(OBS_P *obsp, int tag, int time, int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) > time) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}


int evGetWithDataAtTime(OBS_P *obsp, int tag, int data, int time, int *evdata)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) == time && 
	EV_LIST_VALS(evlist)[i*nparams] == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      return(nparams);
    }
  }
  return(0);
}

int evGetWithDataAtOrAfterTime(OBS_P *obsp, int tag, int data, int time, 
			       int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) >= time &&
	EV_LIST_VALS(evlist)[i*nparams] == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist, i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}

int evGetWithDataAtOrBeforeTime(OBS_P *obsp, int tag, int data, int time, 
				int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) < time &&
	EV_LIST_VALS(evlist)[i*nparams] == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }

  if (rt) *rt = 0;
  return(0);
}

int evGetWithDataBeforeTime(OBS_P *obsp, int tag, int data, int time, 
			    int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) <= time &&
	EV_LIST_VALS(evlist)[i*nparams] == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  
  if (rt) *rt = 0;
  return(0);
}

int evGetWithDataAfterTime(OBS_P *obsp, int tag, int data, int time, 
			   int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) > time  &&
	EV_LIST_VALS(evlist)[i*nparams] == data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}

int evGetWithNotDataAtTime(OBS_P *obsp, int tag, int data, int time, 
			   int *evdata)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) == time && 
	EV_LIST_VALS(evlist)[i*nparams] != data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      return(nparams);
    }
  }
  return(0);
}

int evGetWithNotDataAtOrAfterTime(OBS_P *obsp, int tag, int data, int time, 
				  int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) >= time &&
	EV_LIST_VALS(evlist)[i*nparams] != data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist, i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}

int evGetWithNotDataAtOrBeforeTime(OBS_P *obsp, int tag, int data, int time, 
				   int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) < time &&
	EV_LIST_VALS(evlist)[i*nparams] != data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }

  if (rt) *rt = 0;
  return(0);
}

int evGetWithNotDataBeforeTime(OBS_P *obsp, int tag, int data, int time, 
			       int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = EV_LIST_N(evlist)-1; i >= 0; i--) {
    if (EV_LIST_TIME(evlist, i) <= time &&
	EV_LIST_VALS(evlist)[i*nparams] != data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  
  if (rt) *rt = 0;
  return(0);
}

int evGetWithNotDataAfterTime(OBS_P *obsp, int tag, int data, int time, 
			   int *evdata, int *rt)
{
  int i;
  int nparams = 1;
  EV_LIST *evlist = evGetList(obsp, tag, &nparams);
  
  if (!evlist) return 0;

  for (i = 0; i < EV_LIST_N(evlist)/nparams; i++) {
    if (EV_LIST_TIME(evlist, i) > time  &&
	EV_LIST_VALS(evlist)[i*nparams] != data) {
      if(evdata)
	memcpy(evdata, &EV_LIST_VALS(evlist)[i*nparams], nparams*sizeof(int));
      if (rt) *rt = EV_LIST_TIME(evlist,i);
      return(nparams);
    }
  }
  if (rt) *rt = 0;
  return(0);
}


DYN_LIST *evGetSpikes(OBS_P *obsp, int start, int stop, int offset)
{
  int i, time;
  SP_CH_DATA *spch = SP_CHANNEL(OBSP_SPDATA(obsp),0);
  DYN_LIST *slist;

  if (!spch) return(NULL);

  slist = dfuCreateDynList(DF_FLOAT, 25);

  for(i = 0; i < SP_CH_NSPTIMES(spch); i++) {
    time = SP_CH_SPTIMES(spch)[i];
    if (time > start) {
      if (time > stop) return(slist);
      else dfuAddDynListFloat(slist, time-(float) offset);
    }
  }

  return(slist);
}


int evGetMeanEMPositions (OBS_P *obsp, float *hmean, float *vmean,
			  int *startp, int *stopp)
{
  EM_DATA *emdata;

  int k, emon;
  int fixbegidx, fixendidx, stop = 0, start = 0;
  double sumx, sumy, cnt;

  if (!obsp) return(0);

  emdata = OBSP_EMDATA(obsp);
  if (!emdata) return 0;

  emon = EM_ONTIME(emdata);
  
  /* Just set to zero */
  if (startp && *startp == 0 &&
      stopp && *stopp == 0) {
    *hmean = *vmean = 0.0;
    return 1;
  }

  if (!startp) {
    if (!O_FIXATE_N(obsp)) return 0;
    start = O_FIXATE_T(obsp);
  }
  else start = *startp;

  if (!stopp) {
    if (!O_STIMULUS_N(obsp) && !O_PATTERN_N(obsp))
      return(0); 
    if (O_STIMULUS_N(obsp)) stop = O_STIMULUS_T(obsp);
    else stop = O_PATTERN_T(obsp);
  }
  else stop = *stopp;

  fixbegidx = (start - emon) / EM_RATE(emdata);
  fixendidx = (stop - emon) / EM_RATE(emdata);
  if (fixendidx >= EM_NSAMPS(emdata)) fixendidx = EM_NSAMPS(emdata)-1;
  
  cnt = sumx = sumy = 0;
  for (k = fixbegidx; k < fixendidx; k++) {
    sumx += EM_SAMPS_H(emdata)[k];
    sumy += EM_SAMPS_V(emdata)[k];
    cnt++;
  }
  *hmean = sumx / (float)cnt;
  *vmean = sumy / (float)cnt;
  return(1);
}


DYN_LIST *evGetEMData(OBS_P *obsp, int start, int stop, 
		      int *zerostartp, int *zerostopp, int mode)
{
  int i, start_index, stop_index;
  float pd;
  float hmean, vmean, hpos, vpos, t;
  EM_DATA *emd;
  DYN_LIST *ems = NULL, **vals;
  DYN_LIST *tmp, *time, *h_em, *v_em;

  if (!obsp || !OBSP_EMDATA(obsp)) return(NULL);

  if (start > stop) return NULL;

  emd = OBSP_EMDATA(obsp);

  start_index = (start-EM_ONTIME(emd))/EM_RATE(emd);
  if (start_index < 0) return(NULL);
  stop_index = (stop-EM_ONTIME(emd))/EM_RATE(emd)+1;
  if (stop_index >= EM_NSAMPS(emd)) stop_index = EM_NSAMPS(emd)-1;

  pd = (float)EM_PNT_DEG(emd);
  
  if (!evGetMeanEMPositions (obsp, &hmean, &vmean, zerostartp, zerostopp)) 
    return(NULL);

  switch (mode) {
  case EM_ALL:			/* get horiz only */
    /* Create the list of lists, containing 3 float lists */
    ems = dfuCreateDynList(DF_LIST, 3);
    tmp = dfuCreateDynList(DF_FLOAT, stop_index-start_index);
    dfuAddDynListList(ems, tmp);	/* for time */
    dfuAddDynListList(ems, tmp);	/* for hems */
    dfuAddDynListList(ems, tmp);	/* for vems */
    dfuFreeDynList(tmp);
    
    /* Get pointers to the three lists */
    vals = (DYN_LIST **) DYN_LIST_VALS(ems);
    time = vals[0];
    h_em = vals[1];
    v_em = vals[2];
    
    for (i = start_index, t = start; i < stop_index; i++, t+=EM_RATE(emd)) {
      hpos = ((float)(EM_SAMPS_H(emd)[i])-hmean)/pd; 
      vpos = ((float)(EM_SAMPS_V(emd)[i])-vmean)/pd; 
      dfuAddDynListFloat(time, t);
      dfuAddDynListFloat(h_em, hpos);
      dfuAddDynListFloat(v_em, vpos);
    }
    break;
  case EM_HORIZ:
    ems = dfuCreateDynList(DF_FLOAT, stop_index-start_index);
    for (i = start_index, t = start; i < stop_index; i++, t+=EM_RATE(emd)) {
      hpos = ((float)(EM_SAMPS_H(emd)[i])-hmean)/pd; 
      dfuAddDynListFloat(ems, hpos);
    }
    break;
  case EM_VERT:
    ems = dfuCreateDynList(DF_FLOAT, stop_index-start_index);
    for (i = start_index, t = start; i < stop_index; i++, t+=EM_RATE(emd)) {
      vpos = ((float)(EM_SAMPS_V(emd)[i])-vmean)/pd; 
      dfuAddDynListFloat(ems, vpos);
    }
    break;
  }
  return(ems);
}

