/*---------------------------------------------------------------
 Programmer(s): Daniel R. Reynolds @ SMU
 ----------------------------------------------------------------
 Copyright (c) 2013, Southern Methodist University.
 All rights reserved.
 For details, see the LICENSE file.
 ----------------------------------------------------------------
 This module contains the routines necessary to interface with 
 the ARKBANDPRE module and user-supplied Fortran routines. The 
 routines here call the generically named routines and provide 
 a standard interface to the C code of the ARKBANDPRE package.
 --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "farkode.h"
#include "farkbp.h"
#include <arkode/arkode_bandpre.h>

/*=============================================================*/

/* Fortran interface to C routine ARKBandPrecInit; see farkbp.h 
   for additional information */
void FARK_BPINIT(long int *N, long int *mu, 
		 long int *ml, int *ier)
{
  *ier = ARKBandPrecInit(ARK_arkodemem, *N, *mu, *ml);
  return;
}

/*=============================================================*/

/* Fortran interface to C routines ARKBandPrecGetWorkSpace and 
   ARKBandPrecGetNumRhsEvals; see farkbp.h for additional 
   information */
void FARK_BPOPT(long int *lenrwbp, long int *leniwbp, long int *nfebp)
{
  ARKBandPrecGetWorkSpace(ARK_arkodemem, lenrwbp, leniwbp);
  ARKBandPrecGetNumRhsEvals(ARK_arkodemem, nfebp);
  return;
}

/*===============================================================
   EOF
===============================================================*/
