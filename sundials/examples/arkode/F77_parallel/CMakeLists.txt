# ---------------------------------------------------------------
# Programmer:  Daniel R. Reynolds @ SMU
# ---------------------------------------------------------------
# Copyright (c) 2013, Southern Methodist University.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# CMakeLists.txt file for the FARKODE parallel examples


# Add variable FARKODE_examples with the names of the parallel ARKODE examples

SET(FARKODE_examples
  "fark_diag_non_p\;1\;4"
  "fark_diag_kry_bbd_p\;1\;4"
  )

# Add variable ARKODE_extras with the names of auxiliary files to install

SET(ARKODE_extras
  )

# Check whether we use MPI compiler scripts.
# If yes, then change the Fortran compiler to the MPIF77 script.
# If not, then add the MPI include directory for MPI headers.

IF(MPI_MPIF77 )
  # use MPI_MPIF77 as the compiler
  SET(CMAKE_Fortran_COMPILER ${MPI_MPIF77})
ELSE(MPI_MPIF77)
  # add MPI_INCLUDE_PATH to include directories
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF(MPI_MPIF77)

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(ARKODE_LIB sundials_arkode_static)
  SET(NVECP_LIB sundials_nvecparallel_static)
  SET(FNVECP_LIB sundials_fnvecparallel_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(ARKODE_LIB sundials_arkode_shared)
  SET(NVECP_LIB sundials_nvecparallel_shared)
  SET(FNVECP_LIB sundials_fnvecparallel_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Only static FCMIX libraries are available

SET(FARKODE_LIB sundials_farkode_static)

# Set-up linker flags and link libraries

SET(SUNDIALS_LIBS ${FARKODE_LIB} ${ARKODE_LIB} ${FNVECP_LIB} ${NVECP_LIB} ${EXTRA_LINK_LIBS})
IF(LAPACK_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${LAPACK_LIBRARIES})
ENDIF(LAPACK_FOUND)

IF(KLU_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${KLU_LIBRARIES})
ENDIF(KLU_FOUND)

IF(SUPERLUMT_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${SUPERLUMT_LIBRARIES})
ENDIF(SUPERLUMT_FOUND)

# Add the build and install targets for each ARKODE example

FOREACH(example_tuple ${FARKODE_examples})
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 number_of_nodes)
  list(GET example_tuple 2 number_of_tasks)
  # first item is example
  ADD_EXECUTABLE(${example} ${example}.f)
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
  SUNDIALS_ADD_TEST(${example} ${example} MPI_NPROCS ${number_of_tasks})
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
  IF(NOT MPI_MPIF77)
    TARGET_LINK_LIBRARIES(${example} ${MPI_LIBRARY} ${MPI_EXTRA_LIBRARIES})
  ENDIF(NOT MPI_MPIF77)
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.f ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/F77_parallel)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example_tuple ${FARKODE_examples})

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/F77_parallel)

  # Install the extra files
  FOREACH(extrafile ${ARKODE_extras})
    INSTALL(FILES ${extrafile} DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/F77_parallel)
  ENDFOREACH(extrafile ${ARKODE_extras})

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "ARKODE")
  SET(SOLVER_LIB "sundials_arkode")
  SET(SOLVER_FLIB "sundials_farkode")
  FOREACH(example_tuple ${FARKODE_examples})
    list(GET example_tuple 0 example)
    LIST2STRING(example EXAMPLES)
  ENDFOREACH(example_tuple ${FARKODE_examples})

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_parallel_F77_ex.in
      ${PROJECT_BINARY_DIR}/examples/arkode/F77_parallel/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/arkode/F77_parallel/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/F77_parallel 
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_parallel_F77_ex.in
      ${PROJECT_BINARY_DIR}/examples/arkode/F77_parallel/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/arkode/F77_parallel/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/F77_parallel 
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
