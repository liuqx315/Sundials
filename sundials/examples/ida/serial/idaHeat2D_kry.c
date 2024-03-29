/*
 * -----------------------------------------------------------------
 * $Revision: 4074 $
 * $Date: 2014-04-23 14:13:52 -0700 (Wed, 23 Apr 2014) $
 * -----------------------------------------------------------------
 * Programmer(s): Allan Taylor, Alan Hindmarsh and
 *                Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * Example problem for IDA: 2D heat equation, serial, GMRES.
 *
 * This example solves a discretized 2D heat equation problem.
 * This version uses the Krylov solver IDASpgmr.
 *
 * The DAE system solved is a spatial discretization of the PDE
 *          du/dt = d^2u/dx^2 + d^2u/dy^2
 * on the unit square. The boundary condition is u = 0 on all edges.
 * Initial conditions are given by u = 16 x (1 - x) y (1 - y). The
 * PDE is treated with central differences on a uniform M x M grid.
 * The values of u at the interior points satisfy ODEs, and
 * equations u = 0 at the boundaries are appended, to form a DAE
 * system of size N = M^2. Here M = 10.
 *
 * The system is solved with IDA using the Krylov linear solver
 * IDASPGMR. The preconditioner uses the diagonal elements of the
 * Jacobian only. Routines for preconditioning, required by
 * IDASPGMR, are supplied here. The constraints u >= 0 are posed
 * for all components. Output is taken at t = 0, .01, .02, .04,
 * ..., 10.24. Two cases are run -- with the Gram-Schmidt type
 * being Modified in the first case, and Classical in the second.
 * The second run uses IDAReInit and IDAReInitSpgmr.
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <ida/ida.h>
#include <ida/ida_spgmr.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>

/* Problem Constants */

#define NOUT  11
#define MGRID 10
#define NEQ   MGRID*MGRID
#define ZERO  RCONST(0.0)
#define ONE   RCONST(1.0)
#define TWO   RCONST(2.0)
#define FOUR  RCONST(4.0)

/* User data type */

typedef struct {  
  long int mm;  /* number of grid points */
  realtype dx;
  realtype coeff;
  N_Vector pp;  /* vector of prec. diag. elements */
} *UserData;

/* Prototypes for functions called by IDA */

int resHeat(realtype tres, N_Vector uu, N_Vector up,
            N_Vector resval, void *user_data);

int PsetupHeat(realtype tt, 
               N_Vector uu, N_Vector up, N_Vector rr, 
               realtype c_j, void *prec_data, 
               N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

int PsolveHeat(realtype tt, 
               N_Vector uu, N_Vector up, N_Vector rr, 
               N_Vector rvec, N_Vector zvec, 
               realtype c_j, realtype delta, void *prec_data, 
               N_Vector tmp);

/* Prototypes for private functions */

static int SetInitialProfile(UserData data, N_Vector uu, N_Vector up, 
                             N_Vector res);
static void PrintHeader(realtype rtol, realtype atol);
static void PrintOutput(void *mem, realtype t, N_Vector uu);
static int check_flag(void *flagvalue, char *funcname, int opt);

/*
 *--------------------------------------------------------------------
 * MAIN PROGRAM
 *--------------------------------------------------------------------
 */

int main()
{
  void *mem;
  UserData data;
  N_Vector uu, up, constraints, res;
  int ier, iout;
  realtype rtol, atol, t0, t1, tout, tret;
  long int netf, ncfn, ncfl;

  mem = NULL;
  data = NULL;
  uu = up = constraints = res = NULL;

  /* Allocate N-vectors and the user data structure. */

  uu = N_VNew_Serial(NEQ);
  if(check_flag((void *)uu, "N_VNew_Serial", 0)) return(1);

  up = N_VNew_Serial(NEQ);
  if(check_flag((void *)up, "N_VNew_Serial", 0)) return(1);

  res = N_VNew_Serial(NEQ);
  if(check_flag((void *)res, "N_VNew_Serial", 0)) return(1);

  constraints = N_VNew_Serial(NEQ);
  if(check_flag((void *)constraints, "N_VNew_Serial", 0)) return(1);

  data = (UserData) malloc(sizeof *data);
  data->pp = NULL;
  if(check_flag((void *)data, "malloc", 2)) return(1);

  /* Assign parameters in the user data structure. */

  data->mm  = MGRID;
  data->dx = ONE/(MGRID-ONE);
  data->coeff = ONE/(data->dx * data->dx);
  data->pp = N_VNew_Serial(NEQ);
  if(check_flag((void *)data->pp, "N_VNew_Serial", 0)) return(1);

  /* Initialize uu, up. */

  SetInitialProfile(data, uu, up, res);

  /* Set constraints to all 1's for nonnegative solution values. */

  N_VConst(ONE, constraints);

  /* Assign various parameters. */

  t0   = ZERO;
  t1   = RCONST(0.01);
  rtol = ZERO;
  atol = RCONST(1.0e-3); 

  /* Call IDACreate and IDAMalloc to initialize solution */

  mem = IDACreate();
  if(check_flag((void *)mem, "IDACreate", 0)) return(1);

  ier = IDASetUserData(mem, data);
  if(check_flag(&ier, "IDASetUserData", 1)) return(1);

  ier = IDASetConstraints(mem, constraints);
  if(check_flag(&ier, "IDASetConstraints", 1)) return(1);
  N_VDestroy_Serial(constraints);

  ier = IDAInit(mem, resHeat, t0, uu, up);
  if(check_flag(&ier, "IDAInit", 1)) return(1);

  ier = IDASStolerances(mem, rtol, atol);
  if(check_flag(&ier, "IDASStolerances", 1)) return(1);

  /* Call IDASpgmr to specify the linear solver. */

  ier = IDASpgmr(mem, 0);
  if(check_flag(&ier, "IDASpgmr", 1)) return(1);

  ier = IDASpilsSetPreconditioner(mem, PsetupHeat, PsolveHeat);
  if(check_flag(&ier, "IDASpilsSetPreconditioner", 1)) return(1);

  /* Print output heading. */
  PrintHeader(rtol, atol);
  
  /* 
   * -------------------------------------------------------------------------
   * CASE I 
   * -------------------------------------------------------------------------
   */
  
  /* Print case number, output table heading, and initial line of table. */

  printf("\n\nCase 1: gsytpe = MODIFIED_GS\n");
  printf("\n   Output Summary (umax = max-norm of solution) \n\n");
  printf("  time     umax       k  nst  nni  nje   nre   nreLS    h      npe nps\n" );
  printf("----------------------------------------------------------------------\n");

  /* Loop over output times, call IDASolve, and print results. */

  for (tout = t1,iout = 1; iout <= NOUT ; iout++, tout *= TWO) {
    ier = IDASolve(mem, tout, &tret, uu, up, IDA_NORMAL);
    if(check_flag(&ier, "IDASolve", 1)) return(1);
    PrintOutput(mem, tret, uu);
  }

  /* Print remaining counters. */

  ier = IDAGetNumErrTestFails(mem, &netf);
  check_flag(&ier, "IDAGetNumErrTestFails", 1);

  ier = IDAGetNumNonlinSolvConvFails(mem, &ncfn);
  check_flag(&ier, "IDAGetNumNonlinSolvConvFails", 1);

  ier = IDASpilsGetNumConvFails(mem, &ncfl);
  check_flag(&ier, "IDASpilsGetNumConvFails", 1);

  printf("\nError test failures            = %ld\n", netf);
  printf("Nonlinear convergence failures = %ld\n", ncfn);
  printf("Linear convergence failures    = %ld\n", ncfl);

  /* 
   * -------------------------------------------------------------------------
   * CASE II
   * -------------------------------------------------------------------------
   */
  
  /* Re-initialize uu, up. */

  SetInitialProfile(data, uu, up, res);
  
  /* Re-initialize IDA and IDASPGMR */

  ier = IDAReInit(mem, t0, uu, up);
  if(check_flag(&ier, "IDAReInit", 1)) return(1);
  
  ier = IDASpilsSetGSType(mem, CLASSICAL_GS);
  if(check_flag(&ier, "IDASpilsSetGSType",1)) return(1); 
  
  /* Print case number, output table heading, and initial line of table. */

  printf("\n\nCase 2: gstype = CLASSICAL_GS\n");
  printf("\n   Output Summary (umax = max-norm of solution) \n\n");
  printf("  time     umax       k  nst  nni  nje   nre   nreLS    h      npe nps\n" );
  printf("----------------------------------------------------------------------\n");

  /* Loop over output times, call IDASolve, and print results. */

  for (tout = t1,iout = 1; iout <= NOUT ; iout++, tout *= TWO) {
    ier = IDASolve(mem, tout, &tret, uu, up, IDA_NORMAL);
    if(check_flag(&ier, "IDASolve", 1)) return(1);
    PrintOutput(mem, tret, uu);
  }

  /* Print remaining counters. */

  ier = IDAGetNumErrTestFails(mem, &netf);
  check_flag(&ier, "IDAGetNumErrTestFails", 1);

  ier = IDAGetNumNonlinSolvConvFails(mem, &ncfn);
  check_flag(&ier, "IDAGetNumNonlinSolvConvFails", 1);

  ier = IDASpilsGetNumConvFails(mem, &ncfl);
  check_flag(&ier, "IDASpilsGetNumConvFails", 1);

  printf("\nError test failures            = %ld\n", netf);
  printf("Nonlinear convergence failures = %ld\n", ncfn);
  printf("Linear convergence failures    = %ld\n", ncfl);

  /* Free Memory */

  IDAFree(&mem);

  N_VDestroy_Serial(uu);
  N_VDestroy_Serial(up);
  N_VDestroy_Serial(res);

  N_VDestroy_Serial(data->pp);
  free(data);

  return(0);
}

/*
 *--------------------------------------------------------------------
 * FUNCTIONS CALLED BY IDA
 *--------------------------------------------------------------------
 */

/*
 * resHeat: heat equation system residual function (user-supplied)      
 * This uses 5-point central differencing on the interior points, and   
 * includes algebraic equations for the boundary values.                
 * So for each interior point, the residual component has the form      
 *    res_i = u'_i - (central difference)_i                             
 * while for each boundary point, it is res_i = u_i.                     
 */

int resHeat(realtype tt, 
            N_Vector uu, N_Vector up, N_Vector rr, 
            void *user_data)
{
  long int i, j, offset, loc, mm;
  realtype *uu_data, *up_data, *rr_data, coeff, dif1, dif2;
  UserData data;
  
  uu_data = NV_DATA_S(uu); 
  up_data = NV_DATA_S(up); 
  rr_data = NV_DATA_S(rr);

  data = (UserData) user_data;
  
  coeff = data->coeff;
  mm    = data->mm;
  
  /* Initialize rr to uu, to take care of boundary equations. */
  N_VScale(ONE, uu, rr);
  
  /* Loop over interior points; set res = up - (central difference). */
  for (j = 1; j < MGRID-1; j++) {
    offset = mm*j;
    for (i = 1; i < mm-1; i++) {
      loc = offset + i;
      dif1 = uu_data[loc-1]  + uu_data[loc+1]  - TWO * uu_data[loc];
      dif2 = uu_data[loc-mm] + uu_data[loc+mm] - TWO * uu_data[loc];
      rr_data[loc]= up_data[loc] - coeff * ( dif1 + dif2 );
    }
  }

  return(0);
}

/*
 * PsetupHeat: setup for diagonal preconditioner for idaHeat2D_kry.   
 *                                                                 
 * The optional user-supplied functions PsetupHeat and          
 * PsolveHeat together must define the left preconditoner        
 * matrix P approximating the system Jacobian matrix               
 *                   J = dF/du + cj*dF/du'                         
 * (where the DAE system is F(t,u,u') = 0), and solve the linear   
 * systems P z = r.   This is done in this case by keeping only    
 * the diagonal elements of the J matrix above, storing them as    
 * inverses in a vector pp, when computed in PsetupHeat, for    
 * subsequent use in PsolveHeat.                                 
 *                                                                 
 * In this instance, only cj and data (user data structure, with    
 * pp etc.) are used from the PsetupdHeat argument list.         
 */
  
int PsetupHeat(realtype tt, 
               N_Vector uu, N_Vector up, N_Vector rr, 
               realtype c_j, void *prec_data, 
               N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  
  long int i, j, offset, loc, mm;
  realtype *ppv, pelinv;
  UserData data;
  
  data = (UserData) prec_data;
  ppv = NV_DATA_S(data->pp);
  mm = data->mm;

  /* Initialize the entire vector to 1., then set the interior points to the
     correct value for preconditioning. */
  N_VConst(ONE,data->pp);
  
  /* Compute the inverse of the preconditioner diagonal elements. */
  pelinv = ONE/(c_j + FOUR*data->coeff); 
  
  for (j = 1; j < mm-1; j++) {
    offset = mm * j;
    for (i = 1; i < mm-1; i++) {
      loc = offset + i;
      ppv[loc] = pelinv;
    }
  }
  
  return(0);  
}

/*
 * PsolveHeat: solve preconditioner linear system.              
 * This routine multiplies the input vector rvec by the vector pp 
 * containing the inverse diagonal Jacobian elements (previously  
 * computed in PrecondHeateq), returning the result in zvec.      
 */

int PsolveHeat(realtype tt, 
               N_Vector uu, N_Vector up, N_Vector rr, 
               N_Vector rvec, N_Vector zvec, 
               realtype c_j, realtype delta, void *prec_data, 
               N_Vector tmp)
{
  UserData data;
  data = (UserData) prec_data;
  N_VProd(data->pp, rvec, zvec);
  return(0);
}

/*
 *--------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 *--------------------------------------------------------------------
 */

/*
 * SetInitialProfile: routine to initialize u and up vectors.
 */

static int SetInitialProfile(UserData data, N_Vector uu, N_Vector up, 
                             N_Vector res)
{
  long int mm, mm1, i, j, offset, loc;
  realtype xfact, yfact, *udata, *updata;

  mm = data->mm;

  udata = NV_DATA_S(uu);
  updata = NV_DATA_S(up);

  /* Initialize uu on all grid points. */ 
  mm1 = mm - 1;
  for (j = 0; j < mm; j++) {
    yfact = data->dx * j;
    offset = mm*j;
    for (i = 0;i < mm; i++) {
      xfact = data->dx * i;
      loc = offset + i;
      udata[loc] = RCONST(16.0) * xfact * (ONE - xfact) * yfact * (ONE - yfact);
    }
  }
  
  /* Initialize up vector to 0. */
  N_VConst(ZERO, up);

  /* resHeat sets res to negative of ODE RHS values at interior points. */
  resHeat(ZERO, uu, up, res, data);

  /* Copy -res into up to get correct interior initial up values. */
  N_VScale(-ONE, res, up);

  /* Set up at boundary points to zero. */
  for (j = 0; j < mm; j++) {
    offset = mm*j;
    for (i = 0; i < mm; i++) {
      loc = offset + i;
      if (j == 0 || j == mm1 || i == 0 || i == mm1 ) updata[loc] = ZERO;
    }
  }
  
  return(0);
  }

/*
 * Print first lines of output (problem description)
 */

static void PrintHeader(realtype rtol, realtype atol)
{
  printf("\nidaHeat2D_kry: Heat equation, serial example problem for IDA \n");
  printf("         Discretized heat equation on 2D unit square. \n");
  printf("         Zero boundary conditions,");
  printf(" polynomial initial conditions.\n");
  printf("         Mesh dimensions: %d x %d", MGRID, MGRID);
  printf("        Total system size: %d\n\n", NEQ);
#if defined(SUNDIALS_EXTENDED_PRECISION) 
  printf("Tolerance parameters:  rtol = %Lg   atol = %Lg\n", rtol, atol);
#elif defined(SUNDIALS_DOUBLE_PRECISION) 
  printf("Tolerance parameters:  rtol = %g   atol = %g\n", rtol, atol);
#else
  printf("Tolerance parameters:  rtol = %g   atol = %g\n", rtol, atol);
#endif
  printf("Constraints set to force all solution components >= 0. \n");
  printf("Linear solver: IDASPGMR, preconditioner using diagonal elements. \n");
}

/*
 * PrintOutput: print max norm of solution and current solver statistics
 */

static void PrintOutput(void *mem, realtype t, N_Vector uu)
{
  realtype hused, umax;
  long int nst, nni, nje, nre, nreLS, nli, npe, nps;
  int kused, ier;
  
  umax = N_VMaxNorm(uu);

  ier = IDAGetLastOrder(mem, &kused);
  check_flag(&ier, "IDAGetLastOrder", 1);
  ier = IDAGetNumSteps(mem, &nst);
  check_flag(&ier, "IDAGetNumSteps", 1);
  ier = IDAGetNumNonlinSolvIters(mem, &nni);
  check_flag(&ier, "IDAGetNumNonlinSolvIters", 1);
  ier = IDAGetNumResEvals(mem, &nre);
  check_flag(&ier, "IDAGetNumResEvals", 1);
  ier = IDAGetLastStep(mem, &hused);
  check_flag(&ier, "IDAGetLastStep", 1);
  ier = IDASpilsGetNumJtimesEvals(mem, &nje);
  check_flag(&ier, "IDASpilsGetNumJtimesEvals", 1);
  ier = IDASpilsGetNumLinIters(mem, &nli);
  check_flag(&ier, "IDASpilsGetNumLinIters", 1);
  ier = IDASpilsGetNumResEvals(mem, &nreLS);
  check_flag(&ier, "IDASpilsGetNumResEvals", 1);
  ier = IDASpilsGetNumPrecEvals(mem, &npe);
  check_flag(&ier, "IDASpilsGetPrecEvals", 1);
  ier = IDASpilsGetNumPrecSolves(mem, &nps);
  check_flag(&ier, "IDASpilsGetNumPrecSolves", 1);

#if defined(SUNDIALS_EXTENDED_PRECISION) 
  printf(" %5.2Lf %13.5Le  %d  %3ld  %3ld  %3ld  %4ld  %4ld  %9.2Le  %3ld %3ld\n",
         t, umax, kused, nst, nni, nje, nre, nreLS, hused, npe, nps);
#elif defined(SUNDIALS_DOUBLE_PRECISION) 
  printf(" %5.2f %13.5e  %d  %3ld  %3ld  %3ld  %4ld  %4ld  %9.2e  %3ld %3ld\n",
         t, umax, kused, nst, nni, nje, nre, nreLS, hused, npe, nps);
#else
  printf(" %5.2f %13.5e  %d  %3ld  %3ld  %3ld  %4ld  %4ld  %9.2e  %3ld %3ld\n",
         t, umax, kused, nst, nni, nje, nre, nreLS, hused, npe, nps);
#endif
}

/*
 * Check function return value...
 *   opt == 0 means SUNDIALS function allocates memory so check if
 *            returned NULL pointer
 *   opt == 1 means SUNDIALS function returns a flag so check if
 *            flag >= 0
 *   opt == 2 means function allocates memory so check if returned
 *            NULL pointer 
 */

static int check_flag(void *flagvalue, char *funcname, int opt)
{
  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, 
            "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n", 
            funcname);
    return(1);
  } else if (opt == 1) {
    /* Check if flag < 0 */
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr, 
              "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n", 
              funcname, *errflag);
      return(1); 
    }
  } else if (opt == 2 && flagvalue == NULL) {
    /* Check if function returned NULL pointer - no memory allocated */
    fprintf(stderr, 
            "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n", 
            funcname);
    return(1);
  }

  return(0);
}
