/*
 * dg_json.c
 *  JSON I/O support for dynamic groups
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <df.h>
#include <dynio.h>

#include <jansson.h>

json_t *dl_to_json(DYN_LIST *dl)
{
  int i;  
  json_t *array = json_array();

  if (DYN_LIST_DATATYPE(dl) == DF_LIST) {
    json_t *subarray;
    
    DYN_LIST **vals = (DYN_LIST **) DYN_LIST_VALS(dl);
    DYN_LIST *curlist;

    for (i = 0; i < DYN_LIST_N(dl); i++) {
      curlist = (DYN_LIST *) vals[i];
      if (!curlist) {
	json_decref(array);
	return NULL;
      }
      subarray = dl_to_json(curlist);
      if (!subarray) {
	json_decref(array);
	return NULL;
      }
      json_array_append_new(array, subarray);
    }
    return array;
  }

  else {
    switch (DYN_LIST_DATATYPE(dl)) {
    case DF_LONG:
      {
	int *vals = (int *) DYN_LIST_VALS(dl);
	for (i = 0; i < DYN_LIST_N(dl); i++) {
	  json_array_append_new(array, json_integer(vals[i]));
	}
      }
      break;
    case DF_SHORT:
      {
	short *vals = (short *) DYN_LIST_VALS(dl);
	for (i = 0; i < DYN_LIST_N(dl); i++) {
	  json_array_append_new(array, json_integer(vals[i]));
	}
      }
      break;      
    case DF_CHAR:
      {
	char *vals = (char *) DYN_LIST_VALS(dl);
	for (i = 0; i < DYN_LIST_N(dl); i++) {
	  json_array_append_new(array, json_integer(vals[i]));
	}
      }
      break;      
    case DF_FLOAT:
      {
	float *vals = (float *) DYN_LIST_VALS(dl);
	for (i = 0; i < DYN_LIST_N(dl); i++) {
	  json_array_append_new(array, json_real(vals[i]));
	}
      }
      break;      
    case DF_STRING:
      {
	char **vals = (char **) DYN_LIST_VALS(dl);
	for (i = 0; i < DYN_LIST_N(dl); i++) {
	  json_array_append_new(array, json_string(vals[i]));
	}
	break;
      }
    }
  }
  return array;
}


json_t *dg_to_json(DYN_GROUP *dg)
{
  int i;
  json_t *json_dg, *json_list;

  json_dg = json_object();
  
  for (i = 0; i < DYN_GROUP_NLISTS(dg); i++) {
    json_list = dl_to_json(DYN_GROUP_LIST(dg,i));
    if (!json_list) {
      json_decref(json_dg);
      return NULL;
    }
    json_object_set_new(json_dg, DYN_LIST_NAME(DYN_GROUP_LIST(dg, i)), json_list);    
  }  
  return json_dg;
}


