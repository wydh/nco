/* $Header: /data/zender/nco_20150216/nco/src/nco++/ncap2.cc,v 1.7 2006-02-25 22:29:14 zender Exp $ */

/* ncap -- netCDF arithmetic processor */

/* Purpose: Compute user-defined derived fields using forward algebraic notation applied to netCDF files */

/* Copyright (C) 1995--2006 Charlie Zender

   This software may be modified and/or re-distributed under the terms of the GNU General Public License (GPL) Version 2
   The full license text is at http://www.gnu.ai.mit.edu/copyleft/gpl.html 
   and in the file nco/doc/LICENSE in the NCO source distribution.
   
   As a special exception to the terms of the GPL, you are permitted 
   to link the NCO source code with the HDF, netCDF, OPeNDAP, and UDUnits
   libraries and to distribute the resulting executables under the terms 
   of the GPL, but in addition obeying the extra stipulations of the 
   HDF, netCDF, OPeNDAP, and UDUnits licenses.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
   See the GNU General Public License for more details.
   
   The original author of this software, Charlie Zender, wants to improve it
   with the help of your suggestions, improvements, bug-reports, and patches.
   Please contact the NCO project at http://nco.sf.net or write to
   Charlie Zender
   Department of Earth System Science
   University of California at Irvine
   Irvine, CA 92697-3100 */

/* Usage:
   ncap2 -O -D 1 -S ~/nco/data/ncap2.in ~/nco/data/in.nc ~/foo.nc
   ncap2 -O -D 1 -s two=one+two in.nc foo.nc */

/* temporary define --not for release */
// #define HAVE_CONFIG_H 1

#ifdef HAVE_CONFIG_H
#include "config.h" /* Autotools tokens */
#endif /* !HAVE_CONFIG_H */

/* Standard C headers */
#include <assert.h>  /* assert() debugging macro */
#include <math.h> /* sin cos cos sin 3.14159 */
#include <stdio.h> /* stderr, FILE, NULL, etc. */
#include <stdlib.h> /* atof, atoi, malloc, getopt */
#include <string.h> /* strcmp. . . */
#include <sys/stat.h> /* stat() */
#include <time.h> /* machine time */
#include <unistd.h> /* all sorts of POSIX stuff */
#include <string>
/* GNU getopt() is independent system header on FREEBSD, LINUX, LINUXALPHA, LINUXAMD, LINUXARM, WIN32
   AT&T getopt() is in unistd.h or stdlib.h on AIX, CRAY, NECSX, SUNMP, SUN4SOL2
   fxm: I'm not sure what ALPHA and SGI do */
#ifndef HAVE_GETOPT_LONG
#include "nco_getopt.h"
#else /* !NEED_GETOPT_LONG */ 
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* !HAVE_GETOPT_H */ 
#endif /* HAVE_GETOPT_LONG */

/* 3rd party vendors */
#include <netcdf.h> /* netCDF definitions and C library */
#include "nco_netcdf.h"  /* NCO wrappers for libnetcdf.a */

/* Personal headers */
/* #define MAIN_PROGRAM_FILE MUST precede #include libnco.h */
#define MAIN_PROGRAM_FILE
#include "ncap2.hh" /* netCDF arithmetic processor-specific definitions (symbol table, ...) */
#include "libnco++.hh" /* netCDF Operator (NCO) C++ library */
#include "libnco.h" /* netCDF Operator (NCO) library */

/* Global variables */
size_t ncap_ncl_dpt_crr=0UL; /* [nbr] Depth of current #include file (incremented in ncap_lex.l) */
size_t *ncap_ln_nbr_crr; /* [cnt] Line number (incremented in ncap_lex.l) */
char **ncap_fl_spt_glb=NULL; /* [fl] Script file */

int 
main(int argc,char **argv)
{
  FILE *yyin; /* file handle used to check file existance */
  int parse_antlr(prs_sct*,char*,char*);

  /* fxm TODO nco652 */
  double rnd_nbr(double);
  
  nco_bool EXCLUDE_INPUT_LIST=False; /* Option c */
  nco_bool EXTRACT_ALL_COORDINATES=False; /* Option c */
  nco_bool EXTRACT_ASSOCIATED_COORDINATES=True; /* Option C */
  nco_bool FILE_RETRIEVED_FROM_REMOTE_LOCATION;
  nco_bool FL_LST_IN_FROM_STDIN=False; /* [flg] fl_lst_in comes from stdin */
  nco_bool FORCE_APPEND=False; /* Option A */
  nco_bool FORCE_OVERWRITE=False; /* Option O */
  nco_bool FORTRAN_IDX_CNV=False; /* Option F */
  nco_bool HISTORY_APPEND=True; /* Option h */
  nco_bool CNV_CCM_CCSM_CF;
  nco_bool PRN_FNC_TBL=False; /* Option f */  
  nco_bool PROCESS_ALL_VARS=True; /* Option v */  
  nco_bool REMOVE_REMOTE_FILES_AFTER_PROCESSING=True; /* Option R */
  
  char **fl_lst_abb=NULL; /* Option n */
  char **fl_lst_in;
  char **var_lst_in=NULL_CEWI;
  char *cmd_ln;
  char *fl_in=NULL;
  char *fl_out=NULL; /* Option o */
  char *fl_out_tmp;
  char *fl_pth=NULL; /* Option p */
  char *fl_pth_lcl=NULL; /* Option l */
  char *fl_spt_usr=NULL; /* Option s */
  char *lmt_arg[NC_MAX_DIMS];
  char *opt_crr=NULL; /* [sng] String representation of current long-option name */
#define NCAP_SPT_NBR_MAX 100
  char *spt_arg[NCAP_SPT_NBR_MAX]; /* fxm: Arbitrary size, should be dynamic */
  char *spt_arg_cat=NULL; /* [sng] User-specified script */
  char *time_bfr_srt;

  const char * const CVS_Id="$Id: ncap2.cc,v 1.7 2006-02-25 22:29:14 zender Exp $"; 
  const char * const CVS_Revision="$Revision: 1.7 $";
  const char * const opt_sht_lst="4ACcD:d:Ffhl:n:Oo:p:Rrs:S:vx-:"; /* [sng] Single letter command line options */

  dmn_sct **dmn_in=NULL_CEWI;  /* [lst] Dimensions in input file */
  dmn_sct **dmn_out=NULL_CEWI; /* [lst] Dimensions written to output file */
  dmn_sct *dmn_new;
  dmn_sct *dmn_item;

  // template lists
  NcapVector<sym_sct*> sym_vtr;
  NcapVector<dmn_sct*> dmn_in_vtr;  
  NcapVector<dmn_sct*> dmn_out_vtr;  
  
  // Holder for attributtes and vectors
  NcapVarVector var_vtr;

  extern char *optarg;
  extern int optind;

  
  /* Math float prototypes required by AIX, Solaris, but not by Linux, IRIX */
  /* Basic math: acos, asin, atan, cos, exp, fabs, log, log10, sin, sqrt, tan */
  
  /* GNU g++ barfs at these float declartions -- remove if g++ used */

#ifndef __GNUG__
  
  extern float acosf(float);
  extern float asinf(float);
  extern float atanf(float);
  extern float cosf(float);
  extern float expf(float);
  extern float fabsf(float); /* 20040629: Only AIX may need this */
  extern float logf(float);
  extern float log10f(float);
  extern float rnd_nbrf(float);
  extern float sinf(float);
  extern float sqrtf(float);
  extern float tanf(float);
  
  /* Advanced math: erf, erfc, gamma */
  extern float erff(float);
  extern float erfcf(float);
  extern float gammaf(float);

  /* Hyperbolic trigonometric: acosh, asinh, atanh, cosh, sinh, tanh */
  extern float acoshf(float);
  extern float asinhf(float);
  extern float atanhf(float);
  extern float coshf(float);
  extern float sinhf(float);
  extern float tanhf(float);
  
  /* Basic Rounding: ceil, floor */
  extern float ceilf(float);
  extern float floorf(float);
  
  /* Advanced Rounding: nearbyint, rint, round, trunc */
  extern float nearbyintf(float);
  extern float rintf(float);
  extern float roundf(float);
  extern float truncf(float);
#endif

  int abb_arg_nbr=0;
  int fl_nbr=0;
  int fl_out_fmt=NC_FORMAT_CLASSIC; /* [enm] Output file format */
  int fll_md_old; /* [enm] Old fill mode */
  int idx;
  int in_id;  
  int jdx;
  int lmt_nbr=0; /* Option d. NB: lmt_nbr gets incremented */
  int nbr_dmn_ass=int_CEWI;/* Number of dimensions in temporary list */
  int nbr_dmn_in=int_CEWI; /* Number of dimensions in dim_in */
  int nbr_dmn_out=0; /* [nbr] Number of dimensions in list dmn_out */
  int nbr_lst_a=0; /* size of xtr_lst_a */
  int nbr_spt=0; /* Option s. NB: nbr_spt gets incremented */
  int nbr_var_fix; /* nbr_var_fix gets incremented */
  int nbr_var_fl;/* number of vars in a file */
  int nbr_var_prc; /* nbr_var_prc gets incremented */
  int nbr_xtr=0; /* nbr_xtr will not otherwise be set for -c with no -v */
  int opt;
  int out_id;  
  int rcd=NC_NOERR; /* [rcd] Return code */
  int var_id;
  
  int sym_tbl_nbr; /* [nbr] Size of symbol table */
  
  lmt_sct **lmt=NULL_CEWI;
  
  nm_id_sct *dmn_lst=NULL;
  nm_id_sct *xtr_lst=NULL; /* Non-processed variables to copy to OUTPUT */
  nm_id_sct *xtr_lst_a=NULL; /* Initialize to ALL variables in OUTPUT file */
  
  size_t sng_lng;
  size_t spt_arg_lng=size_t_CEWI;

  time_t time_crr_time_t;
  
  var_sct **var;
  var_sct **var_fix;
  var_sct **var_fix_out;
  var_sct **var_out;
  var_sct **var_prc;
  var_sct **var_prc_out;

  aed_sct att_item; //Used to convert atts in vector to normal form  
  size_t att_char_posn;  
  char att_sng[3*NC_MAX_NAME];
  

  prs_sct prs_arg; /* [sct] Global information required in parser routines */
  
  static struct option opt_lng[]=
    { /* Structure ordered by short option key if possible */
      /* Long options with no argument, no short option counterpart */
      /* Long options with argument, no short option counterpart */
      {"fl_fmt",required_argument,0,0},
      {"file_format",required_argument,0,0},
      /* Long options with short counterparts */
      {"4",no_argument,0,'4'},
      {"64bit",no_argument,0,'4'},
      {"netcdf4",no_argument,0,'4'},
      {"append",no_argument,0,'A'},
      {"coords",no_argument,0,'c'},
      {"crd",no_argument,0,'c'},
      {"no-coords",no_argument,0,'C'},
      {"no-crd",no_argument,0,'C'},
      {"debug",required_argument,0,'D'},
      {"dbg_lvl",required_argument,0,'D'},
      {"dimension",required_argument,0,'d'},
      {"dmn",required_argument,0,'d'},
      {"fnc_tbl",no_argument,0,'f'},
      {"prn_fnc_tbl",no_argument,0,'f'},
      {"ftn",no_argument,0,'F'},
      {"history",no_argument,0,'h'},
      {"hst",no_argument,0,'h'},
      {"local",required_argument,0,'l'},
      {"lcl",required_argument,0,'l'},
      {"nintap",required_argument,0,'n'},
      {"overwrite",no_argument,0,'O'},
      {"ovr",no_argument,0,'O'},
      {"output",required_argument,0,'o'},
      {"fl_out",required_argument,0,'o'},
      {"path",required_argument,0,'p'},
      {"retain",no_argument,0,'R'},
      {"rtn",no_argument,0,'R'},
      {"revision",no_argument,0,'r'},
      {"file",required_argument,0,'S'},
      {"script-file",required_argument,0,'S'},
      {"fl_spt",required_argument,0,'S'},
      {"spt",required_argument,0,'s'},
      {"script",required_argument,0,'s'},
      {"units",no_argument,0,'u'},
      {"variable",no_argument,0,'v'},
      {"version",no_argument,0,'r'},
      {"vrs",no_argument,0,'r'},
      {"exclude",no_argument,0,'x'},
      {"xcl",no_argument,0,'x'},
      {"help",no_argument,0,'?'},
      {0,0,0,0}
    }; /* end opt_lng */
  int opt_idx=0; /* Index of current long option into opt_lng array */
  
  /* Start clock and save command line */ 
  cmd_ln=nco_cmd_ln_sng(argc,argv);
  time_crr_time_t=time((time_t *)NULL);
  time_bfr_srt=ctime(&time_crr_time_t); time_bfr_srt=time_bfr_srt; /* Avoid compiler warning until variable is used for something */
  
  /* Get program name and set program enum (e.g., prg=ncra) */
  prg_nm=prg_prs(argv[0],&prg);
  
  /* Parse command line arguments */
  while(1){
    /* getopt_long_only() allows one dash to prefix long options */
    opt=getopt_long(argc,argv,opt_sht_lst,opt_lng,&opt_idx);
    /* NB: access to opt_crr is only valid when long_opt is detected */
    if(opt == EOF) break; /* Parse positional arguments once getopt_long() returns EOF */
    opt_crr=(char *)strdup(opt_lng[opt_idx].name);

    /* Process long options without short option counterparts */
    if(opt == 0){
      if(!strcmp(opt_crr,"fl_fmt") || !strcmp(opt_crr,"file_format")) rcd=nco_create_mode_prs(optarg,&fl_out_fmt);
    } /* opt != 0 */
    /* Process short options */
    switch(opt){
    case 0: /* Long options have already been processed, return */
      break;
    case '4': /* [flg] Catch-all to prescribe output storage format */
      if(!strcmp(opt_crr,"64bit")) fl_out_fmt=NC_FORMAT_64BIT; else fl_out_fmt=NC_FORMAT_NETCDF4; 
      break;
    case 'A': /* Toggle FORCE_APPEND */
      FORCE_APPEND=!FORCE_APPEND;
      break;
    case 'C': /* Extract all coordinates associated with extracted variables? */
      EXTRACT_ASSOCIATED_COORDINATES=False;
      break;
    case 'c':
      EXTRACT_ALL_COORDINATES=True;
      break;
    case 'D': /* Debugging level. Default is 0. */
      dbg_lvl=(unsigned short)strtol(optarg,(char **)NULL,10);
      break;
    case 'd': /* Copy argument for later processing */
      lmt_arg[lmt_nbr]=(char *)strdup(optarg);
      lmt_nbr++;
      break;
    case 'F': /* Toggle index convention. Default is 0-based arrays (C-style). */
      FORTRAN_IDX_CNV=!FORTRAN_IDX_CNV;
      break;
    case 'f': /* Print function table */
      PRN_FNC_TBL=True;
      break;
    case 'h': /* Toggle appending to history global attribute */
      HISTORY_APPEND=!HISTORY_APPEND;
      break;
    case 'l': /* Local path prefix for files retrieved from remote file system */
      fl_pth_lcl=(char *)strdup(optarg);
      break;
    case 'n': /* NINTAP-style abbreviation of files to process */
      /* Currently not used in ncap but should be to allow processing multiple input files by same script */
      (void)fprintf(stderr,"%s: ERROR %s does not currently implement -n option\n",prg_nm_get(),prg_nm_get());
      fl_lst_abb=lst_prs_2D(optarg,",",&abb_arg_nbr);
      if(abb_arg_nbr < 1 || abb_arg_nbr > 3){
	(void)fprintf(stderr,"%s: ERROR Incorrect abbreviation for file list\n",prg_nm);
	(void)nco_usg_prn();
	nco_exit(EXIT_FAILURE);
      } /* end if */
      break;
    case 'O': /* Toggle FORCE_OVERWRITE */
      FORCE_OVERWRITE=!FORCE_OVERWRITE;
      break;
    case 'o': /* Name of output file */
      fl_out=(char *)strdup(optarg);
      break;
    case 'p': /* Common file path */
      fl_pth=(char *)strdup(optarg);
      break;
    case 'R': /* Toggle removal of remotely-retrieved-files. Default is True. */
      REMOVE_REMOTE_FILES_AFTER_PROCESSING=!REMOVE_REMOTE_FILES_AFTER_PROCESSING;
      break;
    case 'r': /* Print CVS program information and copyright notice */
      (void)copyright_prn(CVS_Id,CVS_Revision);
      (void)nco_lbr_vrs_prn();
      nco_exit(EXIT_SUCCESS);
      break;
    case 's': /* Copy command script for later processing */
      spt_arg[nbr_spt++]=(char *)strdup(optarg);
      if(nbr_spt == NCAP_SPT_NBR_MAX-1) (void)fprintf(stderr,"%s: WARNING No more than %d script arguments allowed. TODO #24\n",prg_nm_get(),NCAP_SPT_NBR_MAX);
      break;
    case 'S': /* Read command script from file rather than from command line */
      fl_spt_usr=(char *)strdup(optarg);
      break;
    case 'v': /* Variables to extract/exclude */
      PROCESS_ALL_VARS=False;
      nbr_xtr=0;
      break;
    case 'x': /* Exclude rather than extract variables specified with -v */
      EXCLUDE_INPUT_LIST=True;
      if(EXCLUDE_INPUT_LIST) (void)fprintf(stderr,"%s: ERROR %s does not currently implement -x option\n",prg_nm_get(),prg_nm_get());
      nco_exit(EXIT_FAILURE);
      break;
    case '?': /* Print proper usage */
      (void)nco_usg_prn();
      nco_exit(EXIT_SUCCESS);
      break;
    case '-': /* Long options are not allowed */
      (void)fprintf(stderr,"%s: ERROR Long options are not available in this build. Use single letter options instead.\n",prg_nm_get());
      nco_exit(EXIT_FAILURE);
      break;
    default: /* Print proper usage */
      (void)nco_usg_prn();
      nco_exit(EXIT_FAILURE);
      break;
    } /* end switch */
    if(opt_crr != NULL) opt_crr=(char *)nco_free(opt_crr);
  } /* end while loop */
  
  /* Append ";\n" to command-script arguments, then concatenate them */
  for(idx=0;idx<nbr_spt;idx++){
    sng_lng=strlen(spt_arg[idx]);
    if(idx == 0){
      spt_arg_cat=(char *)nco_malloc(sng_lng+3);
      strcpy(spt_arg_cat,spt_arg[idx]);
      strcat(spt_arg_cat,";\n");
      spt_arg_lng=sng_lng+3;
    }else{
      spt_arg_lng+=sng_lng+2;
      spt_arg_cat=(char *)nco_realloc(spt_arg_cat,spt_arg_lng);
      strcat(spt_arg_cat,spt_arg[idx]);
      strcat(spt_arg_cat,";\n");
    } /* end else */
  } /* end if */    

  
  /* Create function table */
  sym_tbl_nbr= /* fxm: Make this dynamic */
    +12 /* Basic math: acos, asin, atan, cos, exp, fabs, log, log10, rnd_nbr, sin, sqrt, tan */
    +1 /* Basic math synonyms: ln */
    +6 /* Hyperbolic trigonometric: acosh, asinh, atanh, cosh, sinh, tanh */
    +2 /* Basic Rounding: ceil, floor */
    +4 /* Advanced Rounding: nearbyint, rint, round, trunc */
    +3 /* Advanced math: erf, erfc, gamma */
    ;
  /* Basic math: acos, asin, atan, cos, exp, log, log10, rnd_nbr, sin, sqrt, tan */

  sym_vtr.push(ncap_sym_init("acos",acos,acosf));  
  sym_vtr.push(ncap_sym_init("asin",asin,asinf));
  sym_vtr.push(ncap_sym_init("atan",atan,atanf));
  sym_vtr.push(ncap_sym_init("cos",cos,cosf));  
  sym_vtr.push(ncap_sym_init("exp",exp,expf));
  sym_vtr.push(ncap_sym_init("fabs",fabs,fabsf));
  sym_vtr.push(ncap_sym_init("log",log,logf));
  sym_vtr.push(ncap_sym_init("log10",log10,log10f));
  //sym_vtr.push(ncap_sym_init("rnd_nbr",rnd_nbr,rnd_nbrf));
  sym_vtr.push(ncap_sym_init("sin",sin,sinf));
  sym_vtr.push(ncap_sym_init("sqrt",sqrt,sqrtf));
  sym_vtr.push(ncap_sym_init("tan",tan,tanf));

  /* Basic math synonyms: ln */
  sym_vtr.push(ncap_sym_init("ln",log,logf)); /* ln() is synonym for log() */
  
  /* Basic Rounding: ceil, fl<oor */
  sym_vtr.push(ncap_sym_init("ceil",ceil,ceilf)); /* Round up to nearest integer */
  sym_vtr.push(ncap_sym_init("floor",floor,floorf)); /* Round down to nearest integer */
  
  /* fxm: Change whole function symbol table section to autotools format #if HAVE_ERF ... */

  /* Advanced math: erf, erfc, gamma
     LINUX*, MACOSX*, and SUN* provide these functions with C89
     20020122 and 20020422: AIX, CRAY, SGI*, WIN32 do not define erff(), erfcf(), gammaf() with C89
     20050610: C99 mandates support for erf(), erfc(), tgamma()
     Eventually users without C99 will forego ncap */
#if defined(LINUX) || defined(LINUXAMD64)  || defined(MACOSX)
  sym_vtr.push(ncap_sym_init("erf",erf,erff));
  sym_vtr.push(ncap_sym_init("erfc",erfc,erfcf));
  sym_vtr.push(ncap_sym_init("gamma",tgamma,tgammaf));
#endif /* !LINUX */

  /* Hyperbolic trigonometric: acosh, asinh, atanh, cosh, sinh, tanh
     20020703: AIX, SGI*, WIN32 do not define acoshf, asinhf, atanhf
     20050610: C99 mandates support for acosh(), asinh(), atanh(), cosh(), sinh(), tanh()
     Eventually users without C99 will forego ncap */
#if defined(LINUX) || defined(LINUXAMD64)
  sym_vtr.push(ncap_sym_init("acosh",acosh,acoshf));
  sym_vtr.push(ncap_sym_init("asinh",asinh,asinhf));
  sym_vtr.push(ncap_sym_init("atanh",atanh,atanhf));
  sym_vtr.push(ncap_sym_init("cosh",cosh,coshf));
  sym_vtr.push(ncap_sym_init("sinh",sinh,sinhf));
  sym_vtr.push(ncap_sym_init("tanh",tanh,tanhf));
#endif /* !LINUX */
  
  /* 20020703: AIX, MACOSX, SGI*, WIN32 do not define rintf
     Only LINUX* supplies all of these and I do not care about them enough
     to activate them on LINUX* but not on MACOSX* and SUN* */
 /* Advanced Rounding: nearbyint, rint, round, trunc */
  /* Advanced Rounding: nearbyint, round, trunc */
  /* sym_vtr.push(ncap_sym_init("nearbyint",nearbyint,nearbyintf)); *//* Round to integer value in floating point format using current rounding direction, do not raise inexact exceptions */
  /* sym_vtr.push(ncap_sym_init("round",round,roundf)); *//* Round to nearest integer away from zero */
  /* sym_vtr.push(ncap_sym_init("trunc",trunc,truncf)); *//* Round to nearest integer not larger in absolute value */
  /* sym_vtr.push(ncap_sym_init("rint",rint,rintf)); *//* Round to integer value in floating point format using current rounding direction, raise inexact exceptions */
   
  if(PRN_FNC_TBL){
    /* ncap TODO #43: alphabetize this list */ 
    (void)fprintf(stdout,"Maths functions available in %s:\n",prg_nm_get());
    (void)fprintf(stdout,"Name\tFloat\tDouble\n"); 
    for(idx=0;idx<sym_vtr.size();idx++)
      (void)fprintf(stdout,"%s\t%c\t%c\n",sym_vtr[idx]->nm, (sym_vtr[idx]->fnc_flt ? 'y' : 'n'),(sym_vtr[idx]->fnc_dbl ? 'y' : 'n'));
    nco_exit(EXIT_SUCCESS);
  } /* end if PRN_FNC_TBL */
  
  /* Process positional arguments and fill in filenames */
  fl_lst_in=nco_fl_lst_mk(argv,argc,optind,&fl_nbr,&fl_out,&FL_LST_IN_FROM_STDIN);
  
  /* Make uniform list of user-specified dimension limits */
  if(lmt_nbr > 0) lmt=nco_lmt_prs(lmt_nbr,lmt_arg);
  
  /* Parse filename */
  fl_in=nco_fl_nm_prs(fl_in,0,&fl_nbr,fl_lst_in,abb_arg_nbr,fl_lst_abb,fl_pth);
  /* Make sure file is on local system and is readable or die trying */
  fl_in=nco_fl_mk_lcl(fl_in,fl_pth_lcl,&FILE_RETRIEVED_FROM_REMOTE_LOCATION);
  /* Open file for reading */
  rcd=nco_open(fl_in,NC_NOWRITE,&in_id);
  
  /* Form list of all dimensions in file */  
  dmn_lst=nco_dmn_lst(in_id,&nbr_dmn_in);
  
  //dmn_in=(dmn_sct **)nco_malloc(nbr_dmn_in*sizeof(dmn_sct *));
  for(idx=0;idx<nbr_dmn_in;idx++) 
    dmn_in_vtr.push(nco_dmn_fll(in_id,dmn_lst[idx].id,dmn_lst[idx].nm));
  dmn_in=dmn_in_vtr.ptr(0);

  
  /* Merge hyperslab limit information into dimension structures */
  if(lmt_nbr > 0) (void)nco_dmn_lmt_mrg(dmn_in,nbr_dmn_in,lmt,lmt_nbr);
  
  /* Open output file */
  fl_out_tmp=nco_fl_out_open(fl_out,FORCE_APPEND,FORCE_OVERWRITE,fl_out_fmt,&out_id);
  
  /* Copy global attributes */
  (void)nco_att_cpy(in_id,out_id,NC_GLOBAL,NC_GLOBAL,True);
  
  /* Catenate time-stamped command line to "history" global attribute */
  if(HISTORY_APPEND) (void)nco_hst_att_cat(out_id,cmd_ln);
  
  (void)nco_enddef(out_id);

  
  /* Set arguments for  script execution */
  prs_arg.fl_in=fl_in; /* [sng] Input data file */
  prs_arg.in_id=in_id; /* [id] Input data file ID */
  prs_arg.fl_out=fl_out; /* [sng] Output data file */
  prs_arg.out_id=out_id; /* [id] Output data file ID */
  
  prs_arg.ptr_dmn_in_vtr=&dmn_in_vtr;
  prs_arg.ptr_dmn_out_vtr=&dmn_out_vtr;
  prs_arg.ptr_sym_vtr=&sym_vtr;
  prs_arg.ptr_var_vtr=&var_vtr;

  prs_arg.ntl_scn=False;   //[flg] Initial scan of script */

    
  if(fl_spt_usr == NULL){
    /* No script file specified, look for command-line scripts */
    if(nbr_spt == 0){
       (void)fprintf(stderr,"%s: ERROR no script file or command line scripts specified\n",prg_nm_get());
       (void)fprintf(stderr,"%s: HINT Use, e.g., -s \"foo=bar\"\n",prg_nm_get());
       nco_exit(EXIT_FAILURE);
      } /* end if */
      
     /* Print all command-line scripts */
     if(dbg_lvl_get() > 0){
       for(idx=0;idx<nbr_spt;idx++) (void)fprintf(stderr,"spt_arg[%d] = %s\n",idx,spt_arg[idx]);
     } /* endif debug */
      
     /* Parse command line scripts */
     fl_spt_usr=(char *)strdup("Command-line script");
     }else{ /* ...endif command-line scripts, begin script file... */
      /* Open script file for reading */
      if((yyin=fopen(fl_spt_usr,"r")) == NULL){
        (void)fprintf(stderr,"%s: ERROR Unable to open script file %s\n",prg_nm_get(),fl_spt_usr);
        nco_exit(EXIT_FAILURE);
      } /* end if */
      fclose(yyin); 
    } /* end else script file */
    
    /* Invoke ANTLR parser */
     
    rcd=parse_antlr(&prs_arg,fl_spt_usr,spt_arg_cat);

    
    /* Tidy up */  
    fl_spt_usr=(char*)nco_free(fl_spt_usr);

  
  /* Get number of variables in output file */
  rcd=nco_inq(out_id,(int *)NULL,&nbr_var_fl,(int *)NULL,(int*)NULL);
  
  /* Make list of all new variables in output_file */  
  xtr_lst_a=nco_var_lst_mk(out_id,nbr_var_fl,var_lst_in,False,&nbr_lst_a);
  
  if(PROCESS_ALL_VARS){
    /* Get number of variables in input file */
    rcd=nco_inq(in_id,(int *)NULL,&nbr_var_fl,(int *)NULL,(int *)NULL);
    
    /* Form initial list of all variables in input file */
    xtr_lst=nco_var_lst_mk(in_id,nbr_var_fl,var_lst_in,False,&nbr_xtr);
  }else{
    /* Make list of variables of new attributes whose parent variable is only in input file */
    xtr_lst=nco_att_lst_mk(in_id,out_id,var_vtr,&nbr_xtr);
  } /* endif */
  
    /* Find dimensions associated with xtr_lst */
    /* Write to O only new dims
       Add apropriate coordinate variables to extraction list 
       options -c      -process all cordinates 
       i.e., add coordinates to var list 
       Also add their dims
       
       options --none   -process associated co-ords
       loop though dim_out and append to var list
       
       options -C         no co-ordinates   Do nothing */
  
    /* Subtract list A again */
    /* Finally extract variables on list */
  
  /* Subtract list A */
  if(nbr_lst_a > 0) xtr_lst=nco_var_lst_sub(xtr_lst,&nbr_xtr,xtr_lst_a,nbr_lst_a);
  
  /* Put file in define mode to allow metadata writing */
  (void)nco_redef(out_id);
  
  /* Free current list of all dimensions in input file */
  dmn_lst=nco_nm_id_lst_free(dmn_lst,nbr_dmn_in);

  /* Make list of dimensions of variables in xtr_lst */
  if(nbr_xtr > 0) dmn_lst=nco_dmn_lst_ass_var(in_id,xtr_lst,nbr_xtr,&nbr_dmn_ass);
  
  /* Find and add any new dimensions to output */
  for(idx=0;idx<nbr_dmn_ass;idx++){
    dmn_item=dmn_out_vtr.find(dmn_lst[idx].nm);
    if(dmn_item != NULL) continue;    
    dmn_item=dmn_in_vtr.find(dmn_lst[idx].nm);
    if(dmn_item == NULL) continue;
    dmn_new=nco_dmn_dpl(dmn_item);
    (void)nco_dmn_dfn(fl_out,out_id,&dmn_new,1);
    (void)nco_dmn_xrf(dmn_new,dmn_item);
    (void)dmn_out_vtr.push(dmn_new);
   }  /* end loop over idx */
  
  /* Free current list of all dimensions in input file */
  dmn_lst=nco_nm_id_lst_free(dmn_lst,nbr_dmn_ass);

  /* Dimensions for manually specified extracted variables are now defined in output file
     Add coordinate variables to extraction list
     If EXTRACT_ALL_COORDINATES then write associated dimension to output */


 if(EXTRACT_ASSOCIATED_COORDINATES){
    for(idx=0;idx<dmn_in_vtr.size();idx++){
      if(!dmn_in_vtr[idx]->is_crd_dmn) continue;
      
      if(EXTRACT_ALL_COORDINATES && !dmn_in_vtr[idx]->xrf){
	/* Add dimensions to output list dmn_out */
	dmn_item=dmn_in_vtr[idx];
	dmn_new=nco_dmn_dpl(dmn_item);
	(void)nco_dmn_xrf(dmn_new,dmn_item);
	/* Write dimension to output */
	(void)nco_dmn_dfn(fl_out,out_id,&dmn_new,1);
	(void)dmn_out_vtr.push(dmn_new);
      } /* end if */
      /* Add coordinate variable to extraction list, dimension has already been output */
      if(dmn_in_vtr[idx]->xrf){
	for(jdx=0;jdx<nbr_xtr;jdx++)
	  if(!strcmp(xtr_lst[jdx].nm,dmn_in_vtr[idx]->nm)) break;

	if(jdx != nbr_xtr) continue;
	/* If coordinate is not on list then add it to extraction list */
	xtr_lst=(nm_id_sct *)nco_realloc(xtr_lst,(nbr_xtr+1)*sizeof(nm_id_sct));
	xtr_lst[nbr_xtr].nm=(char *)strdup(dmn_in_vtr[idx]->nm);
	xtr_lst[nbr_xtr++].id=dmn_in_vtr[idx]->cid;
      } /* endif */
    } /* end loop over idx */	      
  } /* end if */ 
 
 
  /* Subtract list A again (it may contain re-defined coordinates) */
  if(nbr_xtr > 0) xtr_lst=nco_var_lst_sub(xtr_lst,&nbr_xtr,xtr_lst_a,nbr_lst_a);
  
  /* Sort extraction list for faster I/O */
  if(nbr_xtr > 1) xtr_lst=nco_lst_srt_nm_id(xtr_lst,nbr_xtr,False);
  
  /* Is this an CCM/CCSM/CF-format history tape? */
  CNV_CCM_CCSM_CF=nco_cnv_ccm_ccsm_cf_inq(in_id);
  
  /* Write "fixed" variables */
  var=(var_sct **)nco_malloc(nbr_xtr*sizeof(var_sct *));
  var_out=(var_sct **)nco_malloc(nbr_xtr*sizeof(var_sct *));
  for(idx=0;idx<nbr_xtr;idx++){
    var[idx]=nco_var_fll(in_id,xtr_lst[idx].id,xtr_lst[idx].nm,dmn_in_vtr.ptr(0),dmn_in_vtr.size());
    var_out[idx]=nco_var_dpl(var[idx]);
    (void)nco_xrf_var(var[idx],var_out[idx]);
    (void)nco_xrf_dmn(var_out[idx]);
  } /* end loop over idx */
  

  /* NB: ncap is not well-suited for nco_var_lst_dvd() */
  /* Divide variable lists into lists of fixed variables and variables to be processed */
  (void)nco_var_lst_dvd(var,var_out,nbr_xtr,CNV_CCM_CCSM_CF,nco_pck_plc_nil,nco_pck_map_nil,(dmn_sct **)NULL,(int)0,&var_fix,&var_fix_out,&nbr_var_fix,&var_prc,&var_prc_out,&nbr_var_prc);
  
  /* csz: Why not call this with var_fix? */
  /* Define non-processed vars */
  (void)nco_var_dfn(in_id,fl_out,out_id,var_out,nbr_xtr,(dmn_sct **)NULL,(int)0,nco_pck_plc_nil,nco_pck_map_nil);
  

  /* Write out new attributes possibly overwriting old ones */
  for(idx=0;idx<var_vtr.size();idx++){

    // Check if attrribute
    if( var_vtr[idx]->type != ncap_att) continue;
    att_char_posn=var_vtr[idx]->s_va_nm.find("@");
    if(att_char_posn == std::string::npos) continue;
    (void)strcpy(att_sng,var_vtr[idx]->s_va_nm.c_str());
    att_sng[att_char_posn]='\0';

    att_item.att_nm=(att_sng+att_char_posn+1);
    att_item.var_nm=att_sng;
    att_item.sz=var_vtr[idx]->var->sz;
    att_item.type=var_vtr[idx]->var->type;
    att_item.val=var_vtr[idx]->var->val;
    att_item.mode=aed_overwrite;

    if(!strcmp(att_item.var_nm,"global")) 
       var_id=NC_GLOBAL;
    else {
       rcd=nco_inq_varid_flg(out_id,att_item.var_nm,&var_id);
       if(rcd != NC_NOERR)  continue;
    }
    // Check size;
      if(att_item.sz > NC_MAX_ATTRS ){ 
	(void)fprintf(stdout,"%s: Attribute %s size %ld excceeds maximium %d\n",prg_nm_get(),att_item.att_nm,att_item.sz, NC_MAX_ATTRS );
	continue;
      }
    /* NB: These attributes should probably be written prior to last data mode */
    (void)nco_aed_prc(out_id,var_id,att_item);
    // delete NcapVar 
    delete var_vtr[idx];
   }/* end for */
  
  /* Turn off default filling behavior to enhance efficiency */
  rcd=nco_set_fill(out_id,NC_NOFILL,&fll_md_old);
  
  /* Take output file out of define mode */
  (void)nco_enddef(out_id);
  
  /* Copy non-processed vars */
  (void)nco_var_val_cpy(in_id,out_id,var_fix,nbr_var_fix);
  
  /* Close input netCDF file */
  rcd=nco_close(in_id);
  
  /* Remove local copy of file */
  if(FILE_RETRIEVED_FROM_REMOTE_LOCATION && REMOVE_REMOTE_FILES_AFTER_PROCESSING) (void)nco_fl_rm(fl_in);
  
  /* Close output file and move it from temporary to permanent location */
  (void)nco_fl_out_cls(fl_out,fl_out_tmp,out_id);
  
  /* ncap-unique memory */
  /* fxm: ncap-specific memory freeing instructions go here */
  /*
  for(idx=0;idx<sym_tbl_nbr;idx++){
    sym_tbl[idx]->nm=(char*)nco_free(sym_tbl[idx]->nm);
    sym_tbl[idx]=(sym_sct*)nco_free(sym_tbl[idx]);
    } 
    sym_tbl=(sym_sct **)nco_free(sym_tbl); */
  if(fl_spt_usr != NULL) fl_spt_usr=(char *)nco_free(fl_spt_usr);
  
  /* Free extraction lists */ 
  xtr_lst=nco_nm_id_lst_free(xtr_lst,nbr_xtr);
  xtr_lst_a=nco_nm_id_lst_free(xtr_lst_a,nbr_lst_a);

  /* Free command line algebraic arguments, if any */
  for(idx=0;idx<nbr_spt;idx++) spt_arg[idx]=(char *)nco_free(spt_arg[idx]);
  if(spt_arg_cat != NULL) spt_arg_cat=(char *)nco_free(spt_arg_cat);
  
  /* NCO-generic clean-up */
  /* Free individual strings/arrays */
  if(cmd_ln != NULL) cmd_ln=(char *)nco_free(cmd_ln);
  if(fl_in != NULL) fl_in=(char*)nco_free(fl_in);
  if(fl_out != NULL) fl_out=(char *)nco_free(fl_out);
  if(fl_out_tmp != NULL) fl_out_tmp=(char *)nco_free(fl_out_tmp);
  if(fl_pth != NULL) fl_pth=(char *)nco_free(fl_pth);
  if(fl_pth_lcl != NULL) fl_pth_lcl=(char *)nco_free(fl_pth_lcl);
  /* Free lists of strings */
  if(fl_lst_in != NULL && fl_lst_abb == NULL) fl_lst_in=nco_sng_lst_free(fl_lst_in,fl_nbr); 
  if(fl_lst_in != NULL && fl_lst_abb != NULL) fl_lst_in=nco_sng_lst_free(fl_lst_in,1);
  if(fl_lst_abb != NULL) fl_lst_abb=nco_sng_lst_free(fl_lst_abb,abb_arg_nbr);
  /* Free limits */
  for(idx=0;idx<lmt_nbr;idx++) lmt_arg[idx]=(char *)nco_free(lmt_arg[idx]);
  if(lmt_nbr > 0) lmt=nco_lmt_lst_free(lmt,lmt_nbr);
  /* Free dimension lists */
  if(nbr_dmn_in > 0) dmn_in=nco_dmn_lst_free(dmn_in,nbr_dmn_in);
  if(nbr_dmn_out > 0) dmn_out=nco_dmn_lst_free(dmn_out,nbr_dmn_out);
  /* Free variable lists */
  if(nbr_xtr > 0) var=nco_var_lst_free(var,nbr_xtr);
  if(nbr_xtr > 0) var_out=nco_var_lst_free(var_out,nbr_xtr);
  var_prc=(var_sct **)nco_free(var_prc);
  var_prc_out=(var_sct **)nco_free(var_prc_out);
  var_fix=(var_sct **)nco_free(var_fix);
  var_fix_out=(var_sct **)nco_free(var_fix_out);

  nco_exit_gracefully();
  return EXIT_SUCCESS;
} /* end main() */
