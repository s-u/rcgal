#include <stdlib.h>
#include <string.h>

#define USE_RINTERNALS
#include <Rinternals.h>

/* from point_poly.cpp */
void *shape_load_(double *x, double *y, int n, int *parts, int parts_n);
int   shape_inside_(void *s, double x, double y);
void  shape_free_(void *s);
void  shape_orientations_(void *s, int *where);
void  shape_areas_(void *s, double *where);
int   shape_count_(void *s);

SEXP shp_orientation(SEXP slist) {
    int i, ns = LENGTH(slist);
    SEXP res = PROTECT(allocVector(VECSXP, ns)), nam;
    for (i = 0; i < ns; i++) {
	SEXP shp = VECTOR_ELT(slist, i);
	SEXP pv = VECTOR_ELT(shp, 3);
	SEXP xv = VECTOR_ELT(shp, 4);
	SEXP yv = VECTOR_ELT(shp, 5);
	SEXP ori;
	void *shape = shape_load_(REAL(xv), REAL(yv), LENGTH(xv), INTEGER(pv), LENGTH(pv));
	int N = shape_count_(shape);
	ori = SET_VECTOR_ELT(res, i, allocVector(INTSXP, N));
	shape_orientations_(shape, INTEGER(ori));
	shape_free_(shape);
    }
    if ((nam = getAttrib(slist, R_NamesSymbol)) != R_NilValue)
	setAttrib(res, R_NamesSymbol, nam);
    UNPROTECT(1);
    return res;
}

SEXP shp_area(SEXP slist) {
    int i, ns = LENGTH(slist);
    SEXP res = PROTECT(allocVector(VECSXP, ns)), nam;
    for (i = 0; i < ns; i++) {
	SEXP shp = VECTOR_ELT(slist, i);
	SEXP pv = VECTOR_ELT(shp, 3);
	SEXP xv = VECTOR_ELT(shp, 4);
	SEXP yv = VECTOR_ELT(shp, 5);
	SEXP ori;
	void *shape = shape_load_(REAL(xv), REAL(yv), LENGTH(xv), INTEGER(pv), LENGTH(pv));
	int N = shape_count_(shape);
	ori = SET_VECTOR_ELT(res, i, allocVector(REALSXP, N));
	shape_areas_(shape, REAL(ori));
	shape_free_(shape);
    }
    if ((nam = getAttrib(slist, R_NamesSymbol)) != R_NilValue)
	setAttrib(res, R_NamesSymbol, nam);
    UNPROTECT(1);
    return res;
}

SEXP shp_inside(SEXP slist, SEXP pxv, SEXP pyv, SEXP sAll) {
    SEXP xv, yv, pv, res;
    double *x, *y, *px, *py;
    int *p, up = 0, *r, np, ns, mp = 0, i;
    int all = Rf_asInteger(sAll);
    if (TYPEOF(slist) != VECSXP || !inherits(slist, "shp"))
	Rf_error("input must be a list of shapes (shp object)");
    if (LENGTH(slist) == 0)
	return allocVector(INTSXP, 0);
    if (LENGTH(pxv) != LENGTH(pyv))
	Rf_error("point coordinates must have the same length");
    if (TYPEOF(pxv) != REALSXP) {
	pxv = PROTECT(coerceVector(pxv, REALSXP)); up++;
    }
    if (TYPEOF(pyv) != REALSXP) {
	pyv = PROTECT(coerceVector(pyv, REALSXP)); up++;
    }
    px = REAL(pxv);
    py = REAL(pyv);
    np = LENGTH(pxv);
    ns = LENGTH(slist);
    if (!all) {
	res = allocVector(INTSXP, np);
	r = INTEGER(res);
	memset(r, 0, sizeof(*r) * np);
	for (i = 0; i < ns; i++) {
	    int j;
	    double *bb;
	    void *shape = 0;
	    SEXP shp = VECTOR_ELT(slist, i);
	    bb = REAL(VECTOR_ELT(shp, 2));
	    pv = VECTOR_ELT(shp, 3); p = INTEGER(pv);
	    xv = VECTOR_ELT(shp, 4); x = REAL(xv);
	    yv = VECTOR_ELT(shp, 5); y = REAL(yv);
	    for (j = 0; j < np; j++) {
		double X = px[j], Y = py[j];
		/* is the point inside the bounding box? */
		if (X >= bb[0] && X <= bb[2] && Y >= bb[1] && Y <= bb[3]) {
		    /* lazy-load shapes */
		    if (!shape)
			shape = shape_load_(x, y, LENGTH(xv), p, LENGTH(pv));
		    
		    /* then use point/poly test */
		    if (shape_inside_(shape, X, Y) && !r[j]) {
			mp++;
			r[j] = i + 1;
			if (mp >= np) { /* if all points got matched, get out */
			    i = ns;
			    break;
			}
		    }
		}
	    }
	    if (shape)
		shape_free_(shape);
	}
	if (mp < np) /* replace 0 (no match) with NA */
	    for (i = 0; i < np; i++)
		if (r[i] == 0) r[i] = NA_INTEGER;
    } else {
	/* return a list of all matches - useful for heavily overlapping shapes */
        SEXP tmp = PROTECT(allocVector(INTSXP, ns)); /* temporary vector to store per-point matches */
        int *ti = INTEGER(tmp), j;
	/* FIXME: this is a temporary hack - we need to use R_alloc or similar to avoid leak on error */
	void **shapes = (void**) calloc(ns, sizeof(void*));
        memset(ti, 0, sizeof(ti[0]) * ns);
        res = PROTECT(allocVector(VECSXP, np)); /* result list */
        up += 2;
        for (i = 0; i < np; i++) {
            double X = px[i], Y = py[i];
            int k = 0;
            for (j = 0; j < ns; j++) {
                double *bb;
                SEXP shp = VECTOR_ELT(slist, j);
		
                bb = REAL(VECTOR_ELT(shp, 2));
                if (X >= bb[0] && X <= bb[2] && Y >= bb[1] && Y <= bb[3]) {
                    /* lazy-load shapes */
		    if (!shapes[j]) {
			pv = VECTOR_ELT(shp, 3); p = INTEGER(pv);
			xv = VECTOR_ELT(shp, 4); x = REAL(xv);
			yv = VECTOR_ELT(shp, 5); y = REAL(yv);
			shapes[j] = shape_load_(x, y, LENGTH(xv), p, LENGTH(pv));
		    }

                    if (shape_inside_(shapes[j], X, Y))
                        ti[k++] = j + 1;
                }
            }
            if (k) {
                memcpy(INTEGER(SET_VECTOR_ELT(res, i, allocVector(INTSXP, k))), ti, sizeof(ti[0]) * k);
                memset(ti, 0, sizeof(ti[0]) * k);
            }
        }
	for (j = 0; j < ns; j++)
	  if (shapes[j])
	    shape_free_(shapes[j]);
	free(shapes);
    }
    if (up) UNPROTECT(up);
    return res;
}
