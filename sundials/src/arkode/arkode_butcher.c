/*---------------------------------------------------------------
 $Revision: 1.0 $
 $Date:  $
----------------------------------------------------------------- 
 Programmer(s): Daniel R. Reynolds @ SMU
-----------------------------------------------------------------
 This is the implementation file for the known Butcher tables 
 for the ARKODE solver.
---------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "arkode_impl.h"
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>


/*---------------------------------------------------------------
 Returns butcher table for Runge Kutta methods.  
 Input:
     imeth -- integer key for the desired method (see below)
 Outputs:
     s -- int number of stages for the requested method [s <= ARK_S_MAX = 8]
     q -- integer, theoretical order of accuracy for the method
     p -- integer, theoretical order of accuracy for the embedding
     A[s][s] -- realtype Butcher table coefficients
     b[s] -- realtype root node coefficients
     c[s] -- realtype canopy node coefficients
     b2[s] -- realtype embedding coefficients

 Allowed 'method' names and properties (those in an ARK pair are marked 
 with a *):

   imeth    name             type   s   q   p   A   L 
  ----------------------------------------------------
    0     Heun-Euler          ERK   2   2   1   -   - 
    1     ERK-3-2             ERK   3   3   2   -   -   **not very accurate 
    2     Bogacki-Shampine    ERK   4   3   2   -   - 
    3*    ARK3(2)4L[2]SA      ERK   4   3   2   -   - 
    4     Merson              ERK   5   4   3   -   -   **not at all accurate
    5     Zonneveld           ERK   5   4   3   -   -
    6*    ARK4(3)6L[2]SA      ERK   6   4   3   -   - 
    7*    Sayfy-Aburub-4-3    ERK   6   4   3   -   - 
    8     Cash-Karp           ERK   6   5   4   -   -
    9     Fehlberg            ERK   6   5   4   -   - 
    10    Dormand-Prince      ERK   7   5   4   -   - 
    11*   ARK5(4)8L[2]SA      ERK   8   5   4   -   - 
    12    Verner-6-5          ERK   8   6   5   -   -
    13    Billington        SDIRK   3   3   2   -   - 
    14    TRBDF2           ESDIRK   3   3   2   -   - 
    15    Kvaerno(4,2,3)   ESDIRK   4   3   2   X   X 
    16*   ARK3(2)4L[2]SA   ESDIRK   4   3   2   X   X 
    17    TRX2             ESDIRK   3   4   2   -   - 
    18    Cash(5,2,4)       SDIRK   5   4   2   X   X 
    19    Cash(5,3,4)       SDIRK   5   4   3   X   X 
    20    SDIRK-5-4         SDIRK   5   4   3   X   X 
    21    Kvaerno(5,3,4)   ESDIRK   5   4   3   X   - 
    22*   ARK4(3)6L[2]SA   ESDIRK   6   4   3   X   X 
    23*   Sayfy-Aburub-4-3  EDIRK   6   4   3   X   -   **not very accurate 
    24    Ismail(7,4,5)    ESDIRK   7   5   4   -   -   **not at all accurate
    25    Kvaerno(7,4,5)   ESDIRK   7   5   4   X   X
    26*   ARK5(4)8L[2]SA   ESDIRK   8   5   4   X   - 
  ----------------------------------------------------

---------------------------------------------------------------*/
int ARKodeLoadButcherTable(int imethod, int *s, int *q, int *p, 
			   realtype *A, realtype *b, 
			   realtype *c, realtype *b2) 
{

  int i, j;

  /* initialize output tables to zero */
  for (i=0; i<ARK_S_MAX; i++) {
    b[i]  = ZERO;
    c[i]  = ZERO;
    b2[i] = ZERO;
    for (j=0; j<ARK_S_MAX; j++)  ARK_A(A,i,j) = ZERO;
  }

  /* fill in coefficients based on method name */
  switch(imethod) {

  case(0):    /* Heun-Euler-ERK */
    *s = 2;
    *q = 2;
    *p = 1;
      
    ARK_A(A,1,0) = RCONST(1.0);

    b[0] = RCONST(0.5);
    b[1] = RCONST(0.5);

    b2[0] = RCONST(1.0);

    c[1] = RCONST(1.0);
    break;

  case(1):  /* ERK-3-2 */
    *s = 3;
    *q = 3;
    *p = 2;
    ARK_A(A,1,0) = RCONST(0.5);
    ARK_A(A,2,0) = -RCONST(1.0);
    ARK_A(A,2,1) = RCONST(2.0);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[1] = RCONST(2.0)/RCONST(3.0);
    b[2] = RCONST(1.0)/RCONST(6.0);

    b2[1] = RCONST(1.0);

    c[1] = RCONST(0.5);
    c[2] = RCONST(1.0);

  case(2):    /* Bogacki-Shampine-ERK */
    *s = 4;
    *q = 3;
    *p = 2;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(2.0);
    ARK_A(A,2,1) = RCONST(3.0)/RCONST(4.0);
    ARK_A(A,3,0) = RCONST(2.0)/RCONST(9.0);
    ARK_A(A,3,1) = RCONST(1.0)/RCONST(3.0);
    ARK_A(A,3,2) = RCONST(4.0)/RCONST(9.0);

    b[0] = RCONST(2.0)/RCONST(9.0);
    b[1] = RCONST(1.0)/RCONST(3.0);
    b[2] = RCONST(4.0)/RCONST(9.0);

    b2[0] = RCONST(7.0)/RCONST(24.0);
    b2[1] = RCONST(1.0)/RCONST(4.0);
    b2[2] = RCONST(1.0)/RCONST(3.0);
    b2[3] = RCONST(1.0)/RCONST(8.0);

    c[1] = RCONST(1.0)/RCONST(2.0);
    c[2] = RCONST(3.0)/RCONST(4.0);
    c[3] = RCONST(1.0);
    break;

  case(3):    /* ARK3(2)4L[2]SA-ERK */
    *s = 4;
    *q = 3;
    *p = 2;
    ARK_A(A,1,0) = RCONST(1767732205903.0)/RCONST(2027836641118.0);
    ARK_A(A,2,0) = RCONST(5535828885825.0)/RCONST(10492691773637.0);
    ARK_A(A,2,1) = RCONST(788022342437.0)/RCONST(10882634858940.0);
    ARK_A(A,3,0) = RCONST(6485989280629.0)/RCONST(16251701735622.0);
    ARK_A(A,3,1) = RCONST(-4246266847089.0)/RCONST(9704473918619.0);
    ARK_A(A,3,2) = RCONST(10755448449292.0)/RCONST(10357097424841.0);

    b[0] = RCONST(1471266399579.0)/RCONST(7840856788654.0);
    b[1] = RCONST(-4482444167858.0)/RCONST(7529755066697.0);
    b[2] = RCONST(11266239266428.0)/RCONST(11593286722821.0);
    b[3] = RCONST(1767732205903.0)/RCONST(4055673282236.0);

    b2[0] = RCONST(2756255671327.0)/RCONST(12835298489170.0);
    b2[1] = RCONST(-10771552573575.0)/RCONST(22201958757719.0);
    b2[2] = RCONST(9247589265047.0)/RCONST(10645013368117.0);
    b2[3] = RCONST(2193209047091.0)/RCONST(5459859503100.0);

    c[1] = RCONST(1767732205903.0)/RCONST(2027836641118.0);
    c[2] = RCONST(3.0)/RCONST(5.0);
    c[3] = RCONST(1.0);
    break;

  case(4):    /* Merson */
    *s = 5;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(3.0);
    ARK_A(A,2,0) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,2,1) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,3,0) = RCONST(1.0)/RCONST(8.0);
    ARK_A(A,3,2) = RCONST(3.0)/RCONST(8.0);
    ARK_A(A,4,0) = RCONST(5.0);
    ARK_A(A,4,2) = RCONST(-3.0)/RCONST(2.0);
    ARK_A(A,4,3) = RCONST(2.0);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[3] = RCONST(2.0)/RCONST(3.0);
    b[4] = RCONST(1.0)/RCONST(6.0);

    b2[0] = RCONST(0.1);
    b2[2] = RCONST(0.3);
    b2[3] = RCONST(0.4);
    b2[4] = RCONST(0.2);

    c[1] = RCONST(1.0)/RCONST(3.0);
    c[2] = RCONST(1.0)/RCONST(3.0);
    c[3] = RCONST(0.5);
    c[4] = RCONST(1.0);
    break;

  case(5):    /* Zonneveld */
    *s = 5;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(0.5);
    ARK_A(A,2,1) = RCONST(0.5);
    ARK_A(A,3,2) = RCONST(1.0);
    ARK_A(A,4,0) = RCONST(5.0)/RCONST(32.0);
    ARK_A(A,4,1) = RCONST(7.0)/RCONST(32.0);
    ARK_A(A,4,2) = RCONST(13.0)/RCONST(32.0);
    ARK_A(A,4,3) = RCONST(-1.0)/RCONST(32.0);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[1] = RCONST(1.0)/RCONST(3.0);
    b[2] = RCONST(1.0)/RCONST(3.0);
    b[3] = RCONST(1.0)/RCONST(6.0);

    b2[0] = RCONST(-1.0)/RCONST(2.0);
    b2[1] = RCONST(7.0)/RCONST(3.0);
    b2[2] = RCONST(7.0)/RCONST(3.0);
    b2[3] = RCONST(13.0)/RCONST(6.0);
    b2[4] = RCONST(-16.0)/RCONST(3.0);

    c[1] = RCONST(0.5);
    c[2] = RCONST(0.5);
    c[3] = RCONST(1.0);
    c[4] = RCONST(0.75);
    break;

  case(6):    /* ARK4(3)6L[2]SA-ERK */
    *s = 6;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(0.5);
    ARK_A(A,2,0) = RCONST(13861.0)/RCONST(62500.0);
    ARK_A(A,2,1) = RCONST(6889.0)/RCONST(62500.0);
    ARK_A(A,3,0) = RCONST(-116923316275.0)/RCONST(2393684061468.0);
    ARK_A(A,3,1) = RCONST(-2731218467317.0)/RCONST(15368042101831.0);
    ARK_A(A,3,2) = RCONST(9408046702089.0)/RCONST(11113171139209.0);
    ARK_A(A,4,0) = RCONST(-451086348788.0)/RCONST(2902428689909.0);
    ARK_A(A,4,1) = RCONST(-2682348792572.0)/RCONST(7519795681897.0);
    ARK_A(A,4,2) = RCONST(12662868775082.0)/RCONST(11960479115383.0);
    ARK_A(A,4,3) = RCONST(3355817975965.0)/RCONST(11060851509271.0);
    ARK_A(A,5,0) = RCONST(647845179188.0)/RCONST(3216320057751.0);
    ARK_A(A,5,1) = RCONST(73281519250.0)/RCONST(8382639484533.0);
    ARK_A(A,5,2) = RCONST(552539513391.0)/RCONST(3454668386233.0);
    ARK_A(A,5,3) = RCONST(3354512671639.0)/RCONST(8306763924573.0);
    ARK_A(A,5,4) = RCONST(4040.0)/RCONST(17871.0);

    b[0] = RCONST(82889.0)/RCONST(524892.0);
    b[2] = RCONST(15625.0)/RCONST(83664.0);
    b[3] = RCONST(69875.0)/RCONST(102672.0);
    b[4] = RCONST(-2260.0)/RCONST(8211.0);
    b[5] = RCONST(1.0)/RCONST(4.0);

    b2[0] = RCONST(4586570599.0)/RCONST(29645900160.0);
    b2[2] = RCONST(178811875.0)/RCONST(945068544.0);
    b2[3] = RCONST(814220225.0)/RCONST(1159782912.0);
    b2[4] = RCONST(-3700637.0)/RCONST(11593932.0);
    b2[5] = RCONST(61727.0)/RCONST(225920.0);

    c[1] = RCONST(1.0)/RCONST(2.0);
    c[2] = RCONST(83.0)/RCONST(250.0);
    c[3] = RCONST(31.0)/RCONST(50.0);
    c[4] = RCONST(17.0)/RCONST(20.0);
    c[5] = RCONST(1.0);
    break;

  case(7):    /* Sayfy-Aburub-4-3-ERK */
    *s = 6;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(2.0); 
    ARK_A(A,2,0) = RCONST(-1.0); 
    ARK_A(A,2,1) = RCONST(2.0); 
    ARK_A(A,3,0) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,3,1) = RCONST(2.0)/RCONST(3.0);
    ARK_A(A,3,2) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,4,0) = RCONST(0.137);
    ARK_A(A,4,1) = RCONST(0.226);
    ARK_A(A,4,2) = RCONST(0.137);
    ARK_A(A,5,0) = RCONST(0.452);
    ARK_A(A,5,1) = RCONST(-0.904);
    ARK_A(A,5,2) = RCONST(-0.548);
    ARK_A(A,5,4) = RCONST(2.0);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[1] = RCONST(1.0)/RCONST(3.0);
    b[2] = RCONST(1.0)/RCONST(12.0);
    b[3] = RCONST(0.0);
    b[4] = RCONST(1.0)/RCONST(3.0);
    b[5] = RCONST(1.0)/RCONST(12.0);

    b2[0] = RCONST(1.0)/RCONST(6.0);
    b2[1] = RCONST(2.0)/RCONST(3.0);
    b2[2] = RCONST(1.0)/RCONST(6.0);

    c[1] = RCONST(1.0)/RCONST(2.0);
    c[2] = RCONST(1.0);
    c[3] = RCONST(1.0);
    c[4] = RCONST(1.0)/RCONST(2.0);
    c[5] = RCONST(1.0);
    break;

  case(8):    /* Cash-Karp-ERK */
    *s = 6;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(5.0);
    ARK_A(A,2,0) = RCONST(3.0)/RCONST(40.0);
    ARK_A(A,2,1) = RCONST(9.0)/RCONST(40.0);
    ARK_A(A,3,0) = RCONST(3.0)/RCONST(10.0);
    ARK_A(A,3,1) = RCONST(-9.0)/RCONST(10.0);
    ARK_A(A,3,2) = RCONST(6.0)/RCONST(5.0);
    ARK_A(A,4,0) = RCONST(-11.0)/RCONST(54.0);
    ARK_A(A,4,1) = RCONST(5.0)/RCONST(2.0);
    ARK_A(A,4,2) = RCONST(-70.0)/RCONST(27.0);
    ARK_A(A,4,3) = RCONST(35.0)/RCONST(27.0);
    ARK_A(A,5,0) = RCONST(1631.0)/RCONST(55296.0);
    ARK_A(A,5,1) = RCONST(175.0)/RCONST(512.0);
    ARK_A(A,5,2) = RCONST(575.0)/RCONST(13824.0);
    ARK_A(A,5,3) = RCONST(44275.0)/RCONST(110592.0);
    ARK_A(A,5,4) = RCONST(253.0)/RCONST(4096.0);

    b[0] = RCONST(37.0)/RCONST(378.0);
    b[2] = RCONST(250.0)/RCONST(621.0);
    b[3] = RCONST(125.0)/RCONST(594.0);
    b[5] = RCONST(512.0)/RCONST(1771.0);

    b2[0] = RCONST(2825.0)/RCONST(27648.0);
    b2[2] = RCONST(18575.0)/RCONST(48384.0);
    b2[3] = RCONST(13525.0)/RCONST(55296.0);
    b2[4] = RCONST(277.0)/RCONST(14336.0);
    b2[5] = RCONST(1.0)/RCONST(4.0);

    c[1] = RCONST(1.0)/RCONST(5.0);
    c[2] = RCONST(3.0)/RCONST(10.0);
    c[3] = RCONST(3.0)/RCONST(5.0);
    c[4] = RCONST(1.0);
    c[5] = RCONST(7.0)/RCONST(8.0);
    break;

  case(9):    /* Fehlberg-ERK */
    *s = 6;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,2,0) = RCONST(3.0)/RCONST(32.0);
    ARK_A(A,2,1) = RCONST(9.0)/RCONST(32.0);
    ARK_A(A,3,0) = RCONST(1932.0)/RCONST(2197.0);
    ARK_A(A,3,1) = RCONST(-7200.0)/RCONST(2197.0);
    ARK_A(A,3,2) = RCONST(7296.0)/RCONST(2197.0);
    ARK_A(A,4,0) = RCONST(439.0)/RCONST(216.0);
    ARK_A(A,4,1) = RCONST(-8.0);
    ARK_A(A,4,2) = RCONST(3680.0)/RCONST(513.0);
    ARK_A(A,4,3) = RCONST(-845.0)/RCONST(4104.0);
    ARK_A(A,5,0) = RCONST(-8.0)/RCONST(27.0);
    ARK_A(A,5,1) = RCONST(2.0);
    ARK_A(A,5,2) = RCONST(-3544.0)/RCONST(2565.0);
    ARK_A(A,5,3) = RCONST(1859.0)/RCONST(4104.0);
    ARK_A(A,5,4) = RCONST(-11.0)/RCONST(40.0);

    b[0] = RCONST(25.0)/RCONST(216.0);
    b[2] = RCONST(1408.0)/RCONST(2565.0);
    b[3] = RCONST(2197.0)/RCONST(4104.0);
    b[4] = RCONST(-1.0)/RCONST(5.0);
      
    b2[0] = RCONST(16.0)/RCONST(135.0);
    b2[2] = RCONST(6656.0)/RCONST(12825.0);
    b2[3] = RCONST(28561.0)/RCONST(56430.0);
    b2[4] = RCONST(-9.0)/RCONST(50.0);
    b2[5] = RCONST(2.0)/RCONST(55.0);

    c[1] = RCONST(1.0)/RCONST(4.0);
    c[2] = RCONST(3.0)/RCONST(8.0);
    c[3] = RCONST(12.0)/RCONST(13.0);
    c[4] = RCONST(1.0);
    c[5] = RCONST(1.0)/RCONST(2.0);
    break;

  case(10):    /* Dormand-Prince-ERK */
    *s = 7;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(5.0);
    ARK_A(A,2,0) = RCONST(3.0)/RCONST(40.0);
    ARK_A(A,2,1) = RCONST(9.0)/RCONST(40.0);
    ARK_A(A,3,0) = RCONST(44.0)/RCONST(45.0);
    ARK_A(A,3,1) = RCONST(-56.0)/RCONST(15.0);
    ARK_A(A,3,2) = RCONST(32.0)/RCONST(9.0);
    ARK_A(A,4,0) = RCONST(19372.0)/RCONST(6561.0);
    ARK_A(A,4,1) = RCONST(-25360.0)/RCONST(2187.0);
    ARK_A(A,4,2) = RCONST(64448.0)/RCONST(6561.0);
    ARK_A(A,4,3) = RCONST(-212.0)/RCONST(729.0);
    ARK_A(A,5,0) = RCONST(9017.0)/RCONST(3168.0);
    ARK_A(A,5,1) = RCONST(-355.0)/RCONST(33.0);
    ARK_A(A,5,2) = RCONST(46732.0)/RCONST(5247.0);
    ARK_A(A,5,3) = RCONST(49.0)/RCONST(176.0);
    ARK_A(A,5,4) = RCONST(-5103.0)/RCONST(18656.0);
    ARK_A(A,6,0) = RCONST(35.0)/RCONST(384.0);
    ARK_A(A,6,2) = RCONST(500.0)/RCONST(1113.0);
    ARK_A(A,6,3) = RCONST(125.0)/RCONST(192.0);
    ARK_A(A,6,4) = RCONST(-2187.0)/RCONST(6784.0);
    ARK_A(A,6,5) = RCONST(11.0)/RCONST(84.0);

    b[0] = RCONST(35.0)/RCONST(384.0);
    b[2] = RCONST(500.0)/RCONST(1113.0);
    b[3] = RCONST(125.0)/RCONST(192.0);
    b[4] = RCONST(-2187.0)/RCONST(6784.0);
    b[5] = RCONST(11.0)/RCONST(84.0);

    b2[0] = RCONST(5179.0)/RCONST(57600.0);
    b2[2] = RCONST(7571.0)/RCONST(16695.0);
    b2[3] = RCONST(393.0)/RCONST(640.0);
    b2[4] = RCONST(-92097.0)/RCONST(339200.0);
    b2[5] = RCONST(187.0)/RCONST(2100.0);
    b2[6] = RCONST(1.0)/RCONST(40.0);

    c[1] = RCONST(1.0)/RCONST(5.0);
    c[2] = RCONST(3.0)/RCONST(10.0);
    c[3] = RCONST(4.0)/RCONST(5.0);
    c[4] = RCONST(8.0)/RCONST(9.0);
    c[5] = RCONST(1.0);
    c[6] = RCONST(1.0);
    break;

  case(11):    /* ARK5(4)8L[2]SA-ERK */
    *s = 8;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(41.0)/RCONST(100.0);
    ARK_A(A,2,0) = RCONST(367902744464.0)/RCONST(2072280473677.0);
    ARK_A(A,2,1) = RCONST(677623207551.0)/RCONST(8224143866563.0);
    ARK_A(A,3,0) = RCONST(1268023523408.0)/RCONST(10340822734521.0);
    ARK_A(A,3,2) = RCONST(1029933939417.0)/RCONST(13636558850479.0);
    ARK_A(A,4,0) = RCONST(14463281900351.0)/RCONST(6315353703477.0);
    ARK_A(A,4,2) = RCONST(66114435211212.0)/RCONST(5879490589093.0);
    ARK_A(A,4,3) = RCONST(-54053170152839.0)/RCONST(4284798021562.0);
    ARK_A(A,5,0) = RCONST(14090043504691.0)/RCONST(34967701212078.0);
    ARK_A(A,5,2) = RCONST(15191511035443.0)/RCONST(11219624916014.0);
    ARK_A(A,5,3) = RCONST(-18461159152457.0)/RCONST(12425892160975.0);
    ARK_A(A,5,4) = RCONST(-281667163811.0)/RCONST(9011619295870.0);
    ARK_A(A,6,0) = RCONST(19230459214898.0)/RCONST(13134317526959.0);
    ARK_A(A,6,2) = RCONST(21275331358303.0)/RCONST(2942455364971.0);
    ARK_A(A,6,3) = RCONST(-38145345988419.0)/RCONST(4862620318723.0);
    ARK_A(A,6,4) = RCONST(-1.0)/RCONST(8.0);
    ARK_A(A,6,5) = RCONST(-1.0)/RCONST(8.0);
    ARK_A(A,7,0) = RCONST(-19977161125411.0)/RCONST(11928030595625.0);
    ARK_A(A,7,2) = RCONST(-40795976796054.0)/RCONST(6384907823539.0);
    ARK_A(A,7,3) = RCONST(177454434618887.0)/RCONST(12078138498510.0);
    ARK_A(A,7,4) = RCONST(782672205425.0)/RCONST(8267701900261.0);
    ARK_A(A,7,5) = RCONST(-69563011059811.0)/RCONST(9646580694205.0);
    ARK_A(A,7,6) = RCONST(7356628210526.0)/RCONST(4942186776405.0);

    b[0] = RCONST(-872700587467.0)/RCONST(9133579230613.0);
    b[3] = RCONST(22348218063261.0)/RCONST(9555858737531.0);
    b[4] = RCONST(-1143369518992.0)/RCONST(8141816002931.0);
    b[5] = RCONST(-39379526789629.0)/RCONST(19018526304540.0);
    b[6] = RCONST(32727382324388.0)/RCONST(42900044865799.0);
    b[7] = RCONST(41.0)/RCONST(200.0);

    b2[0] = RCONST(-975461918565.0)/RCONST(9796059967033.0);
    b2[3] = RCONST(78070527104295.0)/RCONST(32432590147079.0);
    b2[4] = RCONST(-548382580838.0)/RCONST(3424219808633.0);
    b2[5] = RCONST(-33438840321285.0)/RCONST(15594753105479.0);
    b2[6] = RCONST(3629800801594.0)/RCONST(4656183773603.0);
    b2[7] = RCONST(4035322873751.0)/RCONST(18575991585200.0);

    c[1] = RCONST(41.0)/RCONST(100.0);
    c[2] = RCONST(2935347310677.0)/RCONST(11292855782101.0);
    c[3] = RCONST(1426016391358.0)/RCONST(7196633302097.0);
    c[4] = RCONST(92.0)/RCONST(100.0);
    c[5] = RCONST(24.0)/RCONST(100.0);
    c[6] = RCONST(3.0)/RCONST(5.0);
    c[7] = RCONST(1.0);
    break;

  case(12):    /* Verner-6-5 */
    *s = 8;
    *q = 6;
    *p = 5;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,2,0) = RCONST(4.0)/RCONST(75.0);
    ARK_A(A,2,1) = RCONST(16.0)/RCONST(75.0);
    ARK_A(A,3,0) = RCONST(5.0)/RCONST(6.0);
    ARK_A(A,3,1) = RCONST(-8.0)/RCONST(3.0);
    ARK_A(A,3,2) = RCONST(5.0)/RCONST(2.0);
    ARK_A(A,4,0) = RCONST(-165.0)/RCONST(64.0);
    ARK_A(A,4,1) = RCONST(55.0)/RCONST(6.0);
    ARK_A(A,4,2) = RCONST(-425.0)/RCONST(64.0);
    ARK_A(A,4,3) = RCONST(85.0)/RCONST(96.0);
    ARK_A(A,5,0) = RCONST(12.0)/RCONST(5.0);
    ARK_A(A,5,1) = RCONST(-8.0);
    ARK_A(A,5,2) = RCONST(4015.0)/RCONST(612.0);
    ARK_A(A,5,3) = RCONST(-11.0)/RCONST(36.0);
    ARK_A(A,5,4) = RCONST(88.0)/RCONST(255.0);
    ARK_A(A,6,0) = RCONST(-8263.0)/RCONST(15000.0);
    ARK_A(A,6,1) = RCONST(124.0)/RCONST(75.0);
    ARK_A(A,6,2) = RCONST(-643.0)/RCONST(680.0);
    ARK_A(A,6,3) = RCONST(-81.0)/RCONST(250.0);
    ARK_A(A,6,4) = RCONST(2484.0)/RCONST(10625.0);
    ARK_A(A,7,0) = RCONST(3501.0)/RCONST(1720.0);
    ARK_A(A,7,1) = RCONST(-300.0)/RCONST(43.0);
    ARK_A(A,7,2) = RCONST(297275.0)/RCONST(52632.0);
    ARK_A(A,7,3) = RCONST(-319.0)/RCONST(2322.0);
    ARK_A(A,7,4) = RCONST(24068.0)/RCONST(84065.0);
    ARK_A(A,7,6) = RCONST(3850.0)/RCONST(26703.0);

    b[0] = RCONST(3.0)/RCONST(40.0);
    b[2] = RCONST(875.0)/RCONST(2244.0);
    b[3] = RCONST(23.0)/RCONST(72.0);
    b[4] = RCONST(264.0)/RCONST(1955.0);
    b[6] = RCONST(125.0)/RCONST(11592.0);
    b[7] = RCONST(43.0)/RCONST(616.0);

    b2[0] = RCONST(13.0)/RCONST(160.0);
    b2[2] = RCONST(2375.0)/RCONST(5984.0);
    b2[3] = RCONST(5.0)/RCONST(16.0);
    b2[4] = RCONST(12.0)/RCONST(85.0);
    b2[5] = RCONST(3.0)/RCONST(44.0);

    c[0] = RCONST(0.0);
    c[1] = RCONST(1.0)/RCONST(6.0);
    c[2] = RCONST(4.0)/RCONST(15.0);
    c[3] = RCONST(2.0)/RCONST(3.0);
    c[4] = RCONST(5.0)/RCONST(6.0);
    c[5] = RCONST(1.0);
    c[6] = RCONST(1.0)/RCONST(15.0);
    c[7] = RCONST(1.0);
    break;

  case(13):    /* Billington-SDIRK */
    *s = 3;
    *q = 3;
    *p = 2;

    ARK_A(A,0,0) = RCONST(0.292893218813);
    ARK_A(A,1,0) = RCONST(0.798989873223);
    ARK_A(A,1,1) = RCONST(0.292893218813);
    ARK_A(A,2,0) = RCONST(0.740789228841);
    ARK_A(A,2,1) = RCONST(0.259210771159);
    ARK_A(A,2,2) = RCONST(0.292893218813);

    b[0] = RCONST(0.691665115992);
    b[1] = RCONST(0.503597029883);
    b[2] = RCONST(-0.195262145876);

    b2[0] = RCONST(0.740789228840);
    b2[1] = RCONST(0.259210771159);

    c[0] = RCONST(0.292893218813);
    c[1] = RCONST(1.091883092037);
    c[2] = RCONST(1.292893218813);
    break;

  case(14):    /* TRBDF2-ESDIRK */
    *s = 3;
    *q = 3;
    *p = 2;

    realtype one = RCONST(1.0);
    realtype two = RCONST(2.0);
    realtype three = RCONST(3.0);
    realtype four = RCONST(4.0);
    realtype six = RCONST(6.0);
    realtype sqrt2 = RSqrt(two);
    ARK_A(A,1,0) = (two-sqrt2)/two;
    ARK_A(A,1,1) = (two-sqrt2)/two;
    ARK_A(A,2,0) = sqrt2/four;
    ARK_A(A,2,1) = sqrt2/four;
    ARK_A(A,2,2) = (two-sqrt2)/two;

    b[0] = (one-sqrt2/four)/three;
    b[1] = (three*sqrt2/four+one)/three;
    b[2] = (two-sqrt2)/six;

    b2[0] = sqrt2/four;
    b2[1] = sqrt2/four;
    b2[2] = (two-sqrt2)/two;

    c[1] = two-sqrt2;
    c[2] = one;
    break;

  case(15):    /* Kvaerno(4,2,3)-ESDIRK */
    *s = 4;
    *q = 3;
    *p = 2;
    ARK_A(A,1,0) = RCONST(0.4358665215);
    ARK_A(A,1,1) = RCONST(0.4358665215);
    ARK_A(A,2,0) = RCONST(0.490563388419108);
    ARK_A(A,2,1) = RCONST(0.073570090080892);
    ARK_A(A,2,2) = RCONST(0.4358665215);
    ARK_A(A,3,0) = RCONST(0.308809969973036);
    ARK_A(A,3,1) = RCONST(1.490563388254106);
    ARK_A(A,3,2) = RCONST(-1.235239879727145);
    ARK_A(A,3,3) = RCONST(0.4358665215);

    b[0] = RCONST(0.308809969973036);
    b[1] = RCONST(1.490563388254106);
    b[2] = RCONST(-1.235239879727145);
    b[3] = RCONST(0.4358665215);

    b2[0] = RCONST(0.490563388419108);
    b2[1] = RCONST(0.073570090080892);
    b2[2] = RCONST(0.4358665215);

    c[1] = RCONST(0.871733043);
    c[2] = RCONST(1.0);
    c[3] = RCONST(1.0);
    break;

  case(16):    /* ARK3(2)4L[2]SA-ESDIRK */
    *s = 4;
    *q = 3;
    *p = 2;
    ARK_A(A,1,0) = RCONST(1767732205903.0)/RCONST(4055673282236.0);
    ARK_A(A,1,1) = RCONST(1767732205903.0)/RCONST(4055673282236.0);
    ARK_A(A,2,0) = RCONST(2746238789719.0)/RCONST(10658868560708.0);
    ARK_A(A,2,1) = RCONST(-640167445237.0)/RCONST(6845629431997.0);
    ARK_A(A,2,2) = RCONST(1767732205903.0)/RCONST(4055673282236.0);
    ARK_A(A,3,0) = RCONST(1471266399579.0)/RCONST(7840856788654.0);
    ARK_A(A,3,1) = RCONST(-4482444167858.0)/RCONST(7529755066697.0);
    ARK_A(A,3,2) = RCONST(11266239266428.0)/RCONST(11593286722821.0);
    ARK_A(A,3,3) = RCONST(1767732205903.0)/RCONST(4055673282236.0);

    b[0] = RCONST(1471266399579.0)/RCONST(7840856788654.0);
    b[1] = RCONST(-4482444167858.0)/RCONST(7529755066697.0);
    b[2] = RCONST(11266239266428.0)/RCONST(11593286722821.0);
    b[3] = RCONST(1767732205903.0)/RCONST(4055673282236.0);

    b2[0] = RCONST(2756255671327.0)/RCONST(12835298489170.0);
    b2[1] = RCONST(-10771552573575.0)/RCONST(22201958757719.0);
    b2[2] = RCONST(9247589265047.0)/RCONST(10645013368117.0);
    b2[3] = RCONST(2193209047091.0)/RCONST(5459859503100.0);

    c[1] = RCONST(1767732205903.0)/RCONST(2027836641118.0);
    c[2] = RCONST(3.0)/RCONST(5.0);
    c[3] = RCONST(1.0);
    break;

  case(17):    /* TRX2-ESDIRK */
    *s = 3;
    *q = 4;
    *p = 2;

    ARK_A(A,1,0) = RCONST(0.25);
    ARK_A(A,1,1) = RCONST(0.25);
    ARK_A(A,2,0) = RCONST(0.25);
    ARK_A(A,2,1) = RCONST(0.5);
    ARK_A(A,2,2) = RCONST(0.25);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[1] = RCONST(2.0)/RCONST(3.0);
    b[2] = RCONST(1.0)/RCONST(6.0);

    b2[0] = RCONST(0.25);
    b2[1] = RCONST(0.5);
    b2[2] = RCONST(0.25);

    c[1] = RCONST(0.5);
    c[2] = RCONST(1.0);
    break;

  case(18):    /* Cash(5,2,4)-SDIRK */
    *s = 5;
    *q = 4;
    *p = 2;
    ARK_A(A,0,0) = RCONST(0.435866521508);
    ARK_A(A,1,0) = RCONST(-1.13586652150);
    ARK_A(A,1,1) = RCONST(0.435866521508);
    ARK_A(A,2,0) = RCONST(1.08543330679);
    ARK_A(A,2,1) = RCONST(-0.721299828287);
    ARK_A(A,2,2) = RCONST(0.435866521508);
    ARK_A(A,3,0) = RCONST(0.416349501547);
    ARK_A(A,3,1) = RCONST(0.190984004184);
    ARK_A(A,3,2) = RCONST(-0.118643265417);
    ARK_A(A,3,3) = RCONST(0.435866521508);
    ARK_A(A,4,0) = RCONST(0.896869652944);
    ARK_A(A,4,1) = RCONST(0.0182725272734);
    ARK_A(A,4,2) = RCONST(-0.0845900310706);
    ARK_A(A,4,3) = RCONST(-0.266418670647);
    ARK_A(A,4,4) = RCONST(0.435866521508);

    b[0] = RCONST(0.896869652944);
    b[1] = RCONST(0.0182725272734);
    b[2] = RCONST(-0.0845900310706);
    b[3] = RCONST(-0.266418670647);
    b[4] = RCONST(0.435866521508);

    b2[0] = RCONST((-0.7-0.5))/RCONST((-0.7-0.435866521508));
    b2[1] = RCONST((0.5-0.435866521508))/RCONST((-0.7-0.435866521508));

    c[0] = RCONST(0.435866521508);
    c[1] = RCONST(-0.7);
    c[2] = RCONST(0.8);
    c[3] = RCONST(0.924556761814);
    c[4] = RCONST(1.0);
    break;

  case(19):    /* Cash(5,3,4)-SDIRK */
    *s = 5;
    *q = 4;
    *p = 3;
    ARK_A(A,0,0) = RCONST(0.435866521508);
    ARK_A(A,1,0) = RCONST(-1.13586652150);
    ARK_A(A,1,1) = RCONST(0.435866521508);
    ARK_A(A,2,0) = RCONST(1.08543330679);
    ARK_A(A,2,1) = RCONST(-0.721299828287);
    ARK_A(A,2,2) = RCONST(0.435866521508);
    ARK_A(A,3,0) = RCONST(0.416349501547);
    ARK_A(A,3,1) = RCONST(0.190984004184);
    ARK_A(A,3,2) = RCONST(-0.118643265417);
    ARK_A(A,3,3) = RCONST(0.435866521508);
    ARK_A(A,4,0) = RCONST(0.896869652944);
    ARK_A(A,4,1) = RCONST(0.0182725272734);
    ARK_A(A,4,2) = RCONST(-0.0845900310706);
    ARK_A(A,4,3) = RCONST(-0.266418670647);
    ARK_A(A,4,4) = RCONST(0.435866521508);

    b[0] = RCONST(0.896869652944);
    b[1] = RCONST(0.0182725272734);
    b[2] = RCONST(-0.0845900310706);
    b[3] = RCONST(-0.266418670647);
    b[4] = RCONST(0.435866521508);

    b2[0] = RCONST(0.776691932910);
    b2[1] = RCONST(0.0297472791484);
    b2[2] = RCONST(-0.0267440239074);
    b2[3] = RCONST(0.220304811849);

    c[0] = RCONST(0.435866521508);
    c[1] = RCONST(-0.7);
    c[2] = RCONST(0.8);
    c[3] = RCONST(0.924556761814);
    c[4] = RCONST(1.0);
    break;

  case(20):    /* SDIRK-5-4 */
    *s = 5;
    *q = 4;
    *p = 3;
    ARK_A(A,0,0) = RCONST(0.25);
    ARK_A(A,1,0) = RCONST(0.5);
    ARK_A(A,1,1) = RCONST(0.25);
    ARK_A(A,2,0) = RCONST(17.0)/RCONST(50.0);
    ARK_A(A,2,1) = RCONST(-1.0)/RCONST(25.0);
    ARK_A(A,2,2) = RCONST(0.25);
    ARK_A(A,3,0) = RCONST(371.0)/RCONST(1360.0);
    ARK_A(A,3,1) = RCONST(-137.0)/RCONST(2720.0);
    ARK_A(A,3,2) = RCONST(15.0)/RCONST(544.0);
    ARK_A(A,3,3) = RCONST(0.25);
    ARK_A(A,4,0) = RCONST(25.0)/RCONST(24.0);
    ARK_A(A,4,1) = RCONST(-49.0)/RCONST(48.0);
    ARK_A(A,4,2) = RCONST(125.0)/RCONST(16.0);
    ARK_A(A,4,3) = RCONST(-85.0)/RCONST(12.0);
    ARK_A(A,4,4) = RCONST(0.25);

    b[0] = RCONST(25.0)/RCONST(24.0);
    b[1] = RCONST(-49.0)/RCONST(48.0);
    b[2] = RCONST(125.0)/RCONST(16.0);
    b[3] = RCONST(-85.0)/RCONST(12.0);
    b[4] = RCONST(0.25);

    b2[0] = RCONST(59.0)/RCONST(48.0);
    b2[1] = RCONST(-17.0)/RCONST(96.0);
    b2[2] = RCONST(225.0)/RCONST(32.0);
    b2[3] = RCONST(-85.0)/RCONST(12.0);

    c[0] = RCONST(0.25);
    c[1] = RCONST(0.75);
    c[2] = RCONST(11.0)/RCONST(20.0);
    c[3] = RCONST(0.5);
    c[4] = RCONST(1.0);
    break;

  case(21):    /* Kvaerno(5,3,4)-ESDIRK */
    *s = 5;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(0.4358665215); 
    ARK_A(A,1,1) = RCONST(0.4358665215); 
    ARK_A(A,2,0) = RCONST(0.140737774731968);
    ARK_A(A,2,1) = RCONST(-0.108365551378832);
    ARK_A(A,2,2) = RCONST(0.4358665215);
    ARK_A(A,3,0) = RCONST(0.102399400616089);
    ARK_A(A,3,1) = RCONST(-0.376878452267324);
    ARK_A(A,3,2) = RCONST(0.838612530151233);
    ARK_A(A,3,3) = RCONST(0.4358665215);
    ARK_A(A,4,0) = RCONST(0.157024897860995);
    ARK_A(A,4,1) = RCONST(0.117330441357768);
    ARK_A(A,4,2) = RCONST(0.61667803039168);
    ARK_A(A,4,3) = RCONST(-0.326899891110444);
    ARK_A(A,4,4) = RCONST(0.4358665215);

    b[0] = RCONST(0.157024897860995);
    b[1] = RCONST(0.117330441357768);
    b[2] = RCONST(0.61667803039168);
    b[3] = RCONST(-0.326899891110444);
    b[4] = RCONST(0.4358665215);

    b2[0] = RCONST(0.102399400616089);
    b2[1] = RCONST(-0.376878452267324);
    b2[2] = RCONST(0.838612530151233);
    b2[3] = RCONST(0.4358665215);

    c[1] = RCONST(0.871733043);
    c[2] = RCONST(0.468238744853136);
    c[3] = RCONST(1.0);
    c[4] = RCONST(1.0);
    break;

  case(22):    /* ARK4(3)6L[2]SA-ESDIRK */
    *s = 6;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,1,1) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,2,0) = RCONST(8611.0)/RCONST(62500.0);
    ARK_A(A,2,1) = RCONST(-1743.0)/RCONST(31250.0);
    ARK_A(A,2,2) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,3,0) = RCONST(5012029.0)/RCONST(34652500.0);
    ARK_A(A,3,1) = RCONST(-654441.0)/RCONST(2922500.0);
    ARK_A(A,3,2) = RCONST(174375.0)/RCONST(388108.0);
    ARK_A(A,3,3) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,4,0) = RCONST(15267082809.0)/RCONST(155376265600.0);
    ARK_A(A,4,1) = RCONST(-71443401.0)/RCONST(120774400.0);
    ARK_A(A,4,2) = RCONST(730878875.0)/RCONST(902184768.0);
    ARK_A(A,4,3) = RCONST(2285395.0)/RCONST(8070912.0);
    ARK_A(A,4,4) = RCONST(1.0)/RCONST(4.0);
    ARK_A(A,5,0) = RCONST(82889.0)/RCONST(524892.0);
    ARK_A(A,5,2) = RCONST(15625.0)/RCONST(83664.0);
    ARK_A(A,5,3) = RCONST(69875.0)/RCONST(102672.0);
    ARK_A(A,5,4) = RCONST(-2260.0)/RCONST(8211.0);
    ARK_A(A,5,5) = RCONST(1.0)/RCONST(4.0);

    b[0] = RCONST(82889.0)/RCONST(524892.0);
    b[2] = RCONST(15625.0)/RCONST(83664.0);
    b[3] = RCONST(69875.0)/RCONST(102672.0);
    b[4] = RCONST(-2260.0)/RCONST(8211.0);
    b[5] = RCONST(1.0)/RCONST(4.0);

    c[1] = RCONST(1.0)/RCONST(2.0);
    c[2] = RCONST(83.0)/RCONST(250.0);
    c[3] = RCONST(31.0)/RCONST(50.0);
    c[4] = RCONST(17.0)/RCONST(20.0);
    c[5] = RCONST(1.0);

    b2[0] = RCONST(4586570599.0)/RCONST(29645900160.0);
    b2[2] = RCONST(178811875.0)/RCONST(945068544.0);
    b2[3] = RCONST(814220225.0)/RCONST(1159782912.0);
    b2[4] = RCONST(-3700637.0)/RCONST(11593932.0);
    b2[5] = RCONST(61727.0)/RCONST(225920.0);
    break;

  case(23):    /* Sayfy-Aburub-4-3-EDIRK */
    *s = 6;
    *q = 4;
    *p = 3;
    ARK_A(A,1,0) = RCONST(1.0);
    ARK_A(A,1,1) = RCONST(0.788675134594813);
    ARK_A(A,2,0) = RCONST(2.943375672974064);
    ARK_A(A,2,1) = RCONST(-2.732050807568877);
    ARK_A(A,2,2) = RCONST(0.788675134594813);
    ARK_A(A,3,0) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,3,1) = RCONST(2.0)/RCONST(3.0);
    ARK_A(A,3,2) = RCONST(1.0)/RCONST(6.0);
    ARK_A(A,4,0) = RCONST(-0.423883252702594);
    ARK_A(A,4,1) = RCONST(0.20464164);
    ARK_A(A,4,2) = RCONST(0.719241612702594);
    ARK_A(A,4,4) = RCONST(1.9318);
    ARK_A(A,5,0) = RCONST(2.695533010810374);
    ARK_A(A,5,1) = RCONST(-5.391066021620748);
    ARK_A(A,5,2) = RCONST(1.695533010810374);
    ARK_A(A,5,4) = RCONST(2.0);
    ARK_A(A,5,5) = RCONST(1.9318);

    b[0] = RCONST(1.0)/RCONST(6.0);
    b[1] = RCONST(1.0)/RCONST(3.0);
    b[2] = RCONST(1.0)/RCONST(12.0);
    b[4] = RCONST(1.0)/RCONST(3.0);
    b[5] = RCONST(1.0)/RCONST(12.0);

    b2[0] = RCONST(1.0)/RCONST(6.0);
    b2[1] = RCONST(2.0)/RCONST(3.0);
    b2[2] = RCONST(1.0)/RCONST(6.0);

    c[1] = RCONST(1.0)/RCONST(2.0);
    c[2] = RCONST(1.0);
    c[3] = RCONST(1.0);
    c[4] = RCONST(1.0)/RCONST(2.0);
    c[5] = RCONST(1.0);
    break;

  case(24):    /* Ismail(7,4,5)-ESDIRK */
    *s = 7;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(0.28589); 
    ARK_A(A,1,1) = RCONST(0.28589); 
    ARK_A(A,2,0) = RCONST(0.142945000375866); 
    ARK_A(A,2,1) = RCONST(0.924011005); 
    ARK_A(A,2,2) = RCONST(0.28589); 
    ARK_A(A,3,0) = RCONST(0.168035986); 
    ARK_A(A,3,1) = RCONST(-0.04941651); 
    ARK_A(A,3,2) = RCONST(-0.004509476); 
    ARK_A(A,3,3) = RCONST(0.28589); 
    ARK_A(A,4,0) = RCONST(0.182315003); 
    ARK_A(A,4,1) = RCONST(-0.112951603); 
    ARK_A(A,4,2) = RCONST(-0.027793233); 
    ARK_A(A,4,3) = RCONST(0.422539833); 
    ARK_A(A,4,4) = RCONST(0.28589); 
    ARK_A(A,5,0) = RCONST(0.247563917); 
    ARK_A(A,5,1) = RCONST(-0.425378071); 
    ARK_A(A,5,2) = RCONST(-0.107036282); 
    ARK_A(A,5,3) = RCONST(0.395700134); 
    ARK_A(A,5,4) = RCONST(0.503260302); 
    ARK_A(A,5,5) = RCONST(0.28589); 
    ARK_A(A,6,0) = RCONST(0.130018035);
    ARK_A(A,6,2) = RCONST(-0.019290177);
    ARK_A(A,6,3) = RCONST(0.535386266);
    ARK_A(A,6,4) = RCONST(0.234313169);
    ARK_A(A,6,5) = RCONST(-0.166317293);
    ARK_A(A,6,6) = RCONST(0.28589);

    b[0] = RCONST(0.130018035);
    b[2] = RCONST(-0.019290177);
    b[3] = RCONST(0.535386266);
    b[4] = RCONST(0.234313169);
    b[5] = RCONST(-0.166317293);
    b[6] = RCONST(0.28589);

    b2[0] = RCONST(-0.094388662);
    b2[2] = RCONST(-0.039782614);
    b2[3] = RCONST(0.745608552);
    b2[4] = RCONST(-0.505129807);
    b2[5] = RCONST(0.704915206);
    b2[6] = RCONST(0.28589);

    c[1] = RCONST(0.57178);
    c[2] = RCONST(1.352846005375866);
    c[3] = RCONST(0.4);
    c[4] = RCONST(0.75);
    c[5] = RCONST(0.9);
    c[6] = RCONST(1);
    break;

  case(25):    /* Kvaerno(7,4,5)-ESDIRK */
    *s = 7;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(0.26);
    ARK_A(A,1,1) = RCONST(0.26);
    ARK_A(A,2,0) = RCONST(0.13);
    ARK_A(A,2,1) = RCONST(0.84033320996790809);
    ARK_A(A,2,2) = RCONST(0.26);
    ARK_A(A,3,0) = RCONST(0.22371961478320505);
    ARK_A(A,3,1) = RCONST(0.47675532319799699);
    ARK_A(A,3,2) = RCONST(-0.06470895363112615);
    ARK_A(A,3,3) = RCONST(0.26);
    ARK_A(A,4,0) = RCONST(0.16648564323248321);
    ARK_A(A,4,1) = RCONST(0.10450018841591720);
    ARK_A(A,4,2) = RCONST(0.03631482272098715);
    ARK_A(A,4,3) = RCONST(-0.13090704451073998);
    ARK_A(A,4,4) = RCONST(0.26);
    ARK_A(A,5,0) = RCONST(0.13855640231268224);
    ARK_A(A,5,2) = RCONST(-0.04245337201752043);
    ARK_A(A,5,3) = RCONST(0.02446657898003141);
    ARK_A(A,5,4) = RCONST(0.61943039072480676);
    ARK_A(A,5,5) = RCONST(0.26);
    ARK_A(A,6,0) = RCONST(0.13659751177640291);
    ARK_A(A,6,2) = RCONST(-0.05496908796538376);
    ARK_A(A,6,3) = RCONST(-0.04118626728321046);
    ARK_A(A,6,4) = RCONST(0.62993304899016403);
    ARK_A(A,6,5) = RCONST(0.06962479448202728);
    ARK_A(A,6,6) = RCONST(0.26);

    b[0] = RCONST(0.13659751177640291);
    b[2] = RCONST(-0.05496908796538376);
    b[3] = RCONST(-0.04118626728321046);
    b[4] = RCONST(0.62993304899016403);
    b[5] = RCONST(0.06962479448202728);
    b[6] = RCONST(0.26);

    b2[0] = RCONST(0.13855640231268224);
    b2[2] = RCONST(-0.04245337201752043);
    b2[3] = RCONST(0.02446657898003141);
    b2[4] = RCONST(0.61943039072480676);
    b2[5] = RCONST(0.26);

    c[1] = RCONST(0.52);
    c[2] = RCONST(1.230333209967908);
    c[3] = RCONST(0.895765984350076);
    c[4] = RCONST(0.436393609858648);
    c[5] = RCONST(1.0);
    c[6] = RCONST(1.0);
    break;

  case(26):    /* ARK5(4)8L[2]SA-ESDIRK */
    *s = 8;
    *q = 5;
    *p = 4;
    ARK_A(A,1,0) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,1,1) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,2,0) = RCONST(41.0)/RCONST(400.0);
    ARK_A(A,2,1) = RCONST(-567603406766.0)/RCONST(11931857230679.0);
    ARK_A(A,2,2) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,3,0) = RCONST(683785636431.0)/RCONST(9252920307686.0);
    ARK_A(A,3,2) = RCONST(-110385047103.0)/RCONST(1367015193373.0);
    ARK_A(A,3,3) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,4,0) = RCONST(3016520224154.0)/RCONST(10081342136671.0);
    ARK_A(A,4,2) = RCONST(30586259806659.0)/RCONST(12414158314087.0);
    ARK_A(A,4,3) = RCONST(-22760509404356.0)/RCONST(11113319521817.0);
    ARK_A(A,4,4) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,5,0) = RCONST(218866479029.0)/RCONST(1489978393911.0);
    ARK_A(A,5,2) = RCONST(638256894668.0)/RCONST(5436446318841.0);
    ARK_A(A,5,3) = RCONST(-1179710474555.0)/RCONST(5321154724896.0);
    ARK_A(A,5,4) = RCONST(-60928119172.0)/RCONST(8023461067671.0);
    ARK_A(A,5,5) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,6,0) = RCONST(1020004230633.0)/RCONST(5715676835656.0);
    ARK_A(A,6,2) = RCONST(25762820946817.0)/RCONST(25263940353407.0);
    ARK_A(A,6,3) = RCONST(-2161375909145.0)/RCONST(9755907335909.0);
    ARK_A(A,6,4) = RCONST(-211217309593.0)/RCONST(5846859502534.0);
    ARK_A(A,6,5) = RCONST(-4269925059573.0)/RCONST(7827059040749.0);
    ARK_A(A,6,6) = RCONST(41.0)/RCONST(200.0);
    ARK_A(A,7,0) = RCONST(-872700587467.0)/RCONST(9133579230613.0);
    ARK_A(A,7,3) = RCONST(22348218063261.0)/RCONST(9555858737531.0);
    ARK_A(A,7,4) = RCONST(-1143369518992.0)/RCONST(8141816002931.0);
    ARK_A(A,7,5) = RCONST(-39379526789629.0)/RCONST(19018526304540.0);
    ARK_A(A,7,6) = RCONST(32727382324388.0)/RCONST(42900044865799.0);
    ARK_A(A,7,7) = RCONST(41.0)/RCONST(200.0);

    b[0] = RCONST(-872700587467.0)/RCONST(9133579230613.0);
    b[3] = RCONST(22348218063261.0)/RCONST(9555858737531.0);
    b[4] = RCONST(-1143369518992.0)/RCONST(8141816002931.0);
    b[5] = RCONST(-39379526789629.0)/RCONST(19018526304540.0);
    b[6] = RCONST(32727382324388.0)/RCONST(42900044865799.0);
    b[7] = RCONST(41.0)/RCONST(200.0);

    b2[0] = RCONST(-975461918565.0)/RCONST(9796059967033.0);
    b2[3] = RCONST(78070527104295.0)/RCONST(32432590147079.0);
    b2[4] = RCONST(-548382580838.0)/RCONST(3424219808633.0);
    b2[5] = RCONST(-33438840321285.0)/RCONST(15594753105479.0);
    b2[6] = RCONST(3629800801594.0)/RCONST(4656183773603.0);
    b2[7] = RCONST(4035322873751.0)/RCONST(18575991585200.0);

    c[1] = RCONST(41.0)/RCONST(100.0);
    c[2] = RCONST(2935347310677.0)/RCONST(11292855782101.0);
    c[3] = RCONST(1426016391358.0)/RCONST(7196633302097.0);
    c[4] = RCONST(92.0)/RCONST(100.0);
    c[5] = RCONST(24.0)/RCONST(100.0);
    c[6] = RCONST(3.0)/RCONST(5.0);
    c[7] = RCONST(1.0);
    break;

  default:

    arkProcessError(NULL, ARK_ILL_INPUT, "ARKODE", 
		    "ARKodeGetButcherTable", "Unknown Butcher table");
    return(ARK_ILL_INPUT);

  }

  return(ARK_SUCCESS);
}


/*---------------------------------------------------------------
  EOF
  ---------------------------------------------------------------*/