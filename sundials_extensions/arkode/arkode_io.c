/*
 * -----------------------------------------------------------------
 * $Revision: 1.0 $
 * $Date:  $
 * ----------------------------------------------------------------- 
 * Programmer(s): Daniel R. Reynolds @ SMU
 * -----------------------------------------------------------------
 * This is the implementation file for the optional input and output
 * functions for the ARKODE solver.
 * -----------------------------------------------------------------
 ***** UNTOUCHED *****
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "arkode_impl.h"
#include <sundials/sundials_types.h>

#define ZERO RCONST(0.0)
#define ONE  RCONST(1.0)

#define lrw (ark_mem->ark_lrw)
#define liw (ark_mem->ark_liw)
#define lrw1 (ark_mem->ark_lrw1)
#define liw1 (ark_mem->ark_liw1)

/* 
 * =================================================================
 * ARKODE optional input functions
 * =================================================================
 */

/* 
 * ARKodeSetErrHandlerFn
 *
 * Specifies the error handler function
 */

int ARKodeSetErrHandlerFn(void *arkode_mem, ARKErrHandlerFn ehfun, void *eh_data)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetErrHandlerFn", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_ehfun = ehfun;
  ark_mem->ark_eh_data = eh_data;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetErrFile
 *
 * Specifies the FILE pointer for output (NULL means no messages)
 */

int ARKodeSetErrFile(void *arkode_mem, FILE *errfp)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetErrFile", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_errfp = errfp;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetIterType
 *
 * Specifies the iteration type (ARK_FUNCTIONAL or ARK_NEWTON)
 */

int ARKodeSetIterType(void *arkode_mem, int iter)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetIterType", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if ((iter != ARK_FUNCTIONAL) && (iter != ARK_NEWTON)) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetIterType", MSGARK_BAD_ITER);
    return (ARK_ILL_INPUT);
  }

  ark_mem->ark_iter = iter;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetUserData
 *
 * Specifies the user data pointer for f
 */

int ARKodeSetUserData(void *arkode_mem, void *user_data)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetUserData", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_user_data = user_data;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxOrd
 *
 * Specifies the maximum method order
 */

int ARKodeSetMaxOrd(void *arkode_mem, int maxord)
{
  ARKodeMem ark_mem;
  int qmax_alloc;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxOrd", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if (maxord <= 0) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMaxOrd", MSGARK_NEG_MAXORD);
    return(ARK_ILL_INPUT);
  }
  
  /* Cannot increase maximum order beyond the value that
     was used when allocating memory */
  qmax_alloc = ark_mem->ark_qmax_alloc;

  if (maxord > qmax_alloc) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMaxOrd", MSGARK_BAD_MAXORD);
    return(ARK_ILL_INPUT);
  }

  ark_mem->ark_qmax = maxord;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxNumSteps
 *
 * Specifies the maximum number of integration steps
 */

int ARKodeSetMaxNumSteps(void *arkode_mem, long int mxsteps)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxNumSteps", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  /* Passing mxsteps=0 sets the default. Passing mxsteps<0 disables the test. */

  if (mxsteps == 0)
    ark_mem->ark_mxstep = MXSTEP_DEFAULT;
  else
    ark_mem->ark_mxstep = mxsteps;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxHnilWarns
 *
 * Specifies the maximum number of warnings for small h
 */

int ARKodeSetMaxHnilWarns(void *arkode_mem, int mxhnil)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxHnilWarns", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_mxhnil = mxhnil;

  return(ARK_SUCCESS);
}

/* 
 *ARKodeSetStabLimDet
 *
 * Turns on/off the stability limit detection algorithm
 */

int ARKodeSetStabLimDet(void *arkode_mem, booleantype sldet)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetStabLimDet", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if( sldet && (ark_mem->ark_lmm != ARK_BDF) ) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetStabLimDet", MSGARK_SET_SLDET);
    return(ARK_ILL_INPUT);
  }

  ark_mem->ark_sldeton = sldet;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetInitStep
 *
 * Specifies the initial step size
 */

int ARKodeSetInitStep(void *arkode_mem, realtype hin)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetInitStep", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_hin = hin;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMinStep
 *
 * Specifies the minimum step size
 */

int ARKodeSetMinStep(void *arkode_mem, realtype hmin)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMinStep", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if (hmin<0) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMinStep", MSGARK_NEG_HMIN);
    return(ARK_ILL_INPUT);
  }

  /* Passing 0 sets hmin = zero */
  if (hmin == ZERO) {
    ark_mem->ark_hmin = HMIN_DEFAULT;
    return(ARK_SUCCESS);
  }

  if (hmin * ark_mem->ark_hmax_inv > ONE) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMinStep", MSGARK_BAD_HMIN_HMAX);
    return(ARK_ILL_INPUT);
  }

  ark_mem->ark_hmin = hmin;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxStep
 *
 * Specifies the maximum step size
 */

int ARKodeSetMaxStep(void *arkode_mem, realtype hmax)
{
  realtype hmax_inv;
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxStep", MSGARK_NO_MEM);
    return (ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if (hmax < 0) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMaxStep", MSGARK_NEG_HMAX);
    return(ARK_ILL_INPUT);
  }

  /* Passing 0 sets hmax = infinity */
  if (hmax == ZERO) {
    ark_mem->ark_hmax_inv = HMAX_INV_DEFAULT;
    return(ARK_SUCCESS);
  }

  hmax_inv = ONE/hmax;
  if (hmax_inv * ark_mem->ark_hmin > ONE) {
    ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetMaxStep", MSGARK_BAD_HMIN_HMAX);
    return(ARK_ILL_INPUT);
  }

  ark_mem->ark_hmax_inv = hmax_inv;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetStopTime
 *
 * Specifies the time beyond which the integration is not to proceed.
 */

int ARKodeSetStopTime(void *arkode_mem, realtype tstop)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetStopTime", MSGARK_NO_MEM);
    return (ARK_MEM_NULL);
  }
  ark_mem = (ARKodeMem) arkode_mem;

  /* If ARKode was called at least once, test if tstop is legal
   * (i.e. if it was not already passed).
   * If ARKodeSetStopTime is called before the first call to ARKode,
   * tstop will be checked in ARKode. */
  if (ark_mem->ark_nst > 0) {

    if ( (tstop - ark_mem->ark_tn) * ark_mem->ark_h < ZERO ) {
      ARKProcessError(ark_mem, ARK_ILL_INPUT, "ARKODE", "ARKodeSetStopTime", MSGARK_BAD_TSTOP, ark_mem->ark_tn);
      return(ARK_ILL_INPUT);
    }

  }

  ark_mem->ark_tstop = tstop;
  ark_mem->ark_tstopset = TRUE;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxErrTestFails
 *
 * Specifies the maximum number of error test failures during one
 * step try.
 */

int ARKodeSetMaxErrTestFails(void *arkode_mem, int maxnef)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxErrTestFails", MSGARK_NO_MEM);
    return (ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_maxnef = maxnef;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxConvFails
 *
 * Specifies the maximum number of nonlinear convergence failures 
 * during one step try.
 */

int ARKodeSetMaxConvFails(void *arkode_mem, int maxncf)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxConvFails", MSGARK_NO_MEM);
    return (ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_maxncf = maxncf;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetMaxNonlinIters
 *
 * Specifies the maximum number of nonlinear iterations during
 * one solve.
 */

int ARKodeSetMaxNonlinIters(void *arkode_mem, int maxcor)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetMaxNonlinIters", MSGARK_NO_MEM);
    return (ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_maxcor = maxcor;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetNonlinConvCoef
 *
 * Specifies the coeficient in the nonlinear solver convergence
 * test
 */

int ARKodeSetNonlinConvCoef(void *arkode_mem, realtype nlscoef)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetNonlinConvCoef", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_nlscoef = nlscoef;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeSetRootDirection
 *
 * Specifies the direction of zero-crossings to be monitored.
 * The default is to monitor both crossings.
 */

int ARKodeSetRootDirection(void *arkode_mem, int *rootdir)
{
  ARKodeMem ark_mem;
  int i, nrt;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetRootDirection", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  nrt = ark_mem->ark_nrtfn;
  if (nrt==0) {
    ARKProcessError(NULL, ARK_ILL_INPUT, "ARKODE", "ARKodeSetRootDirection", MSGARK_NO_ROOT);
    return(ARK_ILL_INPUT);    
  }

  for(i=0; i<nrt; i++) ark_mem->ark_rootdir[i] = rootdir[i];

  return(ARK_SUCCESS);
}

/*
 * ARKodeSetNoInactiveRootWarn
 *
 * Disables issuing a warning if some root function appears
 * to be identically zero at the beginning of the integration
 */

int ARKodeSetNoInactiveRootWarn(void *arkode_mem)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeSetNoInactiveRootWarn", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  ark_mem->ark_mxgnull = 0;
  
  return(ARK_SUCCESS);
}


/* 
 * =================================================================
 * ARKODE optional output functions
 * =================================================================
 */

/* 
 * Readability constants
 */

#define nst            (ark_mem->ark_nst)
#define nfe            (ark_mem->ark_nfe)
#define ncfn           (ark_mem->ark_ncfn)
#define netf           (ark_mem->ark_netf)
#define nni            (ark_mem->ark_nni)
#define nsetups        (ark_mem->ark_nsetups)
#define qu             (ark_mem->ark_qu)
#define next_q         (ark_mem->ark_next_q)
#define ewt            (ark_mem->ark_ewt)  
#define hu             (ark_mem->ark_hu)
#define next_h         (ark_mem->ark_next_h)
#define h0u            (ark_mem->ark_h0u)
#define tolsf          (ark_mem->ark_tolsf)  
#define acor           (ark_mem->ark_acor)
#define lrw            (ark_mem->ark_lrw)
#define liw            (ark_mem->ark_liw)
#define nge            (ark_mem->ark_nge)
#define iroots         (ark_mem->ark_iroots)
#define nor            (ark_mem->ark_nor)
#define sldeton        (ark_mem->ark_sldeton)
#define tn             (ark_mem->ark_tn)
#define efun           (ark_mem->ark_efun)

/*
 * ARKodeGetNumSteps
 *
 * Returns the current number of integration steps
 */

int ARKodeGetNumSteps(void *arkode_mem, long int *nsteps)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumSteps", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nsteps = nst;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNumRhsEvals
 *
 * Returns the current number of calls to f
 */

int ARKodeGetNumRhsEvals(void *arkode_mem, long int *nfevals)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumRhsEvals", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nfevals = nfe;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNumLinSolvSetups
 *
 * Returns the current number of calls to the linear solver setup routine
 */

int ARKodeGetNumLinSolvSetups(void *arkode_mem, long int *nlinsetups)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumLinSolvSetups", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nlinsetups = nsetups;

  return(ARK_SUCCESS);
}

/*
 * ARKodeGetNumErrTestFails
 *
 * Returns the current number of error test failures
 */

int ARKodeGetNumErrTestFails(void *arkode_mem, long int *netfails)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumErrTestFails", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *netfails = netf;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetLastOrder
 *
 * Returns the order on the last succesful step
 */

int ARKodeGetLastOrder(void *arkode_mem, int *qlast)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetLastOrder", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *qlast = qu;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetCurrentOrder
 *
 * Returns the order to be attempted on the next step
 */

int ARKodeGetCurrentOrder(void *arkode_mem, int *qcur)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetCurrentOrder", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *qcur = next_q;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNumStabLimOrderReds
 *
 * Returns the number of order reductions triggered by the stability
 * limit detection algorithm
 */

int ARKodeGetNumStabLimOrderReds(void *arkode_mem, long int *nslred)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumStabLimOrderReds", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  if (sldeton==FALSE)
    *nslred = 0;
  else
    *nslred = nor;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetActualInitStep
 *
 * Returns the step size used on the first step
 */

int ARKodeGetActualInitStep(void *arkode_mem, realtype *hinused)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetActualInitStep", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *hinused = h0u;

  return(ARK_SUCCESS);
}

/*
 * ARKodeGetLastStep
 *
 * Returns the step size used on the last successful step
 */

int ARKodeGetLastStep(void *arkode_mem, realtype *hlast)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetLastStep", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *hlast = hu;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetCurrentStep
 *
 * Returns the step size to be attempted on the next step
 */

int ARKodeGetCurrentStep(void *arkode_mem, realtype *hcur)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetCurrentStep", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;
  
  *hcur = next_h;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetCurrentTime
 *
 * Returns the current value of the independent variable
 */

int ARKodeGetCurrentTime(void *arkode_mem, realtype *tcur)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetCurrentTime", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *tcur = tn;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetTolScaleFactor
 *
 * Returns a suggested factor for scaling tolerances
 */

int ARKodeGetTolScaleFactor(void *arkode_mem, realtype *tolsfact)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetTolScaleFactor", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *tolsfact = tolsf;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetErrWeights
 *
 * This routine returns the current weight vector.
 */

int ARKodeGetErrWeights(void *arkode_mem, N_Vector eweight)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetErrWeights", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  N_VScale(ONE, ewt, eweight);

  return(ARK_SUCCESS);
}

/*
 * ARKodeGetEstLocalErrors
 *
 * Returns an estimate of the local error
 */

int ARKodeGetEstLocalErrors(void *arkode_mem, N_Vector ele)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetEstLocalErrors", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  N_VScale(ONE, acor, ele);

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetWorkSpace
 *
 * Returns integrator work space requirements
 */

int ARKodeGetWorkSpace(void *arkode_mem, long int *lenrw, long int *leniw)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetWorkSpace", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *leniw = liw;
  *lenrw = lrw;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetIntegratorStats
 *
 * Returns integrator statistics
 */

int ARKodeGetIntegratorStats(void *arkode_mem, long int *nsteps, long int *nfevals, 
                            long int *nlinsetups, long int *netfails, int *qlast, 
                            int *qcur, realtype *hinused, realtype *hlast, 
                            realtype *hcur, realtype *tcur)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetIntegratorStats", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nsteps = nst;
  *nfevals = nfe;
  *nlinsetups = nsetups;
  *netfails = netf;
  *qlast = qu;
  *qcur = next_q;
  *hinused = h0u;
  *hlast = hu;
  *hcur = next_h;
  *tcur = tn;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNumGEvals
 *
 * Returns the current number of calls to g (for rootfinding)
 */

int ARKodeGetNumGEvals(void *arkode_mem, long int *ngevals)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumGEvals", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *ngevals = nge;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetRootInfo
 *
 * Returns pointer to array rootsfound showing roots found
 */

int ARKodeGetRootInfo(void *arkode_mem, int *rootsfound)
{
  ARKodeMem ark_mem;
  int i, nrt;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetRootInfo", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  nrt = ark_mem->ark_nrtfn;

  for (i=0; i<nrt; i++) rootsfound[i] = iroots[i];

  return(ARK_SUCCESS);
}


/* 
 * ARKodeGetNumNonlinSolvIters
 *
 * Returns the current number of iterations in the nonlinear solver
 */

int ARKodeGetNumNonlinSolvIters(void *arkode_mem, long int *nniters)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumNonlinSolvIters", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nniters = nni;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNumNonlinSolvConvFails
 *
 * Returns the current number of convergence failures in the
 * nonlinear solver
 */

int ARKodeGetNumNonlinSolvConvFails(void *arkode_mem, long int *nncfails)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNumNonlinSolvConvFails", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nncfails = ncfn;

  return(ARK_SUCCESS);
}

/* 
 * ARKodeGetNonlinSolvStats
 *
 * Returns nonlinear solver statistics
 */

int ARKodeGetNonlinSolvStats(void *arkode_mem, long int *nniters, 
                            long int *nncfails)
{
  ARKodeMem ark_mem;

  if (arkode_mem==NULL) {
    ARKProcessError(NULL, ARK_MEM_NULL, "ARKODE", "ARKodeGetNonlinSolvStats", MSGARK_NO_MEM);
    return(ARK_MEM_NULL);
  }

  ark_mem = (ARKodeMem) arkode_mem;

  *nniters = nni;
  *nncfails = ncfn;

  return(ARK_SUCCESS);
}

/*-----------------------------------------------------------------*/

char *ARKodeGetReturnFlagName(long int flag)
{
  char *name;

  name = (char *)malloc(24*sizeof(char));

  switch(flag) {
  case ARK_SUCCESS:
    sprintf(name,"ARK_SUCCESS");
    break;
  case ARK_TSTOP_RETURN:
    sprintf(name,"ARK_TSTOP_RETURN");
    break;
  case ARK_ROOT_RETURN:
    sprintf(name,"ARK_ROOT_RETURN");
    break;
  case ARK_TOO_MUCH_WORK:
    sprintf(name,"ARK_TOO_MUCH_WORK");
    break;
  case ARK_TOO_MUCH_ACC:
    sprintf(name,"ARK_TOO_MUCH_ACC");
    break;
  case ARK_ERR_FAILURE:
    sprintf(name,"ARK_ERR_FAILURE");
    break;
  case ARK_CONV_FAILURE:
    sprintf(name,"ARK_CONV_FAILURE");
    break;
  case ARK_LINIT_FAIL:
    sprintf(name,"ARK_LINIT_FAIL");
    break;
  case ARK_LSETUP_FAIL:
    sprintf(name,"ARK_LSETUP_FAIL");
    break;
  case ARK_LSOLVE_FAIL:
    sprintf(name,"ARK_LSOLVE_FAIL");
    break;
  case ARK_RHSFUNC_FAIL:
    sprintf(name,"ARK_RHSFUNC_FAIL");
    break;
  case ARK_FIRST_RHSFUNC_ERR:
    sprintf(name,"ARK_FIRST_RHSFUNC_ERR");
    break;
  case ARK_REPTD_RHSFUNC_ERR:
    sprintf(name,"ARK_REPTD_RHSFUNC_ERR");
    break;
  case ARK_UNREC_RHSFUNC_ERR:
    sprintf(name,"ARK_UNREC_RHSFUNC_ERR");
    break;
  case ARK_RTFUNC_FAIL:
    sprintf(name,"ARK_RTFUNC_FAIL");
    break;
  case ARK_MEM_FAIL:
    sprintf(name,"ARK_MEM_FAIL");
    break;
  case ARK_MEM_NULL:
    sprintf(name,"ARK_MEM_NULL");
    break;
  case ARK_ILL_INPUT:
    sprintf(name,"ARK_ILL_INPUT");
    break;
  case ARK_NO_MALLOC:
    sprintf(name,"ARK_NO_MALLOC");
    break;
  case ARK_BAD_K:
    sprintf(name,"ARK_BAD_K");
    break;
  case ARK_BAD_T:
    sprintf(name,"ARK_BAD_T");
    break;
  case ARK_BAD_DKY:
    sprintf(name,"ARK_BAD_DKY");
    break;
  case ARK_TOO_CLOSE:
    sprintf(name,"ARK_TOO_CLOSE");
    break;    
  default:
    sprintf(name,"NONE");
  }

  return(name);
}
