/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * ----------------------------------------------------------------- 
 * Programmer(s): Radu Serban and Cosmin Petra @ LLNL
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
 *
 * Hessian using adjoint sensitivity example problem. 
 * 
 * This simple example problem for IDAS, due to Robertson, 
 * is from chemical kinetics, and consists of the following three 
 * equations:
 *
 *   [ y1' + p1 * y1 - p2 * y2 * y3              = 0 
 *   [ y2' - p1 * y1 + p2 * y2 * y3 + p3 * y2^2  = 0 
 *   [ y1 + y2 + y3 -1                               = 0 
 * 
 *        [1]        [-p1]
 *   y(0)=[0]  y'(0)=[ p1]   p1 = 0.04   p2 = 1e4   p3 = 1e07   
 *        [0]        [ 0 ]
 *
 *       80
 *      / 
 *  G = | 0.5 * (y1^2 + y2^2 + y3^2) dt
 *      /
 *      0
 * Compute the gradient (using FSA and ASA) and Hessian (FSA over ASA)
 * of G with respect to parameters p1 and p2.
 *
 * Reference: D.B. Ozyurt and P.I. Barton, SISC 26(5) 1725-1743, 2005.
 *
 * Error handling was suppressed for code readibility reasons.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <idas/idas.h>
#include <idas/idas_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>

/* Accessor macros */

#define Ith(v,i)    NV_Ith_S(v,i-1)       /* i-th vector component i= 1..NEQ */
#define IJth(A,i,j) DENSE_ELEM(A,i-1,j-1) /* (i,j)-th matrix component i,j = 1..NEQ */

/* Problem Constants */
#define NEQ      3             /* number of equations                  */
#define NP       2             /* number of sensitivities              */

#define T0       RCONST(0.0)   /* Initial time. */
#define TF       RCONST(80.0)  /* Final time. */

/* Tolerances */
#define RTOL     RCONST(1e-08) /* scalar relative tolerance            */
#define ATOL     RCONST(1e-10) /* vector absolute tolerance components */
#define RTOLA    RCONST(1e-08) /* for adjoint integration              */
#define ATOLA    RCONST(1e-08) /* for adjoint integration              */

/* Parameters */
#define P1 RCONST(0.04)
#define P2 RCONST(1.0e4)
#define P3 RCONST(3.0e7)

/* Predefined consts */
#define HALF RCONST(0.5)
#define ZERO RCONST(0.0)
#define ONE  RCONST(1.0)
#define TWO  RCONST(2.0)

/* User defined struct */
typedef struct { 
  realtype p[3];
} *UserData;

/* residual for forward problem */
static int res(realtype t, N_Vector yy, N_Vector yp, 
               N_Vector resval, void *user_data);

static int resS(int Ns, realtype t, 
                N_Vector yy, N_Vector yp, N_Vector resval,
                N_Vector *yyS, N_Vector *ypS, N_Vector *resvalS,
                void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

static int rhsQ(realtype t, N_Vector yy, N_Vector yp, N_Vector qdot, void *user_data);

static int rhsQS(int Ns, realtype t, N_Vector yy, N_Vector yp, 
                 N_Vector *yyS, N_Vector *ypS, N_Vector rrQ, N_Vector *rhsQS,
                 void *user_data,  N_Vector yytmp, N_Vector yptmp, N_Vector tmpQS);

static int resBS1(realtype tt, N_Vector yy, N_Vector yp, N_Vector *yyS, N_Vector *ypS,
                  N_Vector yyB, N_Vector ypB, N_Vector resvalBQ, void *user_dataB);


static int rhsQBS1(realtype tt, N_Vector yy, N_Vector yp,
                   N_Vector *yyS, N_Vector *ypS, N_Vector yyB, N_Vector ypB,
                   N_Vector rhsBQS, void *user_dataB);

static int resBS2(realtype tt, N_Vector yy, N_Vector yp, N_Vector *yyS, N_Vector *ypS,
                  N_Vector yyB, N_Vector ypB, N_Vector resvalBQ, void *user_dataB);


static int rhsQBS2(realtype tt, N_Vector yy, N_Vector yp,
                   N_Vector *yyS, N_Vector *ypS, N_Vector yyB, N_Vector ypB,
                   N_Vector rhsBQS, void *user_dataB);


int main(int argc, char *argv[])
{
  N_Vector yy, yp, q, *yyS, *ypS, *qS; 
  N_Vector yyB1, ypB1, qB1, yyB2, ypB2, qB2;
  void *ida_mem;
  UserData data;
  realtype time, ti, tf;
  int flag, nckp, indexB1, indexB2;
  realtype G, Gm, Gp, dp1, dp2, grdG_fwd[2], grdG_bck[2], grdG_cntr[2], H11, H22;
  realtype rtolFD, atolFD;


  /* Print problem description */
  printf("\nAdjoint Sensitivity Example for Chemical Kinetics\n");
  printf("---------------------------------------------------------\n");
  printf("DAE: dy1/dt + p1*y1 - p2*y2*y3 = 0\n");
  printf("     dy2/dt - p1*y1 + p2*y2*y3 + p3*(y2)^2 = 0\n");
  printf("               y1  +  y2  +  y3 = 0\n\n");
  printf("Find dG/dp and d^2G/dp^2, where p=[p1,p2] for\n");
  printf("     G = int_t0^tB0 g(t,p,y) dt\n");
  printf("     g(t,p,y) = y3\n\n\n");

  /* Alocate and initialize user data. */
  data = (UserData) malloc(sizeof(*data));
  data->p[0] = P1; data->p[1] = P2; data->p[2] = P3;

  /* Consistent IC */
  yy = N_VNew_Serial(NEQ);
  yp = N_VNew_Serial(NEQ);
  Ith(yy,1) = ONE; Ith(yy,2) = ZERO; Ith(yy,3) = ZERO;
  Ith(yp,1) = -P1; Ith(yp,2) = P1; Ith(yp,3) = 0;

  q = N_VNew_Serial(1);
  N_VConst(ZERO, q);

  yyS = N_VCloneVectorArray_Serial(NP, yy);
  ypS = N_VCloneVectorArray_Serial(NP, yp);
  N_VConst(ZERO, yyS[0]); N_VConst(ZERO, yyS[1]);
  N_VConst(ZERO, ypS[0]); N_VConst(ZERO, ypS[1]);

  qS = N_VCloneVectorArray_Serial(NP, q);
  N_VConst(ZERO, qS[0]);

  ida_mem = IDACreate();

  ti = T0;
  flag = IDAInit(ida_mem, res, ti, yy, yp);

  /* Forward problem's setup. */
  flag = IDASStolerances(ida_mem, RTOL, ATOL);
  flag = IDADense(ida_mem, NEQ);
  flag = IDASetUserData(ida_mem, data);
  flag = IDASetMaxNumSteps(ida_mem, 1500);

  /* Quadrature's setup. */
  flag = IDAQuadInit(ida_mem, rhsQ, q);
  flag = IDAQuadSStolerances(ida_mem, RTOL, ATOL);
  flag = IDASetQuadErrCon(ida_mem, TRUE);

  /* Sensitivity's setup. */
  flag = IDASensInit(ida_mem, NP, IDA_SIMULTANEOUS, resS, yyS, ypS);
  flag = IDASensEEtolerances(ida_mem);
  flag = IDASetSensErrCon(ida_mem, TRUE);

  /* Setup of quadrature's sensitivities */
  flag = IDAQuadSensInit(ida_mem, rhsQS, qS);
  flag = IDAQuadSensEEtolerances(ida_mem);
  flag = IDASetQuadSensErrCon(ida_mem, TRUE); 
  
  /* Initialize ASA. */
  flag = IDAAdjInit(ida_mem, 100, IDA_HERMITE);

  printf("---------------------------------------------------------\n");
  printf("Forward integration\n");
  printf("---------------------------------------------------------\n\n");

  tf = TF;
  flag = IDASolveF(ida_mem, tf, &time, yy, yp, IDA_NORMAL, &nckp);

  IDAGetQuad(ida_mem, &time, q);
  G = Ith(q,1);
  printf("     G:    %12.4e\n", G);

  /* Sensitivities are needed for IC of backward problems. */
  IDAGetSensDky(ida_mem, tf, 0, yyS);
  IDAGetSensDky(ida_mem, tf, 1, ypS);

  IDAGetQuadSens(ida_mem, &time, qS);
  printf("   dG/dp:  %12.4e %12.4e\n", Ith(qS[0],1), Ith(qS[1],1));
  printf("\n");
  /******************************
  * BACKWARD PROBLEM #1
  *******************************/

  /* Consistent IC. */
  yyB1 = N_VNew_Serial(2*NEQ);
  ypB1 = N_VNew_Serial(2*NEQ);

  N_VConst(ZERO, yyB1);
  Ith(yyB1,3) = Ith(yy,3);
  Ith(yyB1,6) = Ith(yyS[0], 3);

  N_VConst(ZERO, ypB1);
  Ith(ypB1,1) = Ith(yy,3)-Ith(yy,1);
  Ith(ypB1,2) = Ith(yy,3)-Ith(yy,2);
  Ith(ypB1,4) = Ith(yyS[0],3) - Ith(yyS[0],1);
  Ith(ypB1,5) = Ith(yyS[0],3) - Ith(yyS[0],2);
  
  qB1 = N_VNew_Serial(2*NP);
  N_VConst(ZERO, qB1);

  flag = IDACreateB(ida_mem, &indexB1);
  flag = IDAInitBS(ida_mem, indexB1, resBS1, tf, yyB1, ypB1);
  flag = IDASStolerancesB(ida_mem, indexB1, RTOLA, ATOLA);   
  flag = IDASetUserDataB(ida_mem, indexB1, data);
  flag = IDASetMaxNumStepsB(ida_mem, indexB1, 5000);
  flag = IDADenseB(ida_mem, indexB1, 2*NEQ);
  flag = IDAQuadInitBS(ida_mem, indexB1, rhsQBS1, qB1);

  /******************************
  * BACKWARD PROBLEM #2  
  *******************************/

  /* Consistent IC. */
  yyB2 = N_VNew_Serial(2*NEQ);
  ypB2 = N_VNew_Serial(2*NEQ);

  N_VConst(ZERO, yyB2);
  Ith(yyB2,3) = Ith(yy,3);
  Ith(yyB2,6) = Ith(yyS[1],3);

  N_VConst(ZERO, ypB2);
  Ith(ypB2,1) = Ith(yy,3)-Ith(yy,1);
  Ith(ypB2,2) = Ith(yy,3)-Ith(yy,2);
  Ith(ypB2,4) = Ith(yyS[1],3) - Ith(yyS[1],1);
  Ith(ypB2,5) = Ith(yyS[1],3) - Ith(yyS[1],2);
  
  qB2 = N_VNew_Serial(2*NP);
  N_VConst(ZERO, qB2);

  flag = IDACreateB(ida_mem, &indexB2);
  flag = IDAInitBS(ida_mem, indexB2, resBS2, tf, yyB2, ypB2);
  flag = IDASStolerancesB(ida_mem, indexB2, RTOLA, ATOLA);   
  flag = IDASetUserDataB(ida_mem, indexB2, data);
  flag = IDASetMaxNumStepsB(ida_mem, indexB2, 2500);
  flag = IDADenseB(ida_mem, indexB2, 2*NEQ);
  flag = IDAQuadInitBS(ida_mem, indexB2, rhsQBS2, qB2);

  /* Integrate backward problems. */
  printf("---------------------------------------------------------\n");
  printf("Backward integration \n");
  printf("---------------------------------------------------------\n\n"); 

  flag = IDASolveB(ida_mem, ti, IDA_NORMAL);

  flag = IDAGetB(ida_mem, indexB1, &time, yyB1, ypB1);
  /* 
     flag = IDAGetNumSteps(IDAGetAdjIDABmem(ida_mem, indexB1), &nst);
     printf("at time=%g \tpb 1 Num steps:%d\n", time, nst); 
     flag = IDAGetNumSteps(IDAGetAdjIDABmem(ida_mem, indexB2), &nst);
     printf("at time=%g \tpb 2 Num steps:%d\n\n", time, nst); 
  */

  flag = IDAGetQuadB(ida_mem, indexB1, &time, qB1);
  flag = IDAGetQuadB(ida_mem, indexB2, &time, qB2);
  printf("   dG/dp:  %12.4e %12.4e   (from backward pb. 1)\n", Ith(qB1,1), Ith(qB1,2));
  printf("   dG/dp:  %12.4e %12.4e   (from backward pb. 2)\n", Ith(qB2,1), Ith(qB2,2));

  printf("\n");
  printf("   H = d2G/dp2:\n");
  printf("        (1)            (2)\n");
  printf("  %12.4e  %12.4e\n", Ith(qB1,3), Ith(qB2,3));
  printf("  %12.4e  %12.4e\n", Ith(qB1,4), Ith(qB2,4));

  IDAFree(&ida_mem);

  /*********************************
  * Use Finite Differences to verify
  **********************************/

  /* Perturbations are of different magnitudes as p1 and p2 are. */
  dp1 = RCONST(1.0e-3);
  dp2 = RCONST(2.5e+2);

  printf("\n");
  printf("---------------------------------------------------------\n");
  printf("Finite Differences ( dp1=%6.1e and dp2 = %6.1e )\n", dp1, dp2);
  printf("---------------------------------------------------------\n\n");

  ida_mem = IDACreate();

  /********************
  * Forward FD for p1
  ********************/
  data->p[0] += dp1;

  Ith(yy,1) = ONE; Ith(yy,2) = ZERO; Ith(yy,3) = ZERO;
  Ith(yp,1) = -data->p[0]; Ith(yp,2) = -Ith(yp,1); Ith(yp,3) = 0;
  N_VConst(ZERO, q);
  ti = T0;
  tf = TF;

  flag = IDAInit(ida_mem, res, ti, yy, yp);

  rtolFD = RCONST(1.0e-12);
  atolFD = RCONST(1.0e-14);

  flag = IDASStolerances(ida_mem, rtolFD, atolFD);
  flag = IDADense(ida_mem, NEQ);
  flag = IDASetUserData(ida_mem, data);
  flag = IDASetMaxNumSteps(ida_mem, 10000);

  flag = IDAQuadInit(ida_mem, rhsQ, q);
  flag = IDAQuadSStolerances(ida_mem, rtolFD, atolFD);
  flag = IDASetQuadErrCon(ida_mem, TRUE);

  flag = IDASolve(ida_mem, tf, &time, yy, yp, IDA_NORMAL);
  flag = IDAGetQuad(ida_mem, &time, q);
  Gp = Ith(q,1);

  /********************
  * Backward FD for p1
  ********************/
  data->p[0] -= 2*dp1;

  Ith(yy,1) = ONE; Ith(yy,2) = ZERO; Ith(yy,3) = ZERO;
  Ith(yp,1) = -data->p[0]; Ith(yp,2) = -Ith(yp,1); Ith(yp,3) = 0;
  N_VConst(ZERO, q);
  
  flag = IDAReInit(ida_mem, ti, yy, yp);
  flag = IDAQuadReInit(ida_mem, q);

  flag = IDASolve(ida_mem, tf, &time, yy, yp, IDA_NORMAL);
  flag = IDAGetQuad(ida_mem, &time, q);
  Gm = Ith(q,1);

  /* Compute FD for p1. */
  grdG_fwd[0] = (Gp-G)/dp1;
  grdG_bck[0] = (G-Gm)/dp1;
  grdG_cntr[0] = (Gp-Gm)/(2.0*dp1);
  H11 = (Gp - 2.0*G + Gm) / (dp1*dp1);

  /********************
  * Forward FD for p2
  ********************/
  /*restore p1*/
  data->p[0] += dp1; 
  data->p[1] += dp2;

  Ith(yy,1) = ONE; Ith(yy,2) = ZERO; Ith(yy,3) = ZERO;
  Ith(yp,1) = -data->p[0]; Ith(yp,2) = -Ith(yp,1); Ith(yp,3) = 0;
  N_VConst(ZERO, q);

  flag = IDAReInit(ida_mem, ti, yy, yp);
  flag = IDAQuadReInit(ida_mem, q);

  flag = IDASolve(ida_mem, tf, &time, yy, yp, IDA_NORMAL);
  flag = IDAGetQuad(ida_mem, &time, q);
  Gp = Ith(q,1);

  /********************
  * Backward FD for p2
  ********************/
  data->p[1] -= 2*dp2;

  Ith(yy,1) = ONE; Ith(yy,2) = ZERO; Ith(yy,3) = ZERO;
  Ith(yp,1) = -data->p[0]; Ith(yp,2) = -Ith(yp,1); Ith(yp,3) = 0;
  N_VConst(ZERO, q);
  
  flag = IDAReInit(ida_mem, ti, yy, yp);
  flag = IDAQuadReInit(ida_mem, q);

  flag = IDASolve(ida_mem, tf, &time, yy, yp, IDA_NORMAL);
  flag = IDAGetQuad(ida_mem, &time, q);
  Gm = Ith(q,1);

  /* Compute FD for p2. */
  grdG_fwd[1] = (Gp-G)/dp2;
  grdG_bck[1] = (G-Gm)/dp2;
  grdG_cntr[1] = (Gp-Gm)/(2.0*dp2);
  H22 = (Gp - 2.0*G + Gm) / (dp2*dp2);


  printf("\n");
  printf("   dG/dp:  %12.4e  %12.4e   (fwd FD)\n",  grdG_fwd[0],  grdG_fwd[1]);
  printf("           %12.4e  %12.4e   (bck FD)\n",  grdG_bck[0],  grdG_bck[1]);
  printf("           %12.4e  %12.4e   (cntr FD)\n", grdG_cntr[0], grdG_cntr[1]);
  printf("\n");
  printf("  H(1,1):  %12.4e\n", H11);
  printf("  H(2,2):  %12.4e\n", H22);

  IDAFree(&ida_mem);

  N_VDestroy_Serial(yyB1);
  N_VDestroy_Serial(ypB1);
  N_VDestroy_Serial(qB1);

  N_VDestroy_Serial(yyB2);
  N_VDestroy_Serial(ypB2);
  N_VDestroy_Serial(qB2);


  N_VDestroy_Serial(yy);
  N_VDestroy_Serial(yp);
  N_VDestroy_Serial(q);
  N_VDestroyVectorArray_Serial(yyS, NP);
  N_VDestroyVectorArray_Serial(ypS, NP);
  N_VDestroyVectorArray_Serial(qS, NP);

  free(data);
  return 0;
}



static int res(realtype tres, N_Vector yy, N_Vector yp, N_Vector rr, void *user_data)
{
  realtype y1, y2, y3, yp1, yp2, yp3, *rval;
  UserData data;
  realtype p1, p2, p3;

  y1  = Ith(yy,1); y2  = Ith(yy,2); y3  = Ith(yy,3); 
  yp1 = Ith(yp,1); yp2 = Ith(yp,2); yp3 = Ith(yp,3);
  rval = NV_DATA_S(rr);

  data = (UserData) user_data;
  p1 = data->p[0]; p2 = data->p[1]; p3 = data->p[2];

  rval[0] = p1*y1-p2*y2*y3;
  rval[1] = -rval[0] + p3*y2*y2 + yp2;
  rval[0]+= yp1;
  rval[2] = y1+y2+y3-1;

  return(0);
}

static int resS(int Ns, realtype t, 
                N_Vector yy, N_Vector yp, N_Vector resval,
                N_Vector *yyS, N_Vector *ypS, N_Vector *resvalS,
                void *user_data, 
                N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  UserData data;
  realtype p1, p2, p3;
  realtype y1, y2, y3;
  realtype yp1, yp2, yp3;
  realtype s1, s2, s3;
  realtype sd1, sd2, sd3;
  realtype rs1, rs2, rs3;
  int is;

  data = (UserData) user_data;
  p1 = data->p[0];
  p2 = data->p[1];
  p3 = data->p[2];

  y1 = Ith(yy,1);
  y2 = Ith(yy,2);
  y3 = Ith(yy,3);

  yp1 = Ith(yp,1);
  yp2 = Ith(yp,2);
  yp3 = Ith(yp,3);

  for (is=0; is<NP; is++) {

    s1 = Ith(yyS[is],1);
    s2 = Ith(yyS[is],2);
    s3 = Ith(yyS[is],3);

    sd1 = Ith(ypS[is],1);
    sd2 = Ith(ypS[is],2);
    sd3 = Ith(ypS[is],3);

    rs1 = sd1 + p1*s1 - p2*y3*s2 - p2*y2*s3;
    rs2 = sd2 - p1*s1 + p2*y3*s2 + p2*y2*s3 + TWO*p3*y2*s2;
    rs3 = s1 + s2 + s3;

    switch (is) {
    case 0:
      rs1 += y1;
      rs2 -= y1;
      break;
    case 1:
      rs1 -= y2*y3;
      rs2 += y2*y3;
      break;
    }
  
    Ith(resvalS[is],1) = rs1;
    Ith(resvalS[is],2) = rs2;
    Ith(resvalS[is],3) = rs3;

  }

  return(0);
}

static int rhsQ(realtype t, N_Vector yy, N_Vector yp, N_Vector qdot, void *user_data)
{

  realtype y1, y2, y3;

  y1 = Ith(yy,1); y2 = Ith(yy,2); y3 = Ith(yy,3); 
  Ith(qdot,1) = HALF*(y1*y1+y2*y2+y3*y3);

  return(0);
}

static int rhsQS(int Ns, realtype t,
                 N_Vector yy, N_Vector yp, 
                 N_Vector *yyS, N_Vector *ypS, 
                 N_Vector rrQ, N_Vector *rhsQS,
                 void *user_data,
                 N_Vector yytmp, N_Vector yptmp, N_Vector tmpQS)
{

  realtype y1, y2, y3;
  realtype s1, s2, s3;

  y1 = Ith(yy,1); 
  y2 = Ith(yy,2); 
  y3 = Ith(yy,3);

  /* 1st sensitivity RHS */
  s1 = Ith(yyS[0],1);
  s2 = Ith(yyS[0],2);
  s3 = Ith(yyS[0],3);
  Ith(rhsQS[0],1) = y1*s1 + y2*s2 + y3*s3;

  /* 2nd sensitivity RHS */
  s1 = Ith(yyS[1],1);
  s2 = Ith(yyS[1],2);
  s3 = Ith(yyS[1],3);
  Ith(rhsQS[1],1) = y1*s1 + y2*s2 + y3*s3;

  return(0);
}

/* Residuals for adjoint model. */
static int resBS1(realtype tt, 
                  N_Vector yy, N_Vector yp, 
                  N_Vector *yyS, N_Vector *ypS,
                  N_Vector yyB, N_Vector ypB,
                  N_Vector rrBS, void *user_dataB)

{
  UserData data;
  realtype y1, y2, y3;
  realtype p1, p2, p3;
  realtype l1, l2, l3, m1, m2, m3;
  realtype lp1, lp2, mp1, mp2;
  realtype s1, s2, s3;
  realtype l21;
  
  data = (UserData) user_dataB;

  /* The parameters. */
  p1 = data->p[0]; p2 = data->p[1]; p3 = data->p[2];

  /* The y vector. */
  y1 = Ith(yy,1); y2 = Ith(yy,2); y3 = Ith(yy,3);

  /* The lambda vector. */
  l1 = Ith(yyB,1); l2 = Ith(yyB,2); l3 = Ith(yyB,3);
  /* The mu vector. */
  m1 = Ith(yyB,4); m2 = Ith(yyB,5); m3 = Ith(yyB,6);

  /* The lambda dot vector. */
  lp1 = Ith(ypB,1); lp2 = Ith(ypB,2);
  /* The mu dot vector. */
  mp1 = Ith(ypB,4); mp2 = Ith(ypB,5);

  /* The sensitivity with respect to p1 */
  s1 = Ith(yyS[0],1); s2 = Ith(yyS[0],2); s3 = Ith(yyS[0],3);

  /* Temporary variables */
  l21 = l2-l1;

  Ith(rrBS,1) = lp1 + p1*l21 - l3 + y1;
  Ith(rrBS,2) = lp2 - p2*y3*l21 - TWO*p3*y2*l2 - l3 + y2;
  Ith(rrBS,3) = -p2*y2*l21 - l3 + y3;

  Ith(rrBS,4) = mp1 + p1*(-m1+m2) - m3 + l21 + s1;
  Ith(rrBS,5) = mp2 + p2*y3*m1 - (p2*y3+TWO*p3*y2)*m2 - m3 + p2*s3*l1 - (TWO*p3*s2+p2*s3)*l2 + s2;
  Ith(rrBS,6) = p2*y2*(m1-m2) - m3 - p2*s2*l21 + s3;

  return(0);
}

static int rhsQBS1(realtype tt, 
                 N_Vector yy, N_Vector yp,
                 N_Vector *yyS, N_Vector *ypS,
                 N_Vector yyB, N_Vector ypB,
                 N_Vector rhsBQS, void *user_dataB)
{
  UserData data;
  realtype y1, y2, y3;
  realtype p1, p2, p3;
  realtype l1, l2, l3, m1, m2, m3;
  realtype s1, s2, s3;
  realtype l21;
  
  data = (UserData) user_dataB;

  /* The p vector */
  p1 = data->p[0]; p2 = data->p[1]; p3 = data->p[2];

  /* The y vector */
  y1 = Ith(yy,1); y2 = Ith(yy,2); y3 = Ith(yy,3);
  
  /* The lambda vector. */
  l1 = Ith(yyB,1); l2 = Ith(yyB,2); l3 = Ith(yyB,3);
  /* The mu vector. */
  m1 = Ith(yyB,4); m2 = Ith(yyB,5); m3 = Ith(yyB,6);

  /* The sensitivity with respect to p1 */
  s1 = Ith(yyS[0],1); s2 = Ith(yyS[0],2); s3 = Ith(yyS[0],3);
  
  /* Temporary variables */
  l21 = l2-l1;

  Ith(rhsBQS,1) = -y1*l21;
  Ith(rhsBQS,2) = y2*y3*l21;

  Ith(rhsBQS,3) = y1*(m1-m2) - s1*l21; 
  Ith(rhsBQS,4) = y2*y3*(m2-m1) + (y3*s2+y2*s3)*l21;

  return(0);
}

static int resBS2(realtype tt, 
                  N_Vector yy, N_Vector yp, 
                  N_Vector *yyS, N_Vector *ypS,
                  N_Vector yyB, N_Vector ypB,
                  N_Vector rrBS, void *user_dataB)

{
  UserData data;
  realtype y1, y2, y3;
  realtype p1, p2, p3;
  realtype l1, l2, l3, m1, m2, m3;
  realtype lp1, lp2, mp1, mp2;
  realtype s1, s2, s3;
  realtype l21;
  
  data = (UserData) user_dataB;

  /* The parameters. */
  p1 = data->p[0]; p2 = data->p[1]; p3 = data->p[2];

  /* The y vector. */
  y1 = Ith(yy,1); y2 = Ith(yy,2); y3 = Ith(yy,3);

  /* The lambda vector. */
  l1 = Ith(yyB,1); l2 = Ith(yyB,2); l3 = Ith(yyB,3);
  /* The mu vector. */
  m1 = Ith(yyB,4); m2 = Ith(yyB,5); m3 = Ith(yyB,6);

  /* The lambda dot vector. */
  lp1 = Ith(ypB,1); lp2 = Ith(ypB,2);
  /* The mu dot vector. */
  mp1 = Ith(ypB,4); mp2 = Ith(ypB,5);

  /* The sensitivity with respect to p2 */
  s1 = Ith(yyS[1],1); s2 = Ith(yyS[1],2); s3 = Ith(yyS[1],3);

  /* Temporary variables */
  l21 = l2-l1;

  Ith(rrBS,1) = lp1 + p1*l21 - l3 + y1;
  Ith(rrBS,2) = lp2 - p2*y3*l21 - TWO*p3*y2*l2 - l3 + y2;
  Ith(rrBS,3) = -p2*y2*l21 - l3 + y3;

  Ith(rrBS,4) = mp1 + p1*(-m1+m2) - m3 + s1;
  Ith(rrBS,5) = mp2 + p2*y3*m1 - (p2*y3+TWO*p3*y2)*m2 - m3 + (y3+p2*s3)*l1 - (y3+TWO*p3*s2+p2*s3)*l2 + s2;
  Ith(rrBS,6) = p2*y2*(m1-m2) - m3 - (y2+p2*s2)*l21 + s3;

  return(0);
}

static int rhsQBS2(realtype tt, 
                 N_Vector yy, N_Vector yp,
                 N_Vector *yyS, N_Vector *ypS,
                 N_Vector yyB, N_Vector ypB,
                 N_Vector rhsBQS, void *user_dataB)
{
  UserData data;
  realtype y1, y2, y3;
  realtype p1, p2, p3;
  realtype l1, l2, l3, m1, m2, m3;
  realtype s1, s2, s3;
  realtype l21;
  
  data = (UserData) user_dataB;

  /* The p vector */
  p1 = data->p[0]; p2 = data->p[1]; p3 = data->p[2];

  /* The y vector */
  y1 = Ith(yy,1); y2 = Ith(yy,2); y3 = Ith(yy,3);
  
  /* The lambda vector. */
  l1 = Ith(yyB,1); l2 = Ith(yyB,2); l3 = Ith(yyB,3);
  /* The mu vector. */
  m1 = Ith(yyB,4); m2 = Ith(yyB,5); m3 = Ith(yyB,6);

  /* The sensitivity with respect to p2 */
  s1 = Ith(yyS[1],1); s2 = Ith(yyS[1],2); s3 = Ith(yyS[1],3);
  
  /* Temporary variables */
  l21 = l2-l1;

  Ith(rhsBQS,1) = -y1*l21;
  Ith(rhsBQS,2) =  y2*y3*l21;

  Ith(rhsBQS,3) = y1*(m1-m2) - s1*l21; 
  Ith(rhsBQS,4) = y2*y3*(m2-m1) + (y3*s2+y2*s3)*l21;

  return(0);
}
