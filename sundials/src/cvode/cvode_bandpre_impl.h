/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * ----------------------------------------------------------------- 
 * Programmer(s): Michael Wittman, Alan C. Hindmarsh and
 *                Radu Serban @ LLNL
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
 * Implementation header file for the CVBANDPRE module.
 * -----------------------------------------------------------------
 */

#ifndef _CVBANDPRE_IMPL_H
#define _CVBANDPRE_IMPL_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <cvode/cvode_bandpre.h>
#include <sundials/sundials_band.h>
#include <sundials/sundials_direct.h>

/*
 * -----------------------------------------------------------------
 * Type: CVBandPrecData
 * -----------------------------------------------------------------
 */

typedef struct CVBandPrecDataRec {

  /* Data set by user in CVBandPrecInit */

  long int N;
  long int ml, mu;

  /* Data set by CVBandPrecSetup */

  DlsMat savedJ;
  DlsMat savedP;
  long int *lpivots;

  /* Rhs calls */

  long int nfeBP;

  /* Pointer to cvode_mem */

  void *cvode_mem;

} *CVBandPrecData;

/*
 * -----------------------------------------------------------------
 * CVBANDPRE error messages
 * -----------------------------------------------------------------
 */

#define MSGBP_MEM_NULL       "Integrator memory is NULL."
#define MSGBP_LMEM_NULL      "Linear solver memory is NULL. One of the SPILS linear solvers must be attached."
#define MSGBP_MEM_FAIL       "A memory request failed."
#define MSGBP_BAD_NVECTOR    "A required vector operation is not implemented."
#define MSGBP_PMEM_NULL      "Band preconditioner memory is NULL. CVBandPrecInit must be called."
#define MSGBP_RHSFUNC_FAILED "The right-hand side routine failed in an unrecoverable manner."


#ifdef __cplusplus
}
#endif

#endif
