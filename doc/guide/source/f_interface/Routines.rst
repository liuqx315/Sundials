..
   Programmer(s): Daniel R. Reynolds @ SMU
   ----------------------------------------------------------------
   Copyright (c) 2013, Southern Methodist University.
   All rights reserved.
   For details, see the LICENSE file.
   ----------------------------------------------------------------

:tocdepth: 3


.. _FInterface.Routines:

FARKODE routines
===========================

In this section, we list the full set of user-callable functions
comprising the FARKODE solver interface.  For each function, we list
the corresponding ARKode functions, to provide a mapping between the
two solver interfaces.  Further documentation on each FARKODE function
is provided in the following sections, :ref:`FInterface.Usage`,
:ref:`FInterface.OptionalOutputs`, :ref:`FInterface.Rootfinding` and
:ref:`FInterface.Preconditioning`.  Additionally, all Fortran and C
functions below are hyperlinked to their definitions in the
documentation, for simplified access.



Interface to the NVECTOR modules
----------------------------------

* :f:func:`FNVINITS()` (defined by NVECTOR_SERIAL) interfaces to
  :c:func:`N_VNewEmpty_Serial()`.

* :f:func:`FNVINITS_OPENMP()` (defined by NVECTOR_OPENMP) interfaces to
  :c:func:`N_VNewEmpty_OpenMP()`.

* :f:func:`FNVINITS_PTHREADS()` (defined by NVECTOR_PTHREADS) interfaces to
  :c:func:`N_VNewEmpty_Pthreads()`.

* :f:func:`FNVINITP()` (defined by NVECTOR_PARALLEL) interfaces to
  :c:func:`N_VNewEmpty_Parallel()`. 


Interface to the main ARKODE module
--------------------------------------

* :f:func:`FARKMALLOC()` interfaces to :c:func:`ARKodeCreate()`,
  :c:func:`ARKodeSetUserData()`, and :c:func:`ARKodeInit()`, as well
  as one of :c:func:`ARKodeSStolerances()` or :c:func:`ARKodeSVtolerances()`.

* :f:func:`FARKREINIT()` interfaces to :c:func:`ARKodeReInit()`.

* :f:func:`FARKRESIZE()` interfaces to :c:func:`ARKodeResize()`.

* :f:func:`FARKSETIIN()` and :f:func:`FARKSETRIN()` interface to the
  ARKodeSet* functions (see :ref:`CInterface.OptionalInputs`).

* :f:func:`FARKEWTSET()` interfaces to :c:func:`ARKodeWFtolerances()`.

* :f:func:`FARKADAPTSET()` interfaces to :c:func:`ARKodeSetAdaptivityFn()`.

* :f:func:`FARKEXPSTABSET()` interfaces to :c:func:`ARKodeSetStabilityFn()`.

..
   * :f:func:`FARKSETDIAGNOSTICS()` interfaces to :c:func:`ARKodeSetDiagnostics()`.

* :f:func:`FARKODE()` interfaces to :c:func:`ARKode()`, the
  ARKodeGet* functions (see :ref:`CInterface.OptionalOutputs`), 
  and to the optional output functions for the selected linear
  solver module (see :ref:`CInterface.OptionalOutputs`). 

* :f:func:`FARKDKY()` interfaces to the interpolated output function
  :c:func:`ARKodeGetDky()`.

* :f:func:`FARKGETERRWEIGHTS()` interfaces to
  :c:func:`ARKodeGetErrWeights()`.

* :f:func:`FARKGETESTLOCALERR()` interfaces to
  :c:func:`ARKodeGetEstLocalErrors()`.

* :f:func:`FARKFREE()` interfaces to :c:func:`ARKodeFree()`.



Interface to the linear solver modules
---------------------------------------

* :f:func:`FARKDENSE()` interfaces to :c:func:`ARKDense()`.

* :f:func:`FARKLAPACKDENSE()` interfaces to :c:func:`ARKLapackDense()`.

* :f:func:`FARKDENSESETJAC()` interfaces to :c:func:`ARKDlsSetDenseJacFn()`.

* :f:func:`FARKBAND()` interfaces to :c:func:`ARKBand()`.

* :f:func:`FARKLAPACKBAND()` interfaces to :c:func:`ARKLapackBand()`.

* :f:func:`FARKBANDSETJAC()` interfaces to :c:func:`ARKDlsSetBandJacFn()`.

* :f:func:`FARKKLU()` interfaces to :c:func:`ARKKLU()`.

* :f:func:`FARKSUPERLUMT()` interfaces to :c:func:`ARKSuperLUMT()`.

* :f:func:`FARKSPGMR()` interfaces to :c:func:`ARKSpgmr()` and the SPGMR optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKSPGMRREINIT()` interfaces to the SPGMR optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKSPBCG()` interfaces to :c:func:`ARKSpbcg()` and the SPBCG optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKSPBCGREINIT()` interfaces to the SPBCG optional input
  functions.

* :f:func:`FARKSPTFQMR()` interfaces to :c:func:`ARKSptfqmr()` and the SPTFQMR optional
  input functions.

* :f:func:`FARKSPTFQMRREINIT()` interfaces to the SPTFQMR optional input
  functions.

* :f:func:`FARKSPFGMR()` interfaces to :c:func:`ARKSpfgmr()` and the SPFGMR optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKSPFGMRREINIT()` interfaces to the SPFGMR optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKPCG()` interfaces to :c:func:`ARKPcg()` and the PCG optional input
  functions (see :ref:`CInterface.ARKSpilsInputTable`).

* :f:func:`FARKPCGREINIT()` interfaces to the PCG optional input
  functions.

* :f:func:`FARKSPILSSETJAC()` interfaces to :c:func:`ARKSpilsSetJacTimesVecFn()`.

* :f:func:`FARKSPILSSETPREC()` interfaces to :c:func:`ARKSpilsSetPreconditioner()`.



.. _FInterface.UserSupplied:


User-supplied routines
---------------------------------------

As with the native C interface, the FARKode solver interface requires
user-supplied functions to specify the ODE problem to be solved.  In
contrast to the case of direct use of ARKode, and of most Fortran ODE
solvers, the names of all user-supplied routines here are fixed, in
order to maximize portability for the resulting mixed-language program. 
As a result, whether using a purely implicit, purely explicit, or
mixed implicit-explicit solver, routines for both :math:`f_E(t,y)` and
:math:`f_I(t,y)` must be provided by the user (though either of which
may do nothing): 

.. cssclass:: table-bordered

+---------------------------+-----------------------------------+
| FARKODE routine           | ARKode interface                  |
| (FORTRAN, user-supplied)  | function type                     |
+===========================+===================================+
| :f:func:`FARKIFUN()`      | :c:func:`ARKRhsFn()`              |
+---------------------------+-----------------------------------+
| :f:func:`FARKEFUN()`      | :c:func:`ARKRhsFn()`              |
+---------------------------+-----------------------------------+

In addition, as with the native C interface a user may provide
additional routines to assist in the solution process.  Each of the
following user-supplied routines is activated by calling the specified
"activation" routine, with the exception of :f:func:`FARKSPJAC()`
which is required whenever a sparse matrix solver is used: 

.. cssclass:: table-bordered

+--------------------------+-----------------------------------+------------------------------+
| FARKODE routine          | ARKode interface                  | FARKODE "activation" routine |
| (FORTRAN, user-supplied) | function type                     |                              |
+==========================+===================================+==============================+
| :f:func:`FARKDJAC()`     | :c:func:`ARKDlsDenseJacFn()`      | :f:func:`FARKDENSESETJAC()`  |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKBJAC()`     | :c:func:`ARKDlsBandJacFn()`       | :f:func:`FARKBANDSETJAC()`   |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKSPJAC()`    | :c:func:`ARKSlsSparseJacFn()`     |                              |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKPSET()`     | :c:func:`ARKSpilsPrecSetupFn()`   | :f:func:`FARKSPILSSETPREC()` |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKPSOL()`     | :c:func:`ARKSpilsPrecSolveFn()`   | :f:func:`FARKSPILSSETPREC()` |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKJTIMES()`   | :c:func:`ARKSpilsJacTimesVecFn()` | :f:func:`FARKSPILSSETJAC()`  |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKEWT()`      | :c:func:`ARKEwtFn()`              | :f:func:`FARKEWTSET()`       |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKADAPT()`    | :c:func:`ARKAdaptFn()`            | :f:func:`FARKADAPTSET()`     |
+--------------------------+-----------------------------------+------------------------------+
| :f:func:`FARKEXPSTAB()`  | :c:func:`ARKExpStabFn()`          | :f:func:`FARKEXPSTABSET()`   |
+--------------------------+-----------------------------------+------------------------------+
