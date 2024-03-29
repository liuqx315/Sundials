# ---------------------------------------------------------------
# Programmer:  Daniel R. Reynolds @ SMU
# ---------------------------------------------------------------
# Copyright (c) 2014, Southern Methodist University.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# CMakeLists.txt file for ARKODE OpenMP examples

# Only include tests if OpenMP is enabled
IF(OPENMP_FOUND)

# Add variable ARKode_examples_OMP with the names of the openmp ARKode examples
SET(ARKODE_examples_OMP
  ark_brusselator1D_omp
  )

# Add variable ARKODE_extras_OMP with the names of auxiliary files to install
SET(ARKODE_extras_OMP
  plot_brusselator1D.py
  )

ENDIF(OPENMP_FOUND)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(ARKODE_LIB sundials_arkode_static)
  SET(NVECOMP_LIB sundials_nvecopenmp_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(ARKODE_LIB sundials_arkode_shared)
  SET(NVECOMP_LIB sundials_nvecopenmp_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Set-up linker flags and link libraries
SET(SUNDIALS_LIBS ${ARKODE_LIB} ${NVECOMP_LIB} ${EXTRA_LINK_LIBS})
IF(LAPACK_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${LAPACK_LIBRARIES})
ENDIF(LAPACK_FOUND)

IF(KLU_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${KLU_LIBRARIES})
ENDIF(KLU_FOUND)

IF(SUPERLUMT_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${SUPERLUMT_LIBRARIES})
ENDIF(SUPERLUMT_FOUND)

# update the compilation flags to include OpenMP support
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_FLAGS}")

# Add the build and install targets for each ARKODE example
FOREACH(example ${ARKODE_examples_OMP})
  ADD_EXECUTABLE(${example} ${example}.c)
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
  SUNDIALS_ADD_TEST(${example} ${example})
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example ${ARKODE_examples_OMP})

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)

  # Install the extra files
  FOREACH(extrafile ${ARKODE_extras_OMP})
    INSTALL(FILES ${extrafile} DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)
  ENDFOREACH(extrafile ${ARKODE_extras_OMP})

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "ARKODE")
  SET(SOLVER_LIB "sundials_arkode")
  LIST2STRING(ARKODE_examples_OMP EXAMPLES)

  STRING (REPLACE ";" " " TMP_STR ${EXAMPLES})
  SET(EXAMPLES ${TMP_STR})


  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_openmp_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp 
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_openmp_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
