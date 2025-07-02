/* from curves.c */
int closest_point(int npts, float *xvals, float *yvals, float x, float y);
int third_order_poly(int npts, float *xvals, float *yvals, int res,
		     float *xinterp, float *yinterp);
int bezier_interpolate(int npts, float *xvals, float *yvals, int res,
		     float *xinterp, float *yinterp);
int polygon_self_intersects(int nverts, float *x, float *y);
int polygon_centroid(float x[], float y[], int n,
		     float *xCentroid, float *yCentroid, float *area);
