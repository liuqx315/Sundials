# ---------------------------------------------------------------
# $Revision: 4137 $
# $Date: 2014-06-15 12:26:15 -0700 (Sun, 15 Jun 2014) $
# ---------------------------------------------------------------
# Programmer:  Radu Serban @ LLNL
# ---------------------------------------------------------------
# LLNS Copyright Start
# Copyright (c) 2014, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End
# ---------------------------------------------------------------
# CMakeLists.txt file for the FIDA opemp examples


# Only include tests if Pthreads is enabled
IF(PTHREADS_FOUND)

# Add variable ida_examples with the names of the pthreads FIDA examples
SET(FIDA_examples
  fidaRoberts_dns_pthreads
  )

# Add variable FIDA_examples_BL with the names of the pthreads FIDA examples
# that use Lapack

SET(FIDA_examples_BL
  )
ENDIF(PTHREADS_FOUND)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(IDA_LIB sundials_ida_static)
  SET(NVECS_LIB sundials_nvecpthreads_static)
  SET(FNVECS_LIB sundials_fnvecpthreads_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(IDA_LIB sundials_ida_shared)
  SET(NVECS_LIB sundials_nvecpthreads_shared)
  SET(FNVECS_LIB sundials_fnvecpthreads_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Only static FCMIX libraries are available

SET(FIDA_LIB sundials_fida_static)

# Set-up linker flags and link libraries

SET(SUNDIALS_LIBS ${FIDA_LIB} ${IDA_LIB} ${FNVECS_LIB} ${NVECS_LIB} ${EXTRA_LINK_LIBS})
IF(LAPACK_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${LAPACK_LIBRARIES})
ENDIF(LAPACK_FOUND)

IF(KLU_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${KLU_LIBRARIES})
ENDIF(KLU_FOUND)

IF(SUPERLUMT_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${SUPERLUMT_LIBRARIES})
ENDIF(SUPERLUMT_FOUND)

# Add the build and install targets for each FIDA example

FOREACH(example ${FIDA_examples})
  ADD_EXECUTABLE(${example} ${example}.f)
  SUNDIALS_ADD_TEST(${example} ${example})
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.f ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/fcmix_pthreads)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example ${FIDA_examples})

# If Lapack support is enabled, add the build and install targets for
# the examples using Lapack

IF(LAPACK_FOUND)
  FOREACH(example ${FIDA_examples_BL})
    ADD_EXECUTABLE(${example} ${example}.f)
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.f ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/fcmix_pthreads)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${FIDA_examples_BL})
ENDIF(LAPACK_FOUND)

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/fcmix_pthreads)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "IDA")
  SET(SOLVER_LIB "sundials_ida")
  SET(SOLVER_FLIB "sundials_fida")
  LIST2STRING(FIDA_examples EXAMPLES)
  IF(LAPACK_FOUND)
    LIST2STRING(FIDA_examples_BL EXAMPLES_BL)
  ELSE(LAPACK_FOUND)
    SET(EXAMPLES_BL "")
  ENDIF(LAPACK_FOUND)

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_pthreads_F77_ex.in
      ${PROJECT_BINARY_DIR}/examples/ida/fcmix_pthreads/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/ida/fcmix_pthreads/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/fcmix_pthreads 
    )

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_pthreads_F77_ex.in
      ${PROJECT_BINARY_DIR}/examples/ida/fcmix_pthreads/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/ida/fcmix_pthreads/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/fcmix_pthreads
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
