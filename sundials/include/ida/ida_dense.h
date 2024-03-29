/*
 * -----------------------------------------------------------------
 * $Revision: 4075 $
 * $Date: 2014-04-24 10:46:58 -0700 (Thu, 24 Apr 2014) $
 * ----------------------------------------------------------------- 
 * Programmer(s): Alan C. Hindmarsh and Radu Serban @ LLNL
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
 * This is the header file for the IDADENSE linear solver module.
 * -----------------------------------------------------------------
 */

#ifndef _IDADENSE_H
#define _IDADENSE_H

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

#include <ida/ida_direct.h>
#include <sundials/sundials_dense.h>

/*
 * -----------------------------------------------------------------
 * Function : IDADense
 * -----------------------------------------------------------------
 * A call to the IDADense function links the main integrator      
 * with the IDADENSE linear solver module.                        
 *                                                                
 * ida_mem is the pointer to integrator memory returned by        
 *     IDACreate.                                                 
 *                                                                
 * Neq  is the problem size                                       
 *                                                                
 * IDADense returns:                                              
 *     IDADLS_SUCCESS   = 0  if successful                              
 *     IDADLS_LMEM_FAIL = -1 if there was a memory allocation failure   
 *     IDADLS_ILL_INPUT = -2 if NVECTOR found incompatible           
 *                                                                
 * NOTE: The dense linear solver assumes a serial implementation  
 *       of the NVECTOR package. Therefore, IDADense will first
 *       test for a compatible N_Vector internal representation
 *       by checking that the functions N_VGetArrayPointer and
 *       N_VSetArrayPointer exist.
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int IDADense(void *ida_mem, long int Neq); 

#ifdef __cplusplus
}
#endif

#endif
