# ---------------------------------------------------------------
# $Revision: 4137 $
# $Date: 2014-06-15 12:26:15 -0700 (Sun, 15 Jun 2014) $
# ---------------------------------------------------------------
# Programmer:  Steven Smith @ LLNL
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
# CMakeLists.txt file for nvector examples

# Add variable nvector_parallel_examples with the names of the nvector examples

SET(nvector_parallel_examples
  test_nvector_mpi
)

# Add source directory to include directories
INCLUDE_DIRECTORIES(. ..)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(NVECS_LIB sundials_nvecparallel_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(NVECS_LIB sundials_nvecparallel_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Set-up linker flags and link libraries

SET(SUNDIALS_LIBS ${NVECS_LIB} ${EXTRA_LINK_LIBS})

IF(MPI_MPICC)
  # use MPI_MPICC as the compiler
  SET(CMAKE_C_COMPILER ${MPI_MPICC})
ELSE(MPI_MPICC)
  # add MPI_INCLUDE_PATH to include directories
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
  TARGET_LINK_LIBRARIES(${example} MPI_LIBRARIES)
ENDIF(MPI_MPICC)

# Add the build and install targets for each nvector example

FOREACH(example ${nvector_parallel_examples})

  ADD_EXECUTABLE(${example} ${example}.c ../test_nvector.c ../../../src/sundials/sundials_nvector.c)
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")

  # Run sequentially
  SUNDIALS_ADD_TEST("${example}" ${example})

  # Run sequentially with arguments
  SUNDIALS_ADD_TEST("${example}_1000_0" ${example} TEST_ARGS 1000 0)

  # Run in parallel on 4 procs
  SUNDIALS_ADD_TEST("${example}_4" ${example} MPI_NPROCS 4)
  
  # Parallel with additional arguments and supplying an answer file
  SUNDIALS_ADD_TEST("${example}_4_1000_0" ${example} MPI_NPROCS 4 TEST_ARGS 1000 0 ANSWER_FILE ${example})

  # Specify floating point precision and integer percentage for comparison
  SUNDIALS_ADD_TEST("${example}_precision" ${example} MPI_NPROCS 4 TEST_ARGS 1000 0 ANSWER_FILE ${example}
    FLOAT_PRECISION 5 INTEGER_PERCENTAGE 0)
  
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})

  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.c ../test_nvector.c ../test_nvector.h DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/parallel)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example ${nvector_parallel_examples})

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/parallel)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER_LIB "sundials_nvecparallel")
  LIST2STRING(nvector_parallel_examples EXAMPLES)

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_parallel_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/nvector/parallel/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/nvector/parallel/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/parallel
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_parallel_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/nvector/parallel/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/nvector/parallel/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/parallel
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
