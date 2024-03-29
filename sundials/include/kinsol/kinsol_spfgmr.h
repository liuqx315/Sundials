/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * -----------------------------------------------------------------
 * Programmer(s): Carol Woodward @ LLNL, based on kinsol_spgmr.h
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
 * This is the header file for the KINSOL Scaled Preconditioned GMRES     
 * linear solver module, KINSPFGMR.    
 * -----------------------------------------------------------------
 */

#ifndef _KINSPFGMR_H
#define _KINSPFGMR_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <kinsol/kinsol_spils.h>
#include <sundials/sundials_spfgmr.h>

/*
 * -----------------------------------------------------------------
 * Function : KINSpfgmr
 * -----------------------------------------------------------------
 * KINSpfgmr links the main KINSOL solver module with the SPFGMR
 * linear solver module. The routine establishes the inter-module
 * interface by setting the generic KINSOL pointers linit, lsetup,
 * lsolve, and lfree to KINSpfgmrInit, KINSpfgmrSetup, KINSpfgmrSolve,
 * and KINSpfgmrFree, respectively.
 *
 *  kinmem  pointer to an internal memory block allocated during a
 *          prior call to KINCreate
 *
 *  maxl  maximum allowable dimension of Krylov subspace (passing
 *        a value of 0 (zero) will cause the default value
 *        KINSPILS_MAXL (predefined constant) to be used)
 *
 * -----------------------------------------------------------------
 * KINSpfgmr Return Values
 * -----------------------------------------------------------------
 *
 * The possible return values for the KINSpfgmr subroutine are the
 * following:
 *
 * KINSPILS_SUCCESS : means the KINSPFGMR linear solver module
 *                    (implementation of the GMRES method) was
 *                    successfully initialized - allocated system
 *                    memory and set shared variables to default
 *                    values [0]
 *
 * KINSPILS_MEM_NULL : means a NULL KINSOL memory block pointer was
 *                     given (must call the KINCreate and KINMalloc
 *                     memory allocation subroutines prior to
 *                     calling KINSpfgmr) [-1]
 *
 * KINSPILS_MEM_FAIL : means either insufficient system resources
 *                     were available to allocate memory for the main
 *                     KINSPFGMR data structure (type KINSpfgmrMemRec),
 *                     or the SpfgmrMalloc subroutine failed (unable
 *                     to allocate enough system memory for vector
 *                     storage and/or the main SPFGMR data structure
 *                     (type SpfgmrMemRec)) [-4]
 *
 * KINSPILS_ILL_INPUT : means a supplied parameter was invalid
 *                      (check error message) [-3]
 *
 * The above constants are defined in kinsol_spils.h
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int KINSpfgmr(void *kinmem, int maxl);


#ifdef __cplusplus
}
#endif

#endif
