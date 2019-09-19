/* $Header$ */

/* Purpose: Description (definition) of spherical geometry functions */

/* This file includes BSD-licensed code whose copyright is held by another author
   The copyright owner and license terms for the NCO modifications to that code are
   Copyright (C) 2018--present Charlie Zender
   This file is part of NCO, the netCDF Operators. NCO is free software.
   You may redistribute and/or modify NCO under the terms of the 
   GNU General Public License (GPL) Version 3 with exceptions described in the LICENSE file */

/* This copyright statement and license terms for modification and redistribution 
   of the original code were agreed to by the original author, Joseph O'Rourke, on 20190517:

   This code is described in "Computational Geometry in C" (Second Edition),
   Chapter 7.  It is not written to be comprehensible without the
   explanation in that book.
   
   Written by Joseph O'Rourke.
   Last modified: December 1997
   Questions to jorourke@smith.edu.
   -------------------------------------------------------------------------
   Copyright 1997 by Joseph O'Rourke <jorourke@smith.edu>
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   
   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ------------------------------------------------------------------------- */

/* Usage:
   #include "nco_sph.h" *//* Spherical geometry intersections */

#ifndef NCO_SPH_H /* Contents have not yet been inserted in current source file */
#define NCO_SPH_H

#include        <stdlib.h>
#include        <stdio.h>
#include        <math.h>

/* Personal headers */
#include "nco.h" /* netCDF Operator (NCO) definitions */
#include "nco_mmr.h"     /* Memory management */
#include "nco_omp.h"     /* OpenMP utilities */
#include "nco_rgr.h"     /* Regridding */
#include "nco_sld.h"     /* Swath-Like Data */
#include "nco_sng_utl.h" /* String utilities */

#include "nco_crt.h" /* Cartesian geometry intersections */
#include "nco_ply.h" /* Polygon structure & utilities */

#define NBR_SPH (5)
#define NBR_RLL (5)

#define VP_MAX    1000            /* Max # of pts in polygon */

/* this is 1.0e-20 * PI / 180.0 */
#define ARC_MIN_LENGTH_RAD (1.0e-15)

/* smallest RADIAN */
#define SIGMA_RAD (1.0e-12)

#define SIGMA_TOLERANCE (1.0e-12)
#define DOT_TOLERANCE (1.0e-14)

/*regular
#define DIST_TOLERANCE (1.0e-14)
 */

/* this value plays nice with edges on grids/ne120np4_pentagons.100310.nc */
#define DIST_TOLERANCE (1.0e-15)

/* convert Degrees to Radians */
#define D2R(x)  ((x) * M_PI / 180.0)
/* convert Radians to degrees */
#define R2D(x)  ((x) * 180.0 / M_PI)

/* if true then longitude 0-360 */
/* we need this to convert 3D back to 2D */
#define IS_LON_360 (1)

// #define DEBUG_SPH (1)

#ifdef __cplusplus
/* Use C-bindings so C++-compiled and C-compiled libraries are compatible */
extern "C" {
#endif /* !__cplusplus */

/*---------------------------------------------------------------------
Function prototypes.
---------------------------------------------------------------------*/

int
nco_sph_intersect(poly_sct *P, poly_sct *Q, poly_sct *R, int *r, int flg_snp_to, const char *pq_pre);

char
nco_sph_seg_int_old(double *a, double *b, double *c, double *d, double *p, double *q);

nco_bool
nco_sph_seg_int(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1, int flg_snp_to, char *codes);

char
nco_sph_seg_parallel(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1, poly_vrl_flg_enm *inflag );

nco_bool
nco_sph_seg_vrt_int(double *a, double *b, double *c);

int
nco_sph_lhs(double *Pi, double *Qi);

nco_bool
nco_sph_face(int iLHS, int iRHS, int jRHS);

double
nco_sph_dot(double *a, double *b);

double
nco_sph_dot_nm(double *a, double *b);

double
nco_sph_cross(double *a, double *b, double *c);

double
nco_sph_cross2(double *a, double *b, double *c);


void
nco_sph_add(double *a, double *b, double *c);

void
nco_sph_sub(double *a, double *b, double *c);

void
nco_sph_mlt(double *a, double m);

double
nco_sph_dist(double *a, double *b);

double
nco_sph_rad(double *a);

double
nco_sph_rad2(double *a);

double
nco_sph_sxcross(double *a, double *b, double *c);

void
nco_sph_adi(double *a, double *b);

void
nco_sph_add_pnt(double **R, int *r, double *P);

nco_bool
nco_sph_between(double a, double b, double x);

nco_bool
nco_sph_lonlat_between(double *a, double *b, double *x);

int
nco_sph_parallel(double *a, double *b, double *c, double *d, double *p, double *q);

int
nco_sph_parallel_lat(double *a, double *b, double *c, double *d, double *p, double *q);


void
nco_sph_prn_pnt(const char *sMsg, double *p, int style, nco_bool bRet);

nco_bool
nco_sph_is_convex(double **sP, int np);

void
nco_sph_prn(double **sR, int r, int istyle);

int
nco_sph_pnt_in_poly(double **sP, int n, double *pControl, double *pVertex);

nco_bool
nco_sph_poly_in_poly(poly_sct *sP,poly_sct *sQ);

void
nco_sph_set_domain(double lon_min_rad, double lon_max_rad, double lat_min_rad, double lat_max_rad);

void
nco_sph_add_lonlat(double *ds);

int
nco_sph_mk_control(poly_sct *sP, nco_bool bInside,  double* pControl  ); /* make a control point that is outside polygon */

nco_bool
nco_sph_intersect_pre(poly_sct *sP,poly_sct *sQ, char *sq_sng  );

int
nco_sph_process_pre(poly_sct *sQ, char *sq_sng, nco_bool *bGenuine);

void
nco_sph_centroid_mk(poly_sct *sP, double *pControl);

nco_bool
nco_sph_inside_mk(poly_sct *sP, double *pControl);

nco_bool
nco_sph_metric( double *p, double *q);

int
nco_sph_metric_int(double *c, double *d, double *Icross);


/***************** nco_geo functions these manimpulate lat & lon  ***************************/
void
nco_geo_sph_2_lonlat(double *a, double *lon, double *lat, nco_bool bDeg);

void
nco_geo_lonlat_2_sph(double lon, double lat, double *b, nco_bool bDeg);

double
nco_geo_lat_correct(double lat1, double lon1, double lon2);

void
nco_geo_get_lat_correct(double lon1, double lat1, double lon2, double lat2, double *dp_min, double *dp_max, nco_bool bDeg);



/**************** functions for RLL grids ***************************************************/
int
nco_rll_intersect(poly_sct *P, poly_sct *Q, poly_sct *R, int *r);

char
nco_rll_seg_int(double *a, double *b, double *c, double *d, double *p, double *q);

char
nco_rll_seg_parallel(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1, poly_vrl_flg_enm *inflag );

void
nco_rll_area(poly_sct *pl);

nco_bool
nco_rll_is_lat_circle(double *p0, double *p1);

int
nco_rll_lhs(double *p0, double *QCross);

int
nco_rll_lhs_lat(double *p0, double *q0, double *q1);



/*********************** functions for matrix*****************************************************/
void
nco_mat_mlt_3x3
(double mat[], double vec[], double vec_out[]);

nco_bool
nco_mat_inv_3x3(double *mat, double *mat_inv);

nco_bool
nco_mat_int_pl(const double *a1, const double *a2, const double *l1, const double *l2, double *p,double *t);

#ifdef __cplusplus
} /* end extern "C" */
#endif /* !__cplusplus */

#endif /* NCO_SPH_H */
