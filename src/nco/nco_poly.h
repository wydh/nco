#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef NCO_POLY_H
#define NCO_POLY_H 

/* Personal headers */
#include "nco.h" /* netCDF Operator (NCO) definitions */
#include "nco_mmr.h" /* Memory management */
#include "nco_omp.h" /* OpenMP utilities */
#include "nco_rgr.h" /* Regridding */
#include "nco_sld.h" /* Swath-Like Data */
#include "nco_sng_utl.h" /* String utilities */

#include "kd.h"
#include "nco_vrl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CORNER_MAX 100


/* filter out cells  larger than below  */   
#define CELL_LATITUDE_MAX  30.0
#define CELL_LONGITUDE_MAX 180.0

  typedef enum {
     poly_none,
     poly_sph,
     poly_crt
  } poly_typ_enm;

  
  typedef struct{

    poly_typ_enm pl_typ;

    double *dp_x;    /* x  vertices */
    double *dp_y;    /* y vertices */
    double *dp_xyz;  /* maybe useful for 3D stuff */ 
    double area;  

    double dp_x_minmax[2];
    double dp_y_minmax[2];
    
    int  stat;     
    int crn_nbr; /* number of vertices */
    int mem_flg; /* [flg]    */ 

  } poly_sct;   


  poly_sct *
  nco_poly_free
  (poly_sct *pl);

  poly_sct *   
  nco_poly_init
  (void);

  poly_sct *
  nco_poly_init_lst
  (poly_typ_enm pl_typ,
   int arr_nbr,
   int mem_flg,
   double *dp_x_in,
   double *dp_y_in);

  poly_sct *
  nco_poly_init_crn
  (poly_typ_enm pl_typ,
  int crn_nbr_in);
  
  poly_sct*
  nco_poly_dpl
  (poly_sct *pl);

  void
  nco_poly_add_minmax
  (poly_sct *pl);

  void
  nco_poly_use_minmax_crn /* use the values of minmax box as dp_x, dp_y  */
  (poly_sct *pl);
  
  void
  nco_poly_prn
  (int style,
   poly_sct *pl);

  
  poly_sct*
  nco_poly_do_vrl(
  poly_sct *pl_in,
  poly_sct *pl_out);


  nco_bool            /* O [flg] True if point in inside (or on boundary ) of polygon */ 
  nco_poly_pnt_in_poly( 
  int crn_nbr,
  double x_in,
  double y_in,
  double *lcl_dp_x,
  double *lcl_dp_y);	

  int             /* O [nbr] returns number of points of pl_out that are inside pl_in */
  nco_poly_poly_in_poly( 
  poly_sct *pl_in,
  poly_sct *pl_out); 		     

  
  nco_bool
  nco_poly_wrp_splt( 
  poly_sct  *pl,
  nco_grd_lon_typ_enm grd_lon_typ,
  poly_sct **pl_wrp_left,
  poly_sct ** pl_wrp_right);

  void
  nco_poly_re_org(
  poly_sct *pl, 		  
  double *lcl_dp_x,
  double *lcl_dp_y);

  nco_bool  /* 0 [flg] True if polygon is convex */
  nco_poly_is_convex(
  poly_sct *pl);		     


/************************ functions that manipulate lists of polygons ****************************************************/

   void
   nco_poly_re_org_lst(  /* for each poly_sct*  in list re-order points so that first point is the leftermost point */
   poly_sct **pl_lst,
   int arr_nbr);
  
   poly_sct**             /* [O] [nbr] Array of poly_sct */   
   nco_poly_mk_lst(
   double *area, /* I [sr] Area of source grid */
   int *msk, /* I [flg] Mask on source grid */
   double *lat_ctr, /* I [dgr] Latitude  centers of source grid */
   double *lon_ctr, /* I [dgr] Longitude centers of source grid */
   double *lat_crn, /* I [dgr] Latitude  corners of source grid */
   double *lon_crn, /* I [dgr] Longitude corners of source grid */
   size_t grd_sz, /* I [nbr] Number of elements in single layer of source grid */
   long grd_crn_nbr, /* I [nbr] Maximum number of corners in source gridcell */
   nco_grd_lon_typ_enm grd_lon_typ, /* I [num] if not nil then split cells that straddle Greenwich or Dateline  */
   poly_typ_enm pl_typ,
   int *pl_nbr);    /* O [nbr] size  poly_sct */
		   
   poly_sct **
   nco_poly_lst_free(
   poly_sct **pl_lst,
   int arr_nbr);


   poly_sct **
   nco_poly_mk_vrl_lst(   /* create overlap mesh */
   poly_sct ** pl_lst_in,
   int pl_cnt_in,
   poly_sct ** pl_lst_out,
   int pl_cnt_out,
   int *pl_cnt_vrl);


   		   
      
  
  
#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* NCO_POLY_H  */