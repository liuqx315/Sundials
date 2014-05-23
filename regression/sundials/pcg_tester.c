/*----------------------------------------------------------------
 Programmer(s): Daniel R. Reynolds @ SMU
 ----------------------------------------------------------------
 Copyright (c) 2013, Southern Methodist University.
 All rights reserved.
 For details, see the LICENSE file.
 ----------------------------------------------------------------
 PCG test routine:
 
 The following test sets up and solves a symmetric linear system 
 using the PCG linear solver module in SUNDIALS.  We set up and 
 solve the system a few times, using different preconditioners to
 test the solver in a variety of situations.
----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_pcg.h>


/* User-supplied Functions Called by the Solver */
static int ATimes(void *A_data, N_Vector x, N_Vector b);
static int PSolve(void *P_data, N_Vector b, N_Vector x, int lr);


/* UserData structure */
typedef struct {  
  long int N;    /* size of system */
  realtype **A;  /* matrix */
  int pchoice;   /* choice of preconditioner */
} *UserData;


/* Main Program */
int main() {

  /* local variables */
  int i, j, iret, nli, nps;
  realtype res_norm;

  /* test problem parameters */
  long int N = 1000;
  int ndeltas = 6;
  realtype deltas[] = {RCONST(1e-2), RCONST(1e-4),  RCONST(1e-6), 
		       RCONST(1e-8), RCONST(1e-10), RCONST(1e-12)};

  /* allocate system and vectors */
  UserData udata = NULL;
  N_Vector b = NULL;
  N_Vector x = NULL;
  N_Vector xtrue = NULL;
  N_Vector error = NULL;
  N_Vector s = NULL;
  PcgMem pcg_mem = NULL;
  udata = (UserData) malloc(sizeof(*udata));
  udata->N = N;
  udata->pchoice = 0;
  udata->A = malloc(N * sizeof(realtype*));
  for (i=0; i<N; i++)  udata->A[i] = malloc(N * sizeof(realtype));
  b = N_VNew_Serial(N);
  x = N_VNew_Serial(N);
  s = N_VNew_Serial(N);
  xtrue = N_VNew_Serial(N);
  error = N_VNew_Serial(N);

  /* Set up matrix and vectors */
  for (i=0; i<N; i++) {
    NV_Ith_S(xtrue,i) = 4.0*i*(N-i)/N/N;
    for (j=0; j<N; j++) 
      udata->A[i][j] = 2.0/(j+i+1);
    udata->A[i][i] += 10.0 + 1.0*i/N;
  }
  N_VConst(1.0, s);
  ATimes((void *) udata, xtrue, b);

  /* Call PcgMalloc to create the solver memory */
  pcg_mem = PcgMalloc(200, x);

  /* Loop over tolerances, calling various solvers */
  for (j=0; j<ndeltas; j++) {

    /* Call solver with preconditioner disabled */
    for (i=0; i<N; i++)  NV_Ith_S(x,i) = 0.0;
    printf("\nCalling solver with P disabled, tol = %g\n", deltas[j]);
    iret = PcgSolve(pcg_mem, (void *) udata, x, b, PREC_NONE, deltas[j], 
		    (void *) udata, s, ATimes, PSolve, &res_norm, &nli, &nps);
    if (iret < 0)  printf("  PcgSolve Error = %i\n", iret);
    else           printf("  PcgSolve Success = %i\n", iret);
    printf("  res_norm = %g\n", res_norm);
    printf("  nli = %i\n", nli);
    printf("  nps = %i\n", nps);
    N_VLinearSum(1.0, x, -1.0, xtrue, error);
    printf("  error norm = %g\n", RSqrt(N_VDotProd(error,error)));

    /* Call solver with no preconditioning */
    for (i=0; i<N; i++)  NV_Ith_S(x,i) = 0.0;
    udata->pchoice = 0;
    printf("\nCalling solver with no P, tol = %g\n", deltas[j]);
    iret = PcgSolve(pcg_mem, (void *) udata, x, b, PREC_LEFT, deltas[j], 
		    (void *) udata, s, ATimes, PSolve, &res_norm, &nli, &nps);
    if (iret < 0)  printf("  PcgSolve Error = %i\n", iret);
    else           printf("  PcgSolve Success = %i\n", iret);
    printf("  res_norm = %g\n", res_norm);
    printf("  nli = %i\n", nli);
    printf("  nps = %i\n", nps);
    N_VLinearSum(1.0, x, -1.0, xtrue, error);
    printf("  error norm = %g\n", RSqrt(N_VDotProd(error,error)));

    /* Call solver with scaled preconditioning */
    for (i=0; i<N; i++)  NV_Ith_S(x,i) = 0.0;
    udata->pchoice = 1;
    printf("\nCalling solver with scaled P, tol = %g\n", deltas[j]);
    iret = PcgSolve(pcg_mem, (void *) udata, x, b, PREC_LEFT, deltas[j], 
		    (void *) udata, s, ATimes, PSolve, &res_norm, &nli, &nps);
    if (iret < 0)  printf("  PcgSolve Error = %i\n", iret);
    else           printf("  PcgSolve Success = %i\n", iret);
    printf("  res_norm = %g\n", res_norm);
    printf("  nli = %i\n", nli);
    printf("  nps = %i\n", nps);
    N_VLinearSum(1.0, x, -1.0, xtrue, error);
    printf("  error norm = %g\n", RSqrt(N_VDotProd(error,error)));

    /* Call solver with Jacobi preconditioning */
    for (i=0; i<N; i++)  NV_Ith_S(x,i) = 0.0;
    udata->pchoice = 2;
    printf("\nCalling solver with Jacobi P, tol = %g\n", deltas[j]);
    iret = PcgSolve(pcg_mem, (void *) udata, x, b, PREC_LEFT, deltas[j], 
		    (void *) udata, s, ATimes, PSolve, &res_norm, &nli, &nps);
    if (iret < 0)  printf("  PcgSolve Error = %i\n", iret);
    else           printf("  PcgSolve Success = %i\n", iret);
    printf("  res_norm = %g\n", res_norm);
    printf("  nli = %i\n", nli);
    printf("  nps = %i\n", nps);
    N_VLinearSum(1.0, x, -1.0, xtrue, error);
    printf("  error norm = %g\n", RSqrt(N_VDotProd(error,error)));

  } /* for j */
  
  /* clean up */
  free(udata);
  N_VDestroy_Serial(b);
  N_VDestroy_Serial(x);
  N_VDestroy_Serial(xtrue);
  N_VDestroy_Serial(error);
  PcgFree(pcg_mem);

  return 0;
}


/*-------------------------------
  Functions called by the solver
  ------------------------------*/

/* matrix-vector product routine */
static int ATimes(void *A_data, N_Vector xvec, N_Vector bvec) {

  /* local variables */
  realtype *x = N_VGetArrayPointer(xvec);
  realtype *b = N_VGetArrayPointer(bvec);
  realtype **A = ((UserData) A_data)->A;
  long int N = ((UserData) A_data)->N;
  long int i, j;

  /* perform product */
  for (i=0; i<N; i++) {
    b[i] = 0.0;
    for (j=0; j<N; j++) 
      b[i] += A[i][j] * x[j];
  }
  return 0;
}


/* preconditioner solve routine */
static int PSolve(void *P_data, N_Vector bvec, N_Vector xvec, int lr) {

  /* local variables */
  realtype *x = N_VGetArrayPointer(xvec);
  realtype *b = N_VGetArrayPointer(bvec);
  realtype **A = ((UserData) P_data)->A;
  long int N = ((UserData) P_data)->N;
  int pchoice = ((UserData) P_data)->pchoice;
  long int i;

  /* perform preconditioner */

  if (pchoice == 0) { /* no preconditoning */
    N_VScale(1.0, bvec, xvec);
    return 0;
  }
			
  if (pchoice == 1) { /* scaled preconditioning */
    N_VScale(1.0/12.0, bvec, xvec);
    return 0;
  }

  if (pchoice == 2) { /* Jacobi preconditioning */
    for (i=0; i<N; i++)  x[i] = b[i] / A[i][i];
    return 0;
  }

  /* illegal pchoice */
  return 1;
}



/*---- end of file ----*/