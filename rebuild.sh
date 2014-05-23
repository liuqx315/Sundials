#!/bin/bash
# ----------------------------------------------------------------
# Programmer(s): Daniel R. Reynolds @ SMU
# ----------------------------------------------------------------
# Copyright (c) 2013, Southern Methodist University.
# All rights reserved.
# For details, see the LICENSE file.
# ----------------------------------------------------------------


# update the ARKode_dev source code
hg pull
hg up

# reconfigure and rebuild ARKode library -- requires 
# sundials to have previously been configured with cmake, 
# with directory structure:
#    <arkode>/sundials -- source directory (already exists)
#    <arkode>/build    -- build directory
#    <arkode>/install  -- installation directory
cd build_opt
cmake ../sundials/
make clean
make
make install