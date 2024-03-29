/*
 * -----------------------------------------------------------------
 * $Revision: 4272 $
 * $Date: 2014-12-02 11:19:41 -0800 (Tue, 02 Dec 2014) $
 * ----------------------------------------------------------------- 
 * Programmer(s): Carol S. Woodward @ LLNL
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 * This is the implementation file for the CVKLU linear solver.
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "cvodes_impl.h"
#include "cvodes_sparse_impl.h"
#include "sundials/sundials_klu_impl.h"
#include "sundials/sundials_math.h"

/* Constants */

#define ONE          RCONST(1.0)
#define TWO          RCONST(2.0)

/* CVKLU linit, lsetup, lsolve, and lfree routines */
 
static int cvKLUInit(CVodeMem cv_mem);

static int cvKLUSetup(CVodeMem cv_mem, int convfail, N_Vector ypred, 
		      N_Vector fpred, booleantype *jcurPtr,
		      N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

static int cvKLUSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
		      N_Vector ycur, N_Vector fcur);

static void cvKLUFree(CVodeMem cv_mem);

/* CVKLU lfreeB function */
static void cvKLUFreeB(CVodeBMem cvb_mem);

/* 
 * ================================================================
 *
 *                   PART I - forward problems
 *
 * ================================================================
 */


/*
 * -----------------------------------------------------------------
 * CVKLU
 * -----------------------------------------------------------------
 * This routine initializes the memory record and sets various function
 * fields specific to the CVODE / KLU linear solver module.  
 * CVKLU first calls the existing lfree routine if this is not NULL.
 * Then it sets the cv_linit, cv_lsetup, cv_lsolve, cv_lperf, and
 * cv_lfree fields in (*cv_mem) to be CVKLUInit, CVKLUSetup,
 * CVKLUSolve, NULL, and CVKLUFree, respectively.
 * It allocates memory for a structure of type CVkluMemRec and sets
 * the cv_lmem field in (*cvode_mem) to the address of this structure.
 * It sets setupNonNull in (*cvode_mem) to TRUE.
 * Finally, it allocates memory for KLU.
 * The return value is CVSLS_SUCCESS = 0, CVSLS_LMEM_FAIL = -1,
 * or CVSLS_ILL_INPUT = -2.
 *
 * NOTE: The KLU linear solver assumes a serial implementation
 *       of the NVECTOR package. Therefore, CVKLU will first 
 *       test for a compatible N_Vector internal representation
 *       by checking that the function N_VGetArrayPointer exists.
 * -----------------------------------------------------------------
 */

int CVKLU(void *cvode_mem, int n, int nnz)
{
  CVodeMem cv_mem;
  CVSlsMem cvsls_mem;
  KLUData klu_data;
  int flag;

  /* Return immediately if cv_mem is NULL. */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVSLS_MEM_NULL, "CVSLS", "cvKLU", 
		    MSGSP_CVMEM_NULL);
    return(CVSLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  /* Test if the NVECTOR package is compatible with the Direct solver */
  if (cv_mem->cv_tempv->ops->nvgetarraypointer == NULL) {
    cvProcessError(cv_mem, CVSLS_ILL_INPUT, "CVSLS", "cvKLU", 
		    MSGSP_BAD_NVECTOR);
    return(CVSLS_ILL_INPUT);
  }

  if (cv_mem->cv_lfree != NULL) cv_mem->cv_lfree(cv_mem);

  /* Set five main function fields in cv_mem. */
  cv_mem->cv_linit  = cvKLUInit;
  cv_mem->cv_lsetup = cvKLUSetup;
  cv_mem->cv_lsolve = cvKLUSolve;
  cv_mem->cv_lfree  = cvKLUFree;

  /* Get memory for CVSlsMemRec. */
  cvsls_mem = (CVSlsMem) malloc(sizeof(struct CVSlsMemRec));
  if (cvsls_mem == NULL) {
    cvProcessError(cv_mem, CVSLS_MEM_FAIL, "CVSLS", "cvKLU", 
		    MSGSP_MEM_FAIL);
    return(CVSLS_MEM_FAIL);
  }

  /* Get memory for KLUData. */
  klu_data = (KLUData)malloc(sizeof(struct KLUDataRec));
  if (klu_data == NULL) {
    cvProcessError(cv_mem, CVSLS_MEM_FAIL, "CVSLS", "cvKLU", 
		    MSGSP_MEM_FAIL);
    return(CVSLS_MEM_FAIL);
  }

  cv_mem->cv_setupNonNull = TRUE;

  /* Set default Jacobian routine and Jacobian data */
  cvsls_mem->s_jaceval = NULL;
  cvsls_mem->s_jacdata = cv_mem->cv_user_data;

  /* Allocate memory for the sparse Jacobian */
  cvsls_mem->s_JacMat = NewSparseMat(n, n, nnz);
  if (cvsls_mem->s_JacMat == NULL) {
    cvProcessError(cv_mem, CVSLS_MEM_FAIL, "CVSLS", "cvKLU", 
		    MSGSP_MEM_FAIL);
    free(cvsls_mem);
    return(CVSLS_MEM_FAIL);
  }

  /* Allocate memory for saved sparse Jacobian */
  cvsls_mem->s_savedJ = NewSparseMat(n, n, nnz);
  if (cvsls_mem->s_savedJ == NULL) {
    cvProcessError(cv_mem, CVSLS_MEM_FAIL, "CVSLS", "cvKLU", 
		    MSGSP_MEM_FAIL);
    DestroySparseMat(cvsls_mem->s_JacMat);
    free(cvsls_mem);
    return(CVSLS_MEM_FAIL);
  }

  /* Allocate structures for KLU */
  klu_data->s_Symbolic = (klu_symbolic *)malloc(sizeof(klu_symbolic));
  klu_data->s_Numeric = (klu_numeric *)malloc(sizeof(klu_numeric));

  /* Set default parameters for KLU */
  flag = klu_defaults(&klu_data->s_Common);
  if (flag == 0) {
    cvProcessError(cv_mem, CVSLS_PACKAGE_FAIL, "CVSLS", "cvKLU", 
		    MSGSP_PACKAGE_FAIL);
    return(CVSLS_PACKAGE_FAIL);
  }

  /* Set ordering to COLAMD as the cvode default use.
     Users can set a different value with CVKLUSetOrdering,
     and the user-set value is loaded before any call to klu_analyze in
     CVKLUSetup.  */
  klu_data->s_ordering = 1;
  klu_data->s_Common.ordering = klu_data->s_ordering;

  /* Attach linear solver memory to the integrator memory */
  cvsls_mem->s_solver_data = (void *) klu_data;
  cv_mem->cv_lmem = cvsls_mem;

  cvsls_mem->s_last_flag = CVSLS_SUCCESS;

  return(CVSLS_SUCCESS);
}

/*
 * -----------------------------------------------------------------
 * CVKLU interface functions
 * -----------------------------------------------------------------
 */

/*
  This routine does remaining initializations specific to the CVKLU
  linear solver module.  
  It returns 0 if successful.
*/

static int cvKLUInit(CVodeMem cv_mem)
{
  CVSlsMem cvsls_mem;

  cvsls_mem = (CVSlsMem)cv_mem->cv_lmem;

  cvsls_mem->s_nje = 0;
  cvsls_mem->s_first_factorize = 1;
  cvsls_mem->s_nstlj = 0;

  cvsls_mem->s_last_flag = 0;
  return(0);
}

/*
  This routine does the setup operations for the CVKLU linear 
  solver module.  It calls the Jacobian evaluation routine,
  updates counters, and calls the LU factorization routine.
  The return value is either
     CVSLS_SUCCESS = 0  if successful,
     +1  if the jac routine failed recoverably or the
         LU factorization failed, or
     -1  if the jac routine failed unrecoverably.
*/

static int cvKLUSetup(CVodeMem cv_mem, int convfail, N_Vector ypred, 
		      N_Vector fpred, booleantype *jcurPtr,
		      N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3)
{
  booleantype jbad, jok;
  int retval;
  long int nst, nstlj;
  realtype tn, gamma, gammap, dgamma;
  CVSlsMem cvsls_mem;
  CVSlsSparseJacFn jaceval;
  KLUData klu_data;
  SlsMat JacMat, savedJ;
  void *jacdata;
  
  cvsls_mem = (CVSlsMem) (cv_mem->cv_lmem);
  tn = cv_mem->cv_tn; 
  gamma = cv_mem->cv_gamma;
  gammap = cv_mem->cv_gammap;
  nst = cv_mem->cv_nst;

  klu_data = (KLUData) cvsls_mem->s_solver_data;

  jaceval = cvsls_mem->s_jaceval;
  jacdata = cvsls_mem->s_jacdata;
  JacMat = cvsls_mem->s_JacMat;
  savedJ = cvsls_mem->s_savedJ;
  nstlj = cvsls_mem->s_nstlj;

  /* Check that Jacobian eval routine is set */
  if (jaceval == NULL) {
    cvProcessError(cv_mem, CVSLS_JAC_NOSET, "CVSLS", "cvKLUSetup", 
		    MSGSP_JAC_NOSET);
    free(cvsls_mem); cvsls_mem = NULL;
    return(CVSLS_JAC_NOSET);
  }

  /* Determine whether Jacobian needs to be recalculated */
  dgamma = SUNRabs((gamma/gammap) - ONE);
  jbad = (nst == 0) || (nst > nstlj + CVS_MSBJ) ||
         ((convfail == CV_FAIL_BAD_J) && (dgamma < CVS_DGMAX)) ||
         (convfail == CV_FAIL_OTHER);
  jok = !jbad;
  
  if (jok) {
    /* If jok = TRUE, use saved copy of J */
    *jcurPtr = FALSE;
    CopySparseMat(savedJ, JacMat);
  } else {
    /* If jok = FALSE, call jac routine for new J value */
    cvsls_mem->s_nje++;
    cvsls_mem->s_nstlj = nst;
    *jcurPtr = TRUE;
    SlsSetToZero(JacMat);
    retval = jaceval(tn, ypred, fpred, JacMat, jacdata, vtemp1, vtemp2, vtemp3);
    if (retval < 0) {
      cvProcessError(cv_mem, CVSLS_JACFUNC_UNRECVR, "CVSLS", "cvKLUSetup", MSGSP_JACFUNC_FAILED);
      cvsls_mem->s_last_flag = CVSLS_JACFUNC_UNRECVR;
      return(-1);
    }
    if (retval > 0) {
      cvsls_mem->s_last_flag = CVSLS_JACFUNC_RECVR;
      return(1);
    }

    CopySparseMat(JacMat, savedJ);
  }

  /* Scale and add I to get M = I - gamma*J */
  ScaleSparseMat(-gamma, JacMat);
  AddIdentitySparseMat(JacMat);

  if (cvsls_mem->s_first_factorize) {
    /* ------------------------------------------------------------
       Get the symbolic factorization
       ------------------------------------------------------------*/ 
    /* Update the ordering option with any user-updated values from 
       calls to CVKLUSetOrdering */
    klu_data->s_Common.ordering = klu_data->s_ordering;

    klu_data->s_Symbolic = klu_analyze(JacMat->N, JacMat->colptrs, 
				       JacMat->rowvals, &(klu_data->s_Common));
    if (klu_data->s_Symbolic == NULL) {
      cvProcessError(cv_mem, CVSLS_PACKAGE_FAIL, "CVSLS", "CVKLUSetup", 
		      MSGSP_PACKAGE_FAIL);
      return(CVSLS_PACKAGE_FAIL);
    }

    cvsls_mem->s_first_factorize = 0;
  }
  else {
    klu_free_numeric(&(klu_data->s_Numeric), &(klu_data->s_Common));
  }


  /* ------------------------------------------------------------
     Compute the LU factorization of  the Jacobian.
     ------------------------------------------------------------*/
  klu_data->s_Numeric = klu_factor(JacMat->colptrs, JacMat->rowvals, 
				   JacMat->data, 
				   klu_data->s_Symbolic, &(klu_data->s_Common));

  if (klu_data->s_Numeric == NULL) {
    cvProcessError(cv_mem, CVSLS_PACKAGE_FAIL, "CVSLS", "CVKLUSetup", 
		    MSGSP_PACKAGE_FAIL);
    return(CVSLS_PACKAGE_FAIL);
  }

  cvsls_mem->s_last_flag = CVSLS_SUCCESS;

  return(0);
}

/*
  This routine handles the solve operation for the CVKLU linear
  solver module.  It calls the KLU solve routine,
  then returns CVSLS_SUCCESS = 0.
*/

static int cvKLUSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
		      N_Vector ycur, N_Vector fcur)
{
  int flag, lmm;
  realtype gamrat;
  CVSlsMem cvsls_mem;
  KLUData klu_data;
  SlsMat JacMat;
  realtype *bd;
  
  gamrat = cv_mem->cv_gamrat;
  lmm = cv_mem->cv_lmm;
  cvsls_mem = (CVSlsMem) cv_mem->cv_lmem;
  JacMat = cvsls_mem->s_JacMat;
  klu_data = (KLUData) cvsls_mem->s_solver_data;
  bd = N_VGetArrayPointer(b);

  /* Call KLU to solve the linear system */
  flag = klu_solve(klu_data->s_Symbolic, klu_data->s_Numeric, JacMat->N, 1, bd, 
	    &(klu_data->s_Common));
  if (flag == 0) {
    cvProcessError(cv_mem, CVSLS_PACKAGE_FAIL, "CVSLS", "CVKLUSolve", 
		    MSGSP_PACKAGE_FAIL);
    return(CVSLS_PACKAGE_FAIL);
  }

  /* Scale the correction to account for change in gamma. */
  if ((lmm == CV_BDF) && (gamrat != ONE)) {
    N_VScale(TWO/(ONE + gamrat), b, b);
  }

  cvsls_mem->s_last_flag = CVSLS_SUCCESS;
  return(CVSLS_SUCCESS);
}

/*
  This routine frees memory specific to the CVKLU linear solver.
*/

static void cvKLUFree(CVodeMem cv_mem)
{
  CVSlsMem cvsls_mem;
  KLUData klu_data;
  
  cvsls_mem = (CVSlsMem) cv_mem->cv_lmem;
  klu_data = (KLUData) cvsls_mem->s_solver_data;

  klu_free_numeric(&(klu_data->s_Numeric), &(klu_data->s_Common));
  klu_free_symbolic(&(klu_data->s_Symbolic), &(klu_data->s_Common));

  free(klu_data->s_Symbolic);

  if (cvsls_mem->s_JacMat) {
    DestroySparseMat(cvsls_mem->s_JacMat);
    cvsls_mem->s_JacMat = NULL;
  }

  if (cvsls_mem->s_savedJ) {
    DestroySparseMat(cvsls_mem->s_savedJ);
    cvsls_mem->s_savedJ = NULL;
  }

  free(klu_data); 
  free(cv_mem->cv_lmem); 

  return;
}

/* 
 * -----------------------------------------------------------------
 * Optional Input Specification Functions
 * -----------------------------------------------------------------
 *
 * CVKLUSetOrdering sets the ordering used by KLU for reducing fill.
 * Options are: 0 for AMD, 1 for COLAMD, and 2 for the natural ordering.
 * The default used in CVODE is 1 for COLAMD.
 * -----------------------------------------------------------------
 */

int CVKLUSetOrdering(void *cv_mem_v, int ordering_choice)
{
  CVodeMem cv_mem;
  CVSlsMem cvsls_mem;
  KLUData klu_data;

 /* Return immediately if cv_mem is NULL */
  if (cv_mem_v == NULL) {
    cvProcessError(NULL, CVSLS_MEM_NULL, "CVSLS", "CVKLUSetOrdering",
		    MSGSP_CVMEM_NULL);
    return(CVSLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cv_mem_v;

 /* Return if ordering choice argument is not valid */
  if ( (ordering_choice != 0) && (ordering_choice != 1) && 
       (ordering_choice != 2) ) {
    cvProcessError(NULL, CVSLS_ILL_INPUT, "CVSLS", "CVKLUSetOrdering",
		    MSGSP_ILL_INPUT);
    return(CVSLS_ILL_INPUT);
  }

  cvsls_mem = (CVSlsMem) cv_mem->cv_lmem;
  klu_data = (KLUData) cvsls_mem->s_solver_data;

  klu_data->s_ordering = ordering_choice;

  return(CVSLS_SUCCESS);
}


/* 
 * ================================================================
 *
 *                   PART II - backward problems
 *
 * ================================================================
 */

/*
 * CVKLUB is a wrapper around CVKLU. It attaches the CVKLU linear solver
 * to the backward problem memory block.
 */

int CVKLUB(void *cvode_mem, int which, int n, int nnz)
{
  CVodeMem cv_mem;
  CVadjMem ca_mem;
  CVodeBMem cvB_mem;
  void *cvodeB_mem;
  CVSlsMemB cvslsB_mem;
  int flag;

  /* Check if cvode_mem exists */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVSLS_MEM_NULL, "CVSKLU", "CVKLUB", MSGSP_CVMEM_NULL);
    return(CVSLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  /* Was ASA initialized? */
  if (cv_mem->cv_adjMallocDone == FALSE) {
    cvProcessError(cv_mem, CVSLS_NO_ADJ, "CVSKLU", "CVKLUB", MSGSP_NO_ADJ);
    return(CVSLS_NO_ADJ);
  } 
  ca_mem = cv_mem->cv_adj_mem;

  /* Check which */
  if ( which >= ca_mem->ca_nbckpbs ) {
    cvProcessError(cv_mem, CVSLS_ILL_INPUT, "CVSKLU", "CVKLUB", MSGSP_BAD_WHICH);
    return(CVSLS_ILL_INPUT);
  }

  /* Find the CVodeBMem entry in the linked list corresponding to which */
  cvB_mem = ca_mem->cvB_mem;
  while (cvB_mem != NULL) {
    if ( which == cvB_mem->cv_index ) break;
    cvB_mem = cvB_mem->cv_next;
  }

  cvodeB_mem = (void *) (cvB_mem->cv_mem);

  /* Get memory for CVSlsMemRecB */
  cvslsB_mem = (CVSlsMemB) malloc(sizeof(struct CVSlsMemRecB));
  if (cvslsB_mem == NULL) {
    cvProcessError(cv_mem, CVSLS_MEM_FAIL, "CVSKLU", "CVKLUB", MSGSP_MEM_FAIL);
    return(CVSLS_MEM_FAIL);
  }

  /* initialize Jacobian function */
  cvslsB_mem->s_djacB = NULL;

  /* attach lmemB and lfreeB */
  cvB_mem->cv_lmem = cvslsB_mem;
  cvB_mem->cv_lfree = cvKLUFreeB;

  flag = CVKLU(cvodeB_mem, n, nnz);

  if (flag != CVSLS_SUCCESS) {
    free(cvslsB_mem);
    cvslsB_mem = NULL;
  }

  return(flag);
}

/*
 * cvKLUFreeB frees the memory associated with the CVSKLU linear
 * solver for backward integration.
 */

static void cvKLUFreeB(CVodeBMem cvB_mem)
{
  CVSlsMemB cvslsB_mem;

  cvslsB_mem = (CVSlsMemB) (cvB_mem->cv_lmem);

  free(cvslsB_mem);
}

