/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * ----------------------------------------------------------------- 
 * Programmer(s): Aaron Collier and Radu Serban @ LLNL
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
 * This is the public header file for the IDAS scaled preconditioned
 * Bi-CGSTAB linear solver module, IDASPBCG.
 *
 * Part I contains function prototypes for using IDASPBCG on forward 
 * problems (DAE integration and/or FSA)
 *
 * Part II contains function prototypes for using IDASPBCG on adjoint 
 * (backward) problems
 * -----------------------------------------------------------------
 */

#ifndef _IDASSPBCG_H
#define _IDASSPBCG_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <idas/idas_spils.h>
#include <sundials/sundials_spbcgs.h>

/* 
 * -----------------------------------------------------------------
 * PART I - forward problems
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------
 * Function : IDASpbcg
 * -----------------------------------------------------------------
 * A call to the IDASpbcg function links the main integrator with
 * the IDASPBCG linear solver module. Its parameters are as
 * follows:
 *
 * IDA_mem   is the pointer to memory block returned by IDACreate.
 *
 * maxl      is the maximum Krylov subspace dimension, an
 *           optional input. Pass 0 to use the default value.
 *           Otherwise pass a positive integer.
 *
 * The return values of IDASpbcg are:
 *    IDASPILS_SUCCESS    if successful
 *    IDASPILS_MEM_NULL   if the IDAS memory was NULL
 *    IDASPILS_MEM_FAIL   if there was a memory allocation failure
 *    IDASPILS_ILL_INPUT  if there was illegal input.
 * The above constants are defined in idas_spils.h
 *
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDASpbcg(void *ida_mem, int maxl);

/* 
 * -----------------------------------------------------------------
 * PART II - backward problems
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDASpbcgB(void *ida_mem, int which, int maxlB);


#ifdef __cplusplus
}
#endif

#endif
