/*---------------------------------------------------------------
 Programmer(s): Daniel R. Reynolds @ SMU
 ----------------------------------------------------------------
 Copyright (c) 2013, Southern Methodist University.
 All rights reserved.
 For details, see the LICENSE file.
 ----------------------------------------------------------------
 Fortran/C interface routines for ARKODE, for the case of a 
 user-supplied explicit stability routine.
 --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "farkode.h"
#include "arkode_impl.h"

/*=============================================================*/

/* Prototype of the Fortran routine */

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

  extern void FARK_EXPSTAB(realtype *Y, realtype *T, realtype *HSTAB, 
			   long int *IPAR, realtype *RPAR, int *IER);

#ifdef __cplusplus
}
#endif

/*=============================================================*/

/* Fortran interface to C routine ARKodeSetStabilityFn; see 
   farkode.h for further information */
void FARK_EXPSTABSET(int *flag, int *ier)
{
  if (*flag == 0) {
    *ier = ARKodeSetStabilityFn(ARK_arkodemem, NULL, NULL);
  } else {
    *ier = ARKodeSetStabilityFn(ARK_arkodemem, FARKExpStab, NULL);
  }
  return;
}

/*=============================================================*/

/* C interface to user-supplied fortran routine FARKEXPSTAB; see 
   farkode.h for further information */
int FARKExpStab(N_Vector y, realtype t, realtype *hstab, void *udata)
{
  int ier = 0;
  realtype *ydata;
  FARKUserData ARK_userdata;

  ydata = N_VGetArrayPointer(y);
  ARK_userdata = (FARKUserData) udata;

  FARK_EXPSTAB(ydata, &t, hstab, ARK_userdata->ipar, 
	       ARK_userdata->rpar, &ier);
  return(ier);
}

/*===============================================================
   EOF
===============================================================*/
