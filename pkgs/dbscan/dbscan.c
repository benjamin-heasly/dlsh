#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <tcl.h>
#include <df.h>
#include <dfana.h>
#include <tcl_dl.h>

// DBSCAN clustering algorithm
// Input: distance matrix, number of points, epsilon, minPts
// Output: cluster assignments for each point

// Constants for cluster labels
#define UNCLASSIFIED -1
#define NOISE 0

// Function to find neighbors within epsilon radius
int* regionQuery(double** distMatrix, int pointIdx, double epsilon, int numPoints, int* numNeighbors) {
    int* neighbors = (int*)malloc(numPoints * sizeof(int));
    *numNeighbors = 0;
    
    for (int i = 0; i < numPoints; i++) {
        if (distMatrix[pointIdx][i] <= epsilon) {
            neighbors[(*numNeighbors)++] = i;
        }
    }
    
    return neighbors;
}

// Expand cluster from seed point
bool expandCluster(double** distMatrix, int pointIdx, int* neighbors, int numNeighbors, 
                  int clusterId, int* clusterAssignments, double epsilon, int minPts, int numPoints) {
    
    // Assign current point to cluster
    clusterAssignments[pointIdx] = clusterId;
    
    // Process all neighbors
    int i = 0;
    while (i < numNeighbors) {
        int currentPoint = neighbors[i];
        
        // If the point was previously marked as noise, add it to the cluster
        if (clusterAssignments[currentPoint] == NOISE) {
            clusterAssignments[currentPoint] = clusterId;
        } 
        // If the point is unclassified, add it to the cluster and expand
        else if (clusterAssignments[currentPoint] == UNCLASSIFIED) {
            clusterAssignments[currentPoint] = clusterId;
            
            // Find neighbors of the current point
            int numPointNeighbors = 0;
            int* pointNeighbors = regionQuery(distMatrix, currentPoint, epsilon, numPoints, &numPointNeighbors);
            
            // If it's a core point, add its neighbors to the processing list
            if (numPointNeighbors >= minPts) {
                // Add new neighbors to the list if they're not already processed
                for (int j = 0; j < numPointNeighbors; j++) {
                    int newNeighbor = pointNeighbors[j];
                    
                    // Check if this neighbor is already in our list or processed
                    bool alreadyIncluded = false;
                    for (int k = 0; k < numNeighbors; k++) {
                        if (neighbors[k] == newNeighbor) {
                            alreadyIncluded = true;
                            break;
                        }
                    }
                    
                    if (!alreadyIncluded) {
                        neighbors[numNeighbors++] = newNeighbor;
                    }
                }
            }
            
            free(pointNeighbors);
        }
        
        i++;
    }
    
    return true;
}

// Main DBSCAN function
int* dbscan(double** distMatrix, int numPoints, double epsilon, int minPts, int* numClusters) {
    // Initialize cluster assignments
    int* clusterAssignments = (int*)malloc(numPoints * sizeof(int));
    for (int i = 0; i < numPoints; i++) {
        clusterAssignments[i] = UNCLASSIFIED;
    }
    
    int clusterId = 1; // Start with cluster ID 1 (0 is reserved for noise)
    
    // Process each point
    for (int i = 0; i < numPoints; i++) {
        // Skip points that have already been processed
        if (clusterAssignments[i] != UNCLASSIFIED) {
            continue;
        }
        
        // Find neighbors
        int numNeighbors = 0;
        int* neighbors = regionQuery(distMatrix, i, epsilon, numPoints, &numNeighbors);
        
        // If not a core point, mark as noise for now
        if (numNeighbors < minPts) {
            clusterAssignments[i] = NOISE;
            free(neighbors);
            continue;
        }
        
        // Expand cluster from this core point
        expandCluster(distMatrix, i, neighbors, numNeighbors, clusterId, 
                     clusterAssignments, epsilon, minPts, numPoints);
        clusterId++;
        
        free(neighbors);
    }
    
    *numClusters = clusterId - 1;
    return clusterAssignments;
}

// Function to print cluster assignments
void printClusterAssignments(int* clusterAssignments, int numPoints) {
    printf("Point\tCluster\n");
    printf("-----\t-------\n");
    for (int i = 0; i < numPoints; i++) {
        printf("%d\t%d\n", i, clusterAssignments[i]);
    }
}

int dbscanCmd (ClientData data, Tcl_Interp *interp,
	       int objc, Tcl_Obj *objv[])
{
  DYN_LIST *dl;
  double epsilon = .5;
  int minPts = 2;
  
  if (objc < 1) {
    Tcl_WrongNumArgs(interp, 1, objv, "distance_matrix [eps] [minpts]");
    return (TCL_ERROR);
  }

  if (objc > 2) {
    if (Tcl_GetDoubleFromObj(interp, objv[2], &epsilon) != TCL_OK)
      return TCL_ERROR;
  }
  
  if (objc > 3) {
    if (Tcl_GetIntFromObj(interp, objv[3], &minPts) != TCL_OK)
      return TCL_ERROR;
  }
  
  if (tclFindDynList(interp, Tcl_GetString(objv[1]), &dl) != TCL_OK)
    return TCL_ERROR;
  
  if (!dynListIsMatrix(dl)) {
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		     ": invalid distance_matrix", NULL);
    return TCL_ERROR;
  }

  int rows, cols;
  dynMatrixDims(dl, (int *) &rows, (int *) &cols);
  if (rows != cols) {
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		     ": distance_matrix not square", NULL);
    return TCL_ERROR;
  }
  
  
  double **distance_matrix = (double**) malloc(sizeof(double*) * rows);
  if (distance_matrix == NULL) {
    Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		     ": memory error", NULL);
    return TCL_ERROR;
  }
  for (int i = 0; i < rows; i++) {
    distance_matrix[i] = (double*) malloc(sizeof(double) * cols);
    if (distance_matrix[i] == NULL) {
      for (int j = 0; j < i; j++) {
	free(distance_matrix[j]);
      }
      free(distance_matrix);
      Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		       ": memory error", NULL);
      return TCL_ERROR;
    }
  }
  
  // Fill the matrices with supplied data
  DYN_LIST **rowlists = (DYN_LIST **) DYN_LIST_VALS(dl);
  for (size_t i = 0; i < rows; i++) {
    float *row = (float *) DYN_LIST_VALS(rowlists[i]);
    for (size_t j = 0; j < cols; j++) {
      distance_matrix[i][j] = row[j];
    }
  }

  int numPoints = DYN_LIST_N(dl);
  int numClusters = 0;
  int* clusterAssignments = dbscan(distance_matrix, numPoints, epsilon, minPts, &numClusters);

  // Free memory
  for (int i = 0; i < numPoints; i++) {
    free(distance_matrix[i]);
  }
  free(distance_matrix);
  
  if (!clusterAssignments) {
      Tcl_AppendResult(interp, Tcl_GetString(objv[0]),
		       ": memory error", NULL);
      return TCL_ERROR;
  }
  
  DYN_LIST *assignments =
    dfuCreateDynListWithVals(DF_LONG, rows, clusterAssignments);
  
  return tclPutList(interp, assignments);
}


/*****************************************************************************
 *
 * EXPORT
 *
 *****************************************************************************/

#ifdef WIN32
EXPORT(int,Dbscan_Init) (Tcl_Interp *interp)
#else
int Dbscan_Init(Tcl_Interp *interp)
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

  if (Tcl_PkgProvide(interp, "dbscan", "1.0") != TCL_OK) {
    return TCL_ERROR;
  }
  
  Tcl_Eval(interp, "namespace eval dbscan {}");

  Tcl_CreateObjCommand(interp, "dbscan::cluster",
		       (Tcl_ObjCmdProc *) dbscanCmd,
		       (ClientData) NULL,
		       (Tcl_CmdDeleteProc *) NULL);
  
  return TCL_OK;
 }
