/*---------------------------------------------------------------
 Programmer(s): Daniel R. Reynolds @ SMU
 ----------------------------------------------------------------
 Copyright (c) 2013, Southern Methodist University.
 All rights reserved.
 For details, see the LICENSE file.
 ----------------------------------------------------------------
 This is the implementation file for the ARKSPBCG linear solver.
 --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <arkode/arkode_spbcgs.h>
#include "arkode_spils_impl.h"
#include "arkode_impl.h"

#include <sundials/sundials_spbcgs.h>
#include <sundials/sundials_math.h>


/* Constants */
#define ZERO RCONST(0.0)
#define ONE  RCONST(1.0)

/* ARKSPBCG linit, lsetup, lsolve, and lfree routines */
static int ARKSpbcgInit(ARKodeMem ark_mem);
static int ARKSpbcgSetup(ARKodeMem ark_mem, int convfail, 
			 N_Vector ypred, N_Vector fpred, 
			 booleantype *jcurPtr, N_Vector vtemp1,
			 N_Vector vtemp2, N_Vector vtemp3);
static int ARKSpbcgSolve(ARKodeMem ark_mem, N_Vector b, 
			 N_Vector weight, N_Vector ynow, 
			 N_Vector fnow);
static void ARKSpbcgFree(ARKodeMem ark_mem);

/* ARKSPBCG minit, msetup, msolve, and mfree routines */
static int ARKMassSpbcgInit(ARKodeMem ark_mem);
static int ARKMassSpbcgSetup(ARKodeMem ark_mem, N_Vector vtemp1, 
			     N_Vector vtemp2, N_Vector vtemp3);
static int ARKMassSpbcgSolve(ARKodeMem ark_mem, N_Vector b, 
			     N_Vector weight);
static void ARKMassSpbcgFree(ARKodeMem ark_mem);


/*---------------------------------------------------------------
 ARKSpbcg:

 This routine initializes the memory record and sets various 
 function fields specific to the Spbcg linear solver module. 
 ARKSpbcg first calls the existing lfree routine if this is not 
 NULL. It then sets the ark_linit, ark_lsetup, ark_lsolve, 
 ark_lfree fields in (*arkode_mem) to be ARKSpbcgInit, 
 ARKSpbcgSetup, ARKSpbcgSolve, and ARKSpbcgFree, respectively. 
 It allocates memory for a structure of type ARKSpilsMemRec and 
 sets the ark_lmem field in (*arkode_mem) to the address of 
 this structure. It sets setupNonNull in (*arkode_mem),
 and sets various fields in the ARKSpilsMemRec structure.
 Finally, ARKSpbcg allocates memory for ytemp and x, and calls
 SpbcgMalloc to allocate memory for the Spbcg solver.
---------------------------------------------------------------*/
int ARKSpbcg(void *arkode_mem, int pretype, int maxl)
{
  ARKodeMem ark_mem;
  ARKSpilsMem arkspils_mem;
  SpbcgMem spbcg_mem;
  int mxl;

  /* Return immediately if arkode_mem is NULL */
  if (arkode_mem == NULL) {
    arkProcessError(NULL, ARKSPILS_MEM_NULL, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_ARKMEM_NULL);
    return(ARKSPILS_MEM_NULL);
  }
  ark_mem = (ARKodeMem) arkode_mem;

  /* Check if N_VDotProd and N_VProd are present */
  if ((ark_mem->ark_tempv->ops->nvdotprod == NULL) ||
      (ark_mem->ark_tempv->ops->nvprod == NULL)) {
    arkProcessError(ark_mem, ARKSPILS_ILL_INPUT, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_BAD_NVECTOR);
    return(ARKSPILS_ILL_INPUT);
  }

  if (ark_mem->ark_lfree != NULL) ark_mem->ark_lfree(ark_mem);

  /* Set four main function fields in ark_mem */
  ark_mem->ark_linit  = ARKSpbcgInit;
  ark_mem->ark_lsetup = ARKSpbcgSetup;
  ark_mem->ark_lsolve = ARKSpbcgSolve;
  ark_mem->ark_lfree  = ARKSpbcgFree;
  ark_mem->ark_lsolve_type = 0;

  /* Get memory for ARKSpilsMemRec */
  arkspils_mem = NULL;
  arkspils_mem = (ARKSpilsMem) malloc(sizeof(struct ARKSpilsMemRec));
  if (arkspils_mem == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_MEM_FAIL);
    return(ARKSPILS_MEM_FAIL);
  }

  /* Set ILS type */
  arkspils_mem->s_type = SPILS_SPBCG;

  /* Set Spbcg parameters that have been passed in call sequence */
  arkspils_mem->s_pretype = pretype;
  mxl = arkspils_mem->s_maxl = (maxl <= 0) ? ARKSPILS_MAXL : maxl;

  /* Set defaults for Jacobian-related fields */
  arkspils_mem->s_jtimesDQ = TRUE;
  arkspils_mem->s_jtimes   = NULL;
  arkspils_mem->s_j_data   = NULL;

  /* Set defaults for preconditioner-related fields */
  arkspils_mem->s_pset   = NULL;
  arkspils_mem->s_psolve = NULL;
  arkspils_mem->s_pfree  = NULL;
  arkspils_mem->s_P_data = ark_mem->ark_user_data;

  /* Initialize counters */
  arkspils_mem->s_npe = arkspils_mem->s_nli = 0;
  arkspils_mem->s_nps = arkspils_mem->s_ncfl = 0;
  arkspils_mem->s_nstlpre = arkspils_mem->s_njtimes = 0;
  arkspils_mem->s_nfes = 0;

  /* Set default values for the rest of the Spbcg parameters */
  arkspils_mem->s_eplifac = ARKSPILS_EPLIN;
  arkspils_mem->s_last_flag = ARKSPILS_SUCCESS;
  ark_mem->ark_setupNonNull = FALSE;

  /* Check for legal pretype */ 
  if ((pretype != PREC_NONE) && (pretype != PREC_LEFT) &&
      (pretype != PREC_RIGHT) && (pretype != PREC_BOTH)) {
    arkProcessError(ark_mem, ARKSPILS_ILL_INPUT, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_BAD_PRETYPE);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_ILL_INPUT);
  }

  /* Allocate memory for ytemp and x */
  arkspils_mem->s_ytemp = N_VClone(ark_mem->ark_tempv);
  if (arkspils_mem->s_ytemp == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_MEM_FAIL);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }

  arkspils_mem->s_x = N_VClone(ark_mem->ark_tempv);
  if (arkspils_mem->s_x == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_MEM_FAIL);
    N_VDestroy(arkspils_mem->s_ytemp);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }

  /* Compute sqrtN from a dot product */
  N_VConst(ONE, arkspils_mem->s_ytemp);
  arkspils_mem->s_sqrtN = SUNRsqrt(N_VDotProd(arkspils_mem->s_ytemp, 
					   arkspils_mem->s_ytemp));

  /* Call SpbcgMalloc to allocate workspace for Spbcg */
  spbcg_mem = NULL;
  spbcg_mem = SpbcgMalloc(mxl, ark_mem->ark_tempv);
  if (spbcg_mem == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKSpbcg", MSGS_MEM_FAIL);
    N_VDestroy(arkspils_mem->s_ytemp);
    N_VDestroy(arkspils_mem->s_x);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }
  
  /* Attach SPBCG memory to spils memory structure */
  arkspils_mem->s_spils_mem = (void *) spbcg_mem;

  /* Attach linear solver memory to integrator memory */
  ark_mem->ark_lmem = arkspils_mem;

  return(ARKSPILS_SUCCESS);
}


/*---------------------------------------------------------------
 ARKSpbcgInit:

 This routine does remaining initializations specific to the 
 Spbcg linear solver.
---------------------------------------------------------------*/
static int ARKSpbcgInit(ARKodeMem ark_mem)
{
  ARKSpilsMem arkspils_mem;
  SpbcgMem spbcg_mem;

  arkspils_mem = (ARKSpilsMem) ark_mem->ark_lmem;
  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;

  /* Initialize counters */
  arkspils_mem->s_npe = arkspils_mem->s_nli = 0;
  arkspils_mem->s_nps = arkspils_mem->s_ncfl = 0;
  arkspils_mem->s_nstlpre = arkspils_mem->s_njtimes = 0;
  arkspils_mem->s_nfes = 0;

  /* Check for legal combination pretype - psolve */
  if ((arkspils_mem->s_pretype != PREC_NONE) && 
      (arkspils_mem->s_psolve == NULL)) {
    arkProcessError(ark_mem, -1, "ARKSPBCG", 
		    "ARKSpbcgInit", MSGS_PSOLVE_REQ);
    arkspils_mem->s_last_flag = ARKSPILS_ILL_INPUT;
    return(-1);
  }

  /* Set setupNonNull = TRUE iff there is preconditioning
     (pretype != PREC_NONE)  and there is a preconditioning
     setup phase (pset != NULL) */
  ark_mem->ark_setupNonNull = (arkspils_mem->s_pretype != PREC_NONE) 
    && (arkspils_mem->s_pset != NULL);

  /* Set Jacobian-related fields, based on jtimesDQ */
  if (arkspils_mem->s_jtimesDQ) {
    arkspils_mem->s_jtimes = ARKSpilsDQJtimes;
    arkspils_mem->s_j_data = ark_mem;
  } else {
    arkspils_mem->s_j_data = ark_mem->ark_user_data;
  }

  /*  Set maxl in the SPBCG memory in case it was changed by the user */
  spbcg_mem->l_max  = arkspils_mem->s_maxl;

  arkspils_mem->s_last_flag = ARKSPILS_SUCCESS;
  return(0);
}


/*---------------------------------------------------------------
 ARKSpbcgSetup:

 This routine does the setup operations for the Spbcg linear 
 solver. It makes a decision as to whether or not to signal for 
 reevaluation of Jacobian data in the pset routine, based on 
 various state variables, then it calls pset. If we signal for 
 reevaluation, then we reset jcur = *jcurPtr to TRUE, regardless 
 of the pset output. In any case, if jcur == TRUE, we increment 
 npe and save nst in nstlpre.
---------------------------------------------------------------*/
static int ARKSpbcgSetup(ARKodeMem ark_mem, int convfail, 
			 N_Vector ypred, N_Vector fpred, 
			 booleantype *jcurPtr, N_Vector vtemp1,
			 N_Vector vtemp2, N_Vector vtemp3)
{
  booleantype jbad, jok;
  realtype dgamma;
  int  retval;
  ARKSpilsMem arkspils_mem;

  arkspils_mem = (ARKSpilsMem) ark_mem->ark_lmem;

  /* Use nst, gamma/gammap, and convfail to set J eval. flag jok */
  dgamma = SUNRabs((ark_mem->ark_gamma/ark_mem->ark_gammap) - ONE);
  jbad = (ark_mem->ark_nst == 0) || 
    (ark_mem->ark_nst > arkspils_mem->s_nstlpre + ARKSPILS_MSBPRE) ||
    ((convfail == ARK_FAIL_BAD_J) && (dgamma < ARKSPILS_DGMAX)) ||
    (convfail == ARK_FAIL_OTHER);
  *jcurPtr = jbad;
  jok = !jbad;

  /* Call pset routine and possibly reset jcur */
  retval = arkspils_mem->s_pset(ark_mem->ark_tn, ypred, fpred, jok, 
				jcurPtr, ark_mem->ark_gamma, 
				arkspils_mem->s_P_data, vtemp1, 
				vtemp2, vtemp3);
  if (retval < 0) {
    arkProcessError(ark_mem, SPBCG_PSET_FAIL_UNREC, "ARKSPBCG", 
		    "ARKSpbcgSetup", MSGS_PSET_FAILED);
    arkspils_mem->s_last_flag = SPBCG_PSET_FAIL_UNREC;
  }
  if (retval > 0) {
    arkspils_mem->s_last_flag = SPBCG_PSET_FAIL_REC;
  }

  if (jbad) *jcurPtr = TRUE;

  /* If jcur = TRUE, increment npe and save nst value */
  if (*jcurPtr) {
    arkspils_mem->s_npe++;
    arkspils_mem->s_nstlpre = ark_mem->ark_nst;
  }

  arkspils_mem->s_last_flag = SPBCG_SUCCESS;

  /* Return the same value that pset returned */
  return(retval);
}


/*---------------------------------------------------------------
 ARKSpbcgSolve:

 This routine handles the call to the generic solver SpbcgSolve
 for the solution of the linear system Ax = b with the SPBCG 
 method. The solution x is returned in the vector b.

 If the WRMS norm of b is small, we return x = b (if this is the 
 first Newton iteration) or x = 0 (if a later Newton iteration).

 Otherwise, we set the tolerance parameter and initial guess 
 (x = 0), call SpbcgSolve, and copy the solution x into b. The 
 x-scaling and b-scaling arrays are both equal to weight.

 The counters nli, nps, and ncfl are incremented, and the return 
 value is set according to the success of SpbcgSolve. The 
 success flag is returned if SpbcgSolve converged, or if this is 
 the first Newton iteration and the residual norm was reduced 
 below its initial value.
---------------------------------------------------------------*/
static int ARKSpbcgSolve(ARKodeMem ark_mem, N_Vector b, 
			 N_Vector weight, N_Vector ynow, 
			 N_Vector fnow)
{
  realtype bnorm, res_norm;
  ARKSpilsMem arkspils_mem;
  SpbcgMem spbcg_mem;
  int nli_inc, nps_inc, retval;
  
  arkspils_mem = (ARKSpilsMem) ark_mem->ark_lmem;
  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;

  /* Test norm(b); if small, return x = 0 or x = b */
  arkspils_mem->s_deltar = arkspils_mem->s_eplifac * ark_mem->ark_eLTE; 

  bnorm = N_VWrmsNorm(b, weight);
  if (bnorm <= arkspils_mem->s_deltar) {
    if (ark_mem->ark_mnewt > 0) N_VConst(ZERO, b); 
    return(0);
  }

  /* Set vectors ycur and fcur for use by the Atimes and Psolve routines */
  arkspils_mem->s_ycur = ynow;
  arkspils_mem->s_fcur = fnow;

  /* Set inputs delta and initial guess x = 0 to SpbcgSolve */  
  arkspils_mem->s_delta = arkspils_mem->s_deltar * arkspils_mem->s_sqrtN;
  N_VConst(ZERO, arkspils_mem->s_x);
  /* N_VConst(ark_mem->ark_uround, arkspils_mem->s_x); */
  
  /* Call SpbcgSolve and copy x to b */
  retval = SpbcgSolve(spbcg_mem, ark_mem, arkspils_mem->s_x, b, 
		      arkspils_mem->s_pretype, arkspils_mem->s_delta,
		      ark_mem, weight, weight, ARKSpilsAtimes, 
		      ARKSpilsPSolve, &res_norm, &nli_inc, &nps_inc);
  N_VScale(ONE, arkspils_mem->s_x, b);
  
  /* Increment counters nli, nps, and ncfl */
  arkspils_mem->s_nli += nli_inc;
  arkspils_mem->s_nps += nps_inc;
  if (retval != SPBCG_SUCCESS) arkspils_mem->s_ncfl++;

  /* Log solver statistics to diagnostics file (if requested) */
  if (ark_mem->ark_report) 
    fprintf(ark_mem->ark_diagfp, "      kry  %19.16g  %19.16g  %i  %i\n", 
	    bnorm, res_norm, nli_inc, nps_inc);
  
  /* Interpret return value from SpbcgSolve */
  arkspils_mem->s_last_flag = retval;

  switch(retval) {

  case SPBCG_SUCCESS:
    return(0);
    break;
  case SPBCG_RES_REDUCED:
    /* allow reduction but not solution on first Newton iteration, 
       otherwise return with a recoverable failure */
    if (ark_mem->ark_mnewt == 0) return(0);
    else                         return(1);
    break;
  case SPBCG_CONV_FAIL:
    return(1);
    break;
  case SPBCG_PSOLVE_FAIL_REC:
    return(1);
    break;
  case SPBCG_ATIMES_FAIL_REC:
    return(1);
    break;
  case SPBCG_MEM_NULL:
    return(-1);
    break;
  case SPBCG_ATIMES_FAIL_UNREC:
    arkProcessError(ark_mem, SPBCG_ATIMES_FAIL_UNREC, "ARKSPBCG", 
		    "ARKSpbcgSolve", MSGS_JTIMES_FAILED);    
    return(-1);
    break;
  case SPBCG_PSOLVE_FAIL_UNREC:
    arkProcessError(ark_mem, SPBCG_PSOLVE_FAIL_UNREC, "ARKSPBCG", 
		    "ARKSpbcgSolve", MSGS_PSOLVE_FAILED);
    return(-1);
    break;
  }

  return(0);
}


/*---------------------------------------------------------------
 ARKSpbcgFree:

 This routine frees memory specific to the Spbcg linear solver.
---------------------------------------------------------------*/
static void ARKSpbcgFree(ARKodeMem ark_mem)
{
  ARKSpilsMem arkspils_mem;
  SpbcgMem spbcg_mem;

  arkspils_mem = (ARKSpilsMem) ark_mem->ark_lmem;

  N_VDestroy(arkspils_mem->s_ytemp);
  N_VDestroy(arkspils_mem->s_x);

  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;
  SpbcgFree(spbcg_mem);

  if (arkspils_mem->s_pfree != NULL) (arkspils_mem->s_pfree)(ark_mem);

  free(arkspils_mem);
  ark_mem->ark_lmem = NULL;
}




/*---------------------------------------------------------------
 ARKMassSpbcg:

 This routine initializes the memory record and sets various 
 function fields specific to the Spbcg mass matrix solver module. 
 ARKMassSpbcg first calls the existing mfree routine if this is 
 not NULL. It then sets the ark_minit, ark_msetup, ark_msolve, 
 ark_mfree fields in (*arkode_mem) to be ARKMassSpbcgInit, 
 ARKMassSpbcgSetup, ARKMassSpbcgSolve, and ARKMassSpbcgFree, 
 respectively. It allocates memory for a structure of type 
 ARKSpilsMassMemRec and sets the ark_mass_mem field in
 (*arkode_mem) to the address of this structure. It sets 
 MassSetupNonNull in (*arkode_mem), and sets various fields in
 the ARKSpilsMassMemRec structure, allocates memory for ytemp 
 and x, and calls SpbcgMalloc to allocate memory for the Spbcg 
 solver.
---------------------------------------------------------------*/
int ARKMassSpbcg(void *arkode_mem, int pretype, int maxl, 
		 ARKMTimesFn mtimes, void *mtimes_data)
{
  ARKodeMem ark_mem;
  ARKSpilsMassMem arkspils_mem;
  SpbcgMem spbcg_mem;
  int mxl;

  /* Return immediately if arkode_mem is NULL */
  if (arkode_mem == NULL) {
    arkProcessError(NULL, ARKSPILS_MEM_NULL, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_ARKMEM_NULL);
    return(ARKSPILS_MEM_NULL);
  }
  ark_mem = (ARKodeMem) arkode_mem;

  /* Check if N_VDotProd is present */
  if (ark_mem->ark_tempv->ops->nvdotprod == NULL) {
    arkProcessError(ark_mem, ARKSPILS_ILL_INPUT, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_BAD_NVECTOR);
    return(ARKSPILS_ILL_INPUT);
  }

  if (ark_mem->ark_mfree != NULL) ark_mem->ark_mfree(ark_mem);

  /* Set four main function fields in ark_mem, enable mass matrix */
  ark_mem->ark_mass_matrix = TRUE;
  ark_mem->ark_minit  = ARKMassSpbcgInit;
  ark_mem->ark_msetup = ARKMassSpbcgSetup;
  ark_mem->ark_msolve = ARKMassSpbcgSolve;
  ark_mem->ark_mfree  = ARKMassSpbcgFree;
  ark_mem->ark_msolve_type = 0;

  /* Get memory for ARKSpilsMassMemRec */
  arkspils_mem = NULL;
  arkspils_mem = (ARKSpilsMassMem) malloc(sizeof(struct ARKSpilsMassMemRec));
  if (arkspils_mem == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_MEM_FAIL);
    return(ARKSPILS_MEM_FAIL);
  }

  /* Set mass-matrix-vector product routine */
  ark_mem->ark_mtimes      = mtimes;
  ark_mem->ark_mtimes_data = mtimes_data;

  /* Set ILS type */
  arkspils_mem->s_type = SPILS_SPBCG;

  /* Set Spbcg parameters that have been passed in call sequence */
  arkspils_mem->s_pretype    = pretype;
  mxl = arkspils_mem->s_maxl = (maxl <= 0) ? ARKSPILS_MAXL : maxl;

  /* Set defaults for preconditioner-related fields */
  arkspils_mem->s_pset   = NULL;
  arkspils_mem->s_psolve = NULL;
  arkspils_mem->s_pfree  = NULL;
  arkspils_mem->s_P_data = ark_mem->ark_user_data;

  /* Initialize counters */
  arkspils_mem->s_npe = arkspils_mem->s_nli  = 0;
  arkspils_mem->s_nps = arkspils_mem->s_ncfl = 0;

  /* Set default values for the rest of the Spbcg parameters */
  arkspils_mem->s_eplifac       = ARKSPILS_EPLIN;
  arkspils_mem->s_last_flag     = ARKSPILS_SUCCESS;
  ark_mem->ark_MassSetupNonNull = FALSE;

  /* Check for legal pretype */ 
  if ((pretype != PREC_NONE) && (pretype != PREC_LEFT) &&
      (pretype != PREC_RIGHT) && (pretype != PREC_BOTH)) {
    arkProcessError(ark_mem, ARKSPILS_ILL_INPUT, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_BAD_PRETYPE);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_ILL_INPUT);
  }

  /* Allocate memory for ytemp and x */
  arkspils_mem->s_ytemp = N_VClone(ark_mem->ark_tempv);
  if (arkspils_mem->s_ytemp == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_MEM_FAIL);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }

  arkspils_mem->s_x = N_VClone(ark_mem->ark_tempv);
  if (arkspils_mem->s_x == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_MEM_FAIL);
    N_VDestroy(arkspils_mem->s_ytemp);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }

  /* Compute sqrtN from a dot product */
  N_VConst(ONE, arkspils_mem->s_ytemp);
  arkspils_mem->s_sqrtN = SUNRsqrt(N_VDotProd(arkspils_mem->s_ytemp, 
					   arkspils_mem->s_ytemp));

  /* Call SpbcgMalloc to allocate workspace for Spbcg */
  spbcg_mem = NULL;
  spbcg_mem = SpbcgMalloc(mxl, ark_mem->ark_tempv);
  if (spbcg_mem == NULL) {
    arkProcessError(ark_mem, ARKSPILS_MEM_FAIL, "ARKSPBCG", 
		    "ARKMassSpbcg", MSGS_MEM_FAIL);
    N_VDestroy(arkspils_mem->s_ytemp);
    N_VDestroy(arkspils_mem->s_x);
    free(arkspils_mem); arkspils_mem = NULL;
    return(ARKSPILS_MEM_FAIL);
  }
  
  /* Attach SPBCG memory to spils memory structure */
  arkspils_mem->s_spils_mem = (void *) spbcg_mem;

  /* Attach linear solver memory to integrator memory */
  ark_mem->ark_mass_mem = arkspils_mem;

  return(ARKSPILS_SUCCESS);
}


/*---------------------------------------------------------------
 ARKMassSpbcgInit:

 This routine does remaining initializations specific to the 
 Spbcg linear solver.
---------------------------------------------------------------*/
static int ARKMassSpbcgInit(ARKodeMem ark_mem)
{
  ARKSpilsMassMem arkspils_mem;
  SpbcgMem spbcg_mem;
  arkspils_mem = (ARKSpilsMassMem) ark_mem->ark_mass_mem;
  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;

  /* Initialize counters */
  arkspils_mem->s_npe = arkspils_mem->s_nli  = 0;
  arkspils_mem->s_nps = arkspils_mem->s_ncfl = 0;

  /* Check for legal combination pretype - psolve */
  if ((arkspils_mem->s_pretype != PREC_NONE) && 
      (arkspils_mem->s_psolve == NULL)) {
    arkProcessError(ark_mem, -1, "ARKSPBCG", 
		    "ARKMassSpbcgInit", MSGS_PSOLVE_REQ);
    arkspils_mem->s_last_flag = ARKSPILS_ILL_INPUT;
    return(-1);
  }

  /* Set MassSetupNonNull = TRUE iff there is preconditioning
     (pretype != PREC_NONE)  and there is a preconditioning
     setup phase (pset != NULL) */
  ark_mem->ark_MassSetupNonNull = (arkspils_mem->s_pretype != PREC_NONE) 
    && (arkspils_mem->s_pset != NULL);

  /*  Set maxl in the SPBCG memory in case it was changed by the user */
  spbcg_mem->l_max = arkspils_mem->s_maxl;

  arkspils_mem->s_last_flag = ARKSPILS_SUCCESS;
  return(0);
}


/*---------------------------------------------------------------
 ARKMassSpbcgSetup:

 This routine does the setup operations for the Spbcg mass matrix 
 solver. It calls pset and increments npe.
---------------------------------------------------------------*/
static int ARKMassSpbcgSetup(ARKodeMem ark_mem, N_Vector vtemp1, 
			     N_Vector vtemp2, N_Vector vtemp3)
{
  int  retval;
  ARKSpilsMassMem arkspils_mem;
  arkspils_mem = (ARKSpilsMassMem) ark_mem->ark_mass_mem;

  /* Call pset routine */
  retval = arkspils_mem->s_pset(ark_mem->ark_tn, 
				arkspils_mem->s_P_data, 
				vtemp1, vtemp2, vtemp3);
  arkspils_mem->s_npe++;
  if (retval < 0) {
    arkProcessError(ark_mem, SPBCG_PSET_FAIL_UNREC, "ARKSPBCG", 
		    "ARKMassSpbcgSetup", MSGS_PSET_FAILED);
    arkspils_mem->s_last_flag = SPBCG_PSET_FAIL_UNREC;
  }
  if (retval > 0) {
    arkspils_mem->s_last_flag = SPBCG_PSET_FAIL_REC;
  }
  if (retval == 0) {
    arkspils_mem->s_last_flag = SPBCG_SUCCESS;
  }

  /* Return the same value that pset returned */
  return(retval);
}


/*---------------------------------------------------------------
 ARKMassSpbcgSolve:

 This routine handles the call to the generic solver SpbcgSolve
 for the solution of the mass matrix system Mx = b with the SPBCG 
 method. The solution x is returned in the vector b.

 We set the tolerance parameter and initial guess (x = 0), call
 SpbcgSolve, and copy the solution x into b. The 
 x-scaling and b-scaling arrays are both equal to weight.

 The counters nli, nps, and ncfl are incremented, and the return 
 value is set according to the success of SpbcgSolve.
---------------------------------------------------------------*/
static int ARKMassSpbcgSolve(ARKodeMem ark_mem, N_Vector b, 
			     N_Vector weight)
{
  realtype res_norm;
  ARKSpilsMassMem arkspils_mem;
  SpbcgMem spbcg_mem;
  int nli_inc, nps_inc, retval;
  
  arkspils_mem = (ARKSpilsMassMem) ark_mem->ark_mass_mem;
  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;

  /* Set inputs delta and initial guess x = 0 to SpbcgSolve */  
  arkspils_mem->s_deltar = arkspils_mem->s_eplifac * ark_mem->ark_eLTE; 
  arkspils_mem->s_delta  = arkspils_mem->s_deltar * arkspils_mem->s_sqrtN;
  N_VConst(ZERO, arkspils_mem->s_x);
  
  /* Call SpbcgSolve and copy x to b */
  retval = SpbcgSolve(spbcg_mem, ark_mem, arkspils_mem->s_x, b, 
		      arkspils_mem->s_pretype, arkspils_mem->s_delta,
		      ark_mem, weight, weight, ARKSpilsMtimes, 
		      ARKSpilsMPSolve, &res_norm, &nli_inc, &nps_inc);
  N_VScale(ONE, arkspils_mem->s_x, b);
  
  /* Increment counters nli, nps, and ncfl */
  arkspils_mem->s_nli += nli_inc;
  arkspils_mem->s_nps += nps_inc;
  if (retval != SPBCG_SUCCESS) arkspils_mem->s_ncfl++;

  /* Log solver statistics to diagnostics file (if requested) */
  if (ark_mem->ark_report) 
    fprintf(ark_mem->ark_diagfp, "      mass  %19.16g  %i  %i\n", 
	    res_norm, nli_inc, nps_inc);
  
  /* Interpret return value from SpbcgSolve */
  arkspils_mem->s_last_flag = retval;

  switch(retval) {

  case SPBCG_SUCCESS:
    return(0);
    break;
  case SPBCG_RES_REDUCED:
    return(1);
    break;
  case SPBCG_CONV_FAIL:
    return(1);
    break;
  case SPBCG_PSOLVE_FAIL_REC:
    return(1);
    break;
  case SPBCG_ATIMES_FAIL_REC:
    return(1);
    break;
  case SPBCG_MEM_NULL:
    return(-1);
    break;
  case SPBCG_ATIMES_FAIL_UNREC:
    arkProcessError(ark_mem, SPBCG_ATIMES_FAIL_UNREC, "ARKSPBCG", 
		    "ARKMassSpbcgSolve", MSGS_MTIMES_FAILED);    
    return(-1);
    break;
  case SPBCG_PSOLVE_FAIL_UNREC:
    arkProcessError(ark_mem, SPBCG_PSOLVE_FAIL_UNREC, "ARKSPBCG", 
		    "ARKMassSpbcgSolve", MSGS_PSOLVE_FAILED);
    return(-1);
    break;
  }

  return(0);
}


/*---------------------------------------------------------------
 ARKMassSpbcgFree:

 This routine frees memory specific to the Spbcg mass matrix solver.
---------------------------------------------------------------*/
static void ARKMassSpbcgFree(ARKodeMem ark_mem)
{
  ARKSpilsMassMem arkspils_mem;
  SpbcgMem spbcg_mem;
  arkspils_mem = (ARKSpilsMassMem) ark_mem->ark_mass_mem;

  N_VDestroy(arkspils_mem->s_ytemp);
  N_VDestroy(arkspils_mem->s_x);

  spbcg_mem = (SpbcgMem) arkspils_mem->s_spils_mem;
  SpbcgFree(spbcg_mem);

  if (arkspils_mem->s_pfree != NULL) (arkspils_mem->s_pfree)(ark_mem);

  free(arkspils_mem);
  ark_mem->ark_mass_mem = NULL;
}


/*---------------------------------------------------------------
     EOF
---------------------------------------------------------------*/
