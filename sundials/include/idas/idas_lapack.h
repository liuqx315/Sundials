/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * ----------------------------------------------------------------- 
 * Programmer: Radu Serban @ LLNL
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
 * Header file for the IDAS dense linear solver IDASLAPACK.
 * -----------------------------------------------------------------
 */

#ifndef _IDALAPACK_H
#define _IDALAPACK_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <idas/idas_direct.h>
#include <sundials/sundials_lapack.h>

/*
 * -----------------------------------------------------------------
 * Function : IDALapackDense
 * -----------------------------------------------------------------
 * A call to the IDALapackDense function links the main integrator
 * with the IDALAPACK linear solver using dense Jacobians.
 *
 * ida_mem is the pointer to the integrator memory returned by
 *           IDACreate.
 *
 * N is the size of the DAE system.
 *
 * The return value of IDALapackDense is one of:
 *    IDADLS_SUCCESS   if successful
 *    IDADLS_MEM_NULL  if the IDAS memory was NULL
 *    IDADLS_MEM_FAIL  if there was a memory allocation failure
 *    IDADLS_ILL_INPUT if a required vector operation is missing
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDALapackDense(void *ida_mem, int N);

/*
 * -----------------------------------------------------------------
 * Function : IDALapackBand
 * -----------------------------------------------------------------
 * A call to the IDALapackBand function links the main integrator
 * with the IDALAPACK linear solver using banded Jacobians. 
 *
 * ida_mem is the pointer to the integrator memory returned by
 *           IDACreate.
 *
 * N is the size of the ODE system.
 *
 * mupper is the upper bandwidth of the band Jacobian approximation.
 *
 * mlower is the lower bandwidth of the band Jacobian approximation.
 *
 * The return value of IDALapackBand is one of:
 *    IDADLS_SUCCESS   if successful
 *    IDADLS_MEM_NULL  if the IDAS memory was NULL
 *    IDADLS_MEM_FAIL  if there was a memory allocation failure
 *    IDADLS_ILL_INPUT if a required vector operation is missing
 *                        or if a bandwidth has an illegal value.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDALapackBand(void *ida_mem, int N, int mupper, int mlower);

/*
 * -----------------------------------------------------------------
 * Function: IDALapackDenseB
 * -----------------------------------------------------------------
 * IDALapackDenseB links the main IDAS integrator with the dense
 * IDALAPACK linear solver for the backward integration.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDALapackDenseB(void *ida_mem, int NeqB);

/*
 * -----------------------------------------------------------------
 * Function: IDALapackBandB
 * -----------------------------------------------------------------
 * IDALapackBandB links the main IDAS integrator with the band
 * IDALAPACK linear solver for the backward integration.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDALapackBandB(void *ida_mem, int NeqB, int mupperB, int mlowerB);


#ifdef __cplusplus
}
#endif

#endif
