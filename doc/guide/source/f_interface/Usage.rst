:tocdepth: 3

.. _FInterface.Usage:

Usage of the FARKODE interface module
==========================================

The usage of FARKODE requires calls to five or more interface
functions, depending on the method options selected, and one or more
user-supplied routines which define the problem to be solved.  These 
function calls and user routines are summarized separately below.
Some details are omitted, and the user is referred to the description
of the corresponding ARKode functions for complete information on
the arguments of any given user-callable interface routine.  The usage
of FARKODE for rootfinding and with preconditioner modules is
described in later subsections.

Steps marked [**S**] in the instructions below apply to the serial
NVECTOR implementation (NVECTOR_SERIAL) only, while those
marked with a [**P**] apply to NVECTOR_PARALLEL.


.. _FInterface.RHS:

Right-hand side specification
--------------------------------------

The user must in all cases supply the following Fortran routines:

.. f:subroutine:: FARKIFUN(T, Y, YDOT, IPAR, RPAR, IER)
   
   Sets the YDOT array to :math:`f_I(t,y)`, the
   implicit portion of the right-hand side of the ODE system, as
   function of the independent variable T :math:`=t` and the array
   of dependent state variables Y :math:`=y`.
      
   **Arguments:**
      * T (``realtype``, input) -- current value of the independent variable
      * Y (``realtype``, input) -- array containing state variables  
      * YDOT (``realtype``, output) -- array containing state derivatives 
      * IPAR (``long int``, input) -- array containing integer user
        data that was passed to :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing real user
        data that was passed to :f:func:`FARKMALLOC()` 
      * IER (``int``, output) -- return flag (0 success, >0
        recoverable error, <0 unrecoverable error)  
   

.. f:subroutine:: FARKEFUN(T, Y, YDOT, IPAR, RPAR, IER)
   
   Sets the YDOT array to :math:`f_E(t,y)`, the
   explicit portion of the right-hand side of the ODE system, as
   function of the independent variable T :math:`=t` and the array
   of dependent state variables Y :math:`=y`.
      
   **Arguments:**
      * T (``realtype``, input) -- current value of the independent variable
      * Y (``realtype``, input) -- array containing state variables  
      * YDOT (``realtype``, output) -- array containing state derivatives 
      * IPAR (``long int``, input) -- array containing integer user
        data that was passed to :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing real user
        data that was passed to :f:func:`FARKMALLOC()` 
      * IER (``int``, output) -- return flag (0 success, >0
        recoverable error, <0 unrecoverable error)  

For purely explicit problems, although the routine
:f:func:`FARKIFUN()` must exist, it will never be called, and may
remain empty.  Similarly, for purely implicit problems,
:f:func:`FARKEFUN()` will never be called and must exist but need not
do anything.


.. _FInterface.NVector:

NVECTOR module initialization
--------------------------------------

[**S**] To initialize the serial NVECTOR module, the user must
call the function FNVINITS with the argument KEY = 4.

.. f:subroutine:: FNVINITS(KEY, NEQ, IER)
   
   Initializes the Fortran interface to the serial
   NVECTOR module.
      
   **Arguments:** 
      * KEY (``int``, input) -- integer flag denoting which solver is to be
        used (1 is CVODE, 2 is IDA, 3 is KINSOL and 4 is
        ARKode) 
      * NEQ (``long int``, input) -- size of the ODE system 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


[**P**] To initialize the parallel NVECTOR module, the user must
call the function FNVINITP with the argument KEY = 4.

.. f:subroutine:: FNVINITP(COMM, KEY, NLOCAL, NGLOBAL, IER)
   
   Initializes the Fortran interface to the parallel
   NVECTOR module.
      
   **Arguments:** 
      * COMM (``int``, input) -- the MPI communicator 
      * KEY (``int``, input) -- integer flag denoting which solver is to be
        used (1 is CVODE, 2 is IDA, 3 is KINSOL and 4 is
        ARKode) 
      * NLOCAL (``long int``, input) -- local size of vectors on this processor 
      * NGLOBAL (``long int``, input) -- the size of the ODE system, and the global size of
        vectors (the sum of all values of NLOCAL) 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 
      
   **Notes:** If the header file ``sundials_config.h`` defines
   ``SUNDIALS_MPI_COMM_F2C`` to be 1 (meaning the MPI implementation 
   used to build SUNDIALS includes the ``MPI_Comm_f2c`` function),
   then COMM can be any valid MPI communicator.  Otherwise,
   ``MPI_COMM_WORLD`` will be used, so the user can just pass an
   integer value as a placeholder.



.. _FInterface.Problem:

Problem specification
--------------------------------------

To set various problem and solution parameters and allocate internal
memory, the user must call FARKMALLOC.


.. f:subroutine:: FARKMALLOC(T0, Y0, IMEX, IATOL, RTOL, ATOL, IOUT, ROUT, IPAR, RPAR, IER)
   
   Initializes the Fortran interface to the ARKode
   solver, providing interfaces to the C routines :c:func:`ARKodeCreate()`,
   :c:func:`ARKodeSetUserData()`, and :c:func:`ARKodeInit()`, as well
   as one of :c:func:`ARKodeSStolerances()` or
   :c:func:`ARKodeSVtolerances()`.
      
   **Arguments:** 
      * T0 (``realtype``, input) -- initial value of :math:`t` 
      * Y0 (``realtype``, input) -- array of initial conditions 
      * IMEX (``int``, input) -- flag denoting basic integration method:
         * 0 = implicit, 
         * 1 = explicit, 
         * 2 = imex.
      * IATOL (``int``, input) -- type for absolute tolerance input ATOL:
         * 1 = scalar, 
         * 2 = array,
         * 3 = user-supplied function; the user must subsequently call
           :f:func:`FARKEWTSET()` and supply a routine :f:func:`FARKEWT()` to
           compute the error weight vector.
      * RTOL (``realtype``, input) -- scalar relative tolerance 
      * ATOL (``realtype``,
        input) -- scalar or array absolute tolerance 
      * IOUT (``long
        int``, input/output) -- array of length 22 for integer optional outputs 
      * ROUT (``realtype``, input/output) -- array of length 6 for real optional outputs
      * IPAR (``long int``, input/output) -- array of user integer data, which will be passed
        unmodified to all user-provided routines 
      * RPAR (``realtype``, input/output) -- array with user real data, which will be passed
        unmodified to all user-provided routines 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 
      
   **Notes:** Modifications to the user data arrays IPAR and RPAR
   inside a user-provided routine will be propagated to all
   subsequent calls to such routines. The optional outputs
   associated with the main ARKode integrator are listed in
   :ref:`FInterface.IOUTTable` and :ref:`FInterface.ROUTTable`, in
   the section :ref:`FInterface.OptionalOutputs`. 


As an alternative to providing tolerances in the call to
:f:func:`FARKMALLOC()`, the user may provide a routine to compute the
error weights used in the WRMS norm evaluations.  If supplied, it must
have the following form:

.. f:subroutine:: FARKEWT(Y, EWT, IPAR, RPAR, IER)
   
   It must set the positive components of the error weight
   vector EWT for the calculation of the WRMS norm of Y.
      
   **Arguments:** 
      * Y (``realtype``, input) -- array containing state variables  
      * EWT (``realtype``, output) -- array containing the error weight vector  
      * IPAR (``long int``, input) -- array containing the integer user data that was passed
        to :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing the real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 

   
If the FARKEWT routine is provided, then, following the call to
:f:func:`FARKMALLOC()`, the user must call the function FARKEWTSET.

.. f:subroutine:: FARKEWTSET(FLAG, IER)
 
   Informs FARKODE to use the user-supplied
   :f:func:`FARKEWT()` function.
      
   **Arguments:** 
      * FLAG (``int``, input) -- flag, use "1" to denoting to use FARKEWT.
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


.. _FInterface.OptionalInputs:

Setting optional inputs
--------------------------------------

To set desired optional inputs, the user can call the routines
:f:func:`FARKSETIIN()` and :f:func:`FARKSETRIN()`, as described below.


.. f:subroutine:: FARKSETIIN(KEY, IVAL, IER)
   
   Specification routine to pass optional integer inputs
   to the :f:func:`FARKODE()` solver.
      
   **Arguments:** 
      * KEY (quoted string, input) -- which optional input
        is set (see :ref:`FInterface.IINOptionTable`).
      * IVAL (``long int``, input) -- the integer input value to be used 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


.. _FInterface.IINOptionTable:

Table: Keys for setting FARKODE integer optional inputs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. cssclass:: table-bordered

=================  =========================================
Key                ARKode routine
=================  =========================================
ORDER              :c:func:`ARKodeSetOrder()`
DENSE_ORDER        :c:func:`ARKodeSetDenseOrder()`
LINEAR             :c:func:`ARKodeSetLinear()`
NONLINEAR          :c:func:`ARKodeSetNonlinear()`
EXPLICIT           :c:func:`ARKodeSetExplicit()`
IMPLICIT           :c:func:`ARKodeSetImplicit()`
IMEX               :c:func:`ARKodeSetImEx()`
IRK_TABLE_NUM      :c:func:`ARKodeSetIRKTableNum()`
ERK_TABLE_NUM      :c:func:`ARKodeSetERKTableNum()`
ARK_TABLE_NUM `*`  :c:func:`ARKodeSetARKTableNum()`      
MAX_NSTEPS         :c:func:`ARKodeSetMaxNumSteps()`
HNIL_WARNS         :c:func:`ARKodeSetMaxHnilWarns()`
PREDICT_METHOD     :c:func:`ARKodeSetPredictorMethod()`
MAX_ERRFAIL        :c:func:`ARKodeSetMaxErrTestFails()`
MAX_NITERS         :c:func:`ARKodeSetMaxNonlinIters()`
MAX_CONVFAIL       :c:func:`ARKodeSetMaxConvFails()`
ADAPT_METHOD       :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_SMALL_NEF    :c:func:`ARKodeSetAdaptivityConstants()`
LSETUP_MSBP        :c:func:`ARKodeSetLSetupConstants()`
=================  =========================================

`*` When setting ARK_TABLE_NUM, pass in IVAL as an array of
length 2, specifying the IRK table number first, then the ERK table
number. 
      

.. f:subroutine:: FARKSETRIN(KEY, RVAL, IER)
  
   Specification routine to pass optional real inputs
   to the :f:func:`FARKODE()` solver.
      
   **Arguments:** 
      * KEY (quoted string, input) -- which optional input
        is set (see :ref:`FInterface.RINOptionTable`).
      * RVAL (``realtype``, input) -- the real input value to be used 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


.. _FInterface.RINOptionTable:

Table: Keys for setting FARKODE real optional inputs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. cssclass:: table-bordered

============  =========================================
Key           ARKode routine
============  =========================================
INIT_STEP     :c:func:`ARKodeSetInitStep()`
MAX_STEP      :c:func:`ARKodeSetMaxStep()`
MIN_STEP      :c:func:`ARKodeSetMinStep()`
STOP_TIME     :c:func:`ARKodeSetStopTime()`
NLCONV_COEF   :c:func:`ARKodeSetNonlinConvCoef()`
ADAPT_CFL     :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_SAFETY  :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_BIAS    :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_GROWTH  :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_LB      :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_UB      :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_K1      :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_K2      :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_K3      :c:func:`ARKodeSetAdaptivityMethod()`
ADAPT_ETAMX1  :c:func:`ARKodeSetAdaptivityConstants()`
ADAPT_ETAMXF  :c:func:`ARKodeSetAdaptivityConstants()`
ADAPT_ETACF   :c:func:`ARKodeSetAdaptivityConstants()`
NEWT_CRDOWN   :c:func:`ARKodeSetNewtonConstants()`
NEWT_RDIV     :c:func:`ARKodeSetNewtonConstants()`
LSETUP_DGMAX  :c:func:`ARKodeSetLSetupConstants()`
============  =========================================


Alternatively, if a user wishes to reset all of the options to their
default values, they may call the routine FARKSETDEFAULTS.

.. f:subroutine:: FARKSETDEFAULTS(IER)
   
   Specification routine to reset all FARKODE optional
   inputs to their default values.
      
   **Arguments:** 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 
   


FARKODE supplies additional routines to specify optional advanced
inputs to the :c:func:`ARKode()` solver.  These are summarized below,
and the user is referred to their C routine counterparts for more
complete information. 



.. f:subroutine:: FARKSETERKTABLE(S, Q, P, C, A, B, BEMBED, IER)
   
   Interface to the routine :c:func:`ARKodeSetERKTable()`.
      
   **Arguments:** 
      * S (``int``, input) -- number of stages in the table 
      * Q (``int``, input) -- global order of accuracy of the method 
      * P (``int``, input) -- global order of accuracy of the embedding 
      * C (``realtype``, input) -- array of length S containing the stage times
      * A (``realtype``, input) -- array of length S*S containing the ERK coefficients
        (stored in row-major, "C", order) 
      * B (``realtype``, input) -- array of length S containing the solution coefficients 
      * BEMBED (``realtype``, input) -- array of length S containing the embedding
        coefficients 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


.. f:subroutine:: FARKSETIRKTABLE(S, Q, P, C, A, B, BEMBED, IER)
   
   Interface to the routine :c:func:`ARKodeSetIRKTable()`.
      
   **Arguments:** 
      * S (``int``, input) -- number of stages in the table 
      * Q (``int``, input) -- global order of accuracy of the method 
      * P (``int``, input) -- global order of accuracy of the embedding 
      * C (``realtype``, input) -- array of length S containing the stage times
      * A (``realtype``, input) -- array of length S*S containing the IRK coefficients
        (stored in row-major, "C", order) 
      * B (``realtype``, input) -- array of length S containing the solution coefficients 
      * BEMBED (``realtype``, input) -- array of length S containing the embedding
        coefficients 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 

.. f:subroutine:: FARKSETARKTABLES(S, Q, P, C, AI, AE, B, BEMBED, IER)
   
   Interface to the routine :c:func:`ARKodeSetARKTables()`.
   
   **Arguments:** 
      * S (``int``, input) -- number of stages in the table 
      * Q (``int``, input) -- global order of accuracy of the method 
      * P (``int``, input) -- global order of accuracy of the embedding 
      * C (``realtype``, input) -- array of length S containing the stage times
      * AI (``realtype``, input) -- array of length S*S containing the IRK coefficients
        (stored in row-major, "C", order) 
      * AE (``realtype``, input) -- array of length S*S containing the ERK coefficients
        (stored in row-major, "C", order) 
      * B (``realtype``, input) -- array of length S containing the solution coefficients 
      * BEMBED (``realtype``, input) -- array of length S containing the embedding
        coefficients 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 
   

Lastly, the user may provide functions to aid/replace those within
ARKode for handling adaptive error control and explicit stability.
The former of these is designed for advanced users who wish to
investigate custom step adaptivity approaches as opposed to using any
of those built-in to ARKode.  In ARKode's C/C++ interface, this would be
provided by a function of type :c:func:`ARKAdaptFn()`; in the Fortran
interface this is provided through the user-supplied function:

.. f:subroutine:: FARKADAPT(Y, T, H1, H2, H3, E1, E2, E3, Q, P, HNEW, IPAR, RPAR, IER)
   
   It must set the new step size HNEW based on the three previous
   steps (H1, H2, H3) and the three previous error estimates (E1, E2, E3).
      
   **Arguments:** 
      * Y (``realtype``, input) -- array containing state variables  
      * T (``realtype``, input) -- current value of the independent variable
      * H1 (``realtype``, input) -- current step size
      * H2 (``realtype``, input) -- previous step size
      * H3 (``realtype``, input) -- previous-previous step size
      * E1 (``realtype``, input) -- estimated temporal error in current step
      * E2 (``realtype``, input) -- estimated temporal error in previous step
      * E3 (``realtype``, input) -- estimated temporal error in previous-previous step
      * Q (``int``, input) -- global order of accuracy for RK method
      * P (``int``, input) -- global order of accuracy for RK embedding
      * HNEW (``realtype``, output) -- array containing the error weight vector  
      * IPAR (``long int``, input) -- array containing the integer
	user data that was passed to :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing the real user
	data that was passed to :f:func:`FARKMALLOC()` 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 


This routine is enabled by a call to the activation routine:

.. f:subroutine:: FARKADAPTSET(FLAG, IER)
   
   Informs FARKODE to use the user-supplied
   :f:func:`FARKADAPT()` function.
      
   **Arguments:** 
      * FLAG (``int``, input) -- flag, use "1" to denoting to use
	:f:func:`FARKADAPT()`, or use "0" to denote a return to the
        default adaptivity strategy.
      * IER (``int``, output) -- return flag (0 success, :math:`\ne
	0` failure) 

   Note: The call to :f:func:`FARKADAPTSET()` must occur *after* the call
   to :f:func:`FARKMALLOC()`.

Similarly, if either an explicit or mixed implicit-explicit
integration method is to be employed, the user may specify a function
to provide the maximum explicitly-stable step for their problem.
Again, in the C/C++ interface this would be a function of type
:c:func:`ARKExpStabFn()`, while in ARKode's Fortran interface this
must be given through the user-supplied function:

.. f:subroutine:: FARKEXPSTAB(Y, T, HSTAB, IPAR, RPAR, IER)
   
   It must set the maximum explicitly-stable step size, HSTAB, based
   on the current solution, Y.
      
   **Arguments:** 
      * Y (``realtype``, input) -- array containing state variables  
      * T (``realtype``, input) -- current value of the independent variable
      * HSTAB (``realtype``, output) -- maximum explicitly-stable step size
      * IPAR (``long int``, input) -- array containing the integer user data that was passed
        to :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing the real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * IER (``int``, output) -- return flag (0 success, :math:`\ne 0` failure) 
 
This routine is enabled by a call to the activation routine:

.. f:subroutine:: FARKEXPSTABSET(FLAG, IER)
   
   Informs FARKODE to use the user-supplied :f:func:`FARKEXPSTAB()` function.
      
   **Arguments:** 
      * FLAG (``int``, input) -- flag, use "1" to denoting to use
	:f:func:`FARKEXPSTAB()`, or use "0" to denote a return to the 
        default error-based stability strategy.
      * IER (``int``, output) -- return flag (0 success, :math:`\ne
	0` failure) 

   Note: The call to :f:func:`FARKEXPSTABSET()` must occur *after* the call
   to :f:func:`FARKMALLOC()`.


   
.. _FInterface.LinearSolver:

Linear solver specification
---------------------------------

In the case of using either an implicit or ImEx method, the solution
of each Runge-Kutta stage may involve the solution of linear systems
related to the Jacobian :math:`J = \frac{\partial f_I}{\partial y}` of
the implicit portion of the ODE system. ARKode presently includes
seven choices for the treatment of these systems, and the user of
FARKODE must call a routine with a specific name to make the
desired choice. 


[**S**] Dense treatment of the linear system
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To use the direct dense linear solver based on the internal ARKode
implementation, the user must call the FARKDENSE routine.


.. f:subroutine:: FARKDENSE(NEQ, IER)
   
   Interfaces with the :c:func:`ARKDense()` function to
   specify use of the dense direct linear solver.
      
   **Arguments:** 
      * NEQ (``long int``, input) -- size of the ODE system 
      * IER (``int``, output) -- return flag (0 if success, -1 if a memory allocation
        error occurred, -2 for an illegal input) 


Alteratively, to use the LAPACK-based direct dense linear solver, a
user must call the similar FARKLAPACKDENSE routine.

.. f:subroutine:: FARKLAPACKDENSE(NEQ, IER)
   
   Interfaces with the :c:func:`ARKLapackDense()` function
   to specify use of the LAPACK the dense direct linear solver.
      
   **Arguments:** 
      * NEQ (``int``, input) -- size of the ODE system 
      * IER (``int``, output) -- return flag (0 if success, -1 if a memory allocation
        error occurred, -2 for an illegal input) 

As an option when using either of these dense linear solvers, the user
may supply a routine that computes a dense approximation of the system
Jacobian :math:`J = \frac{\partial f_I}{\partial y}`. If supplied, it
must have the following form:


.. f:subroutine:: FARKDJAC(NEQ, T, Y, FY, DJAC, H, IPAR, RPAR, WK1, WK2, WK3, IER)
   
   Interface to provide a user-supplied dense Jacobian
   approximation function (of type :c:func:`ARKDenseJacFn()`), to be
   used by the :f:func:`FARKDENSE()` solver.
      
   **Arguments:** 
      * NEQ (``long int``, input) -- size of the ODE system 
      * T (``realtype``, input) -- current value of the independent variable 
      * Y (``realtype``, input) -- array containing values of the dependent state variables 
      * FY (``realtype``, input) -- array containing values of the dependent state derivatives 
      * DJAC (``realtype`` of size (NEQ,NEQ), output) -- 2D array containing the Jacobian entries 
      * H (``realtype``, input) -- current step size 
      * IPAR (``long int``, input) -- array containing integer user data that was passed to
        :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * WK1, WK2, WK3  (``realtype``, input) -- array containing temporary workspace
        of same size as Y 
      * IER (``int``, output) -- return flag (0 if success, >0 if a recoverable error
        occurred, <0 if an unrecoverable error occurred) 
      
   **Notes:** Typically this routine will use only NEQ, T, Y, and
   DJAC. It must compute the Jacobian and store it column-wise in DJAC. 
  
   
If the above routine uses difference quotient approximations, it may
need to use the error weight array EWT and current stepsize H
in the calculation of suitable increments. The array EWT can be
obtained by calling :f:func:`FARKGETERRWEIGHTS()` using one of the work
arrays as temporary storage for EWT. It may also need the unit
roundoff, which can be obtained as the optional output ROUT(6),
passed from the calling program to this routine using either RPAR
or a common block. 

If the :f:func:`FARKDJAC()` routine is provided, then, following the
call to :f:func:`FARKDENSE()` or :f:func:`FARKLAPACKDENSE()`, the user
must call the routine FARKDENSESETJAC. 


.. f:subroutine:: FARKDENSESETJAC(FLAG, IER)
   
   Interface to the :c:func:`ARKDenseSetJacFn()` function,
   specifying to use the user-supplied routine :f:func:`FARKDJAC()` for
   the Jacobian approximation.
      
   **Arguments:** 
      * FLAG (``int``, input) -- any nonzero value specifies to use :f:func:`FARKDJAC()` 
      * IER (``int``, output) -- return flag (0 if success, :math:`\ne 0` if an error
        occurred) 
   

   


[**S**] Band treatment of the linear system
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To use the direct band linear solver based on the internal ARKode
implementation, the user must call the FARKBAND routine.


.. f:subroutine:: FARKBAND(NEQ, MU, ML, IER)
   
   Interfaces with the :c:func:`ARKBand()` function to
   specify use of the dense banded linear solver.
      
   **Arguments:** 
      * NEQ (``long int``, input) -- size of the ODE system 
      * MU (``long int``, input) -- upper half-bandwidth 
      * ML (``long int``, input) -- lower half-bandwidth 
      * IER (``int``, output) -- return flag (0 if success, -1 if a memory allocation
        error occurred, -2 for an illegal input) 

Alteratively, to use the LAPACK-based direct banded linear solver, a
user must call the similar FARKLAPACKBAND routine.


.. f:subroutine:: FARKLAPACKBAND(NEQ, MU, ML, IER)
   
   Interfaces with the :c:func:`ARKLapackBand()` function
   to specify use of the dense banded linear solver.
      
   **Arguments:** 
      * NEQ (``int``, input) -- size of the ODE system 
      * MU (``int``, input) -- upper half-bandwidth 
      * ML (``int``, input) -- lower half-bandwidth 
      * IER (``int``, output) -- return flag (0 if success, -1 if a memory allocation
        error occurred, -2 for an illegal input) 
   

   
As an option when using either of these banded linear solvers, the user
may supply a routine that computes a banded approximation of the
linear system Jacobian :math:`J = \frac{\partial f_I}{\partial y}`. If
supplied, it must have the following form:

.. f:subroutine:: FARKBJAC(NEQ, MU, ML, MDIM, T, Y, FY, BJAC, H, IPAR, RPAR, WK1, WK2, WK3, IER)
   
   Interface to provide a user-supplied band Jacobian
   approximation function (of type :c:func:`ARKBandJacFn()`), to be
   used by the :f:func:`FARKBAND()` solver.
     
   **Arguments:** 
      * NEQ (``long int``, input) -- size of the ODE system 
      * MU   (``long int``, input) -- upper half-bandwidth 
      * ML   (``long int``, input) -- lower half-bandwidth 
      * MDIM (``long int``, input) -- leading dimension of BJAC array 
      * T    (``realtype``, input) -- current value of the independent variable 
      * Y    (``realtype``, input) -- array containing dependent state variables 
      * FY   (``realtype``, input) -- array containing dependent state derivatives 
      * BJAC (``realtype`` of size
        (MDIM,NEQ), output) -- 2D array containing the Jacobian entries 
      * H    (``realtype``, input) -- current step size 
      * IPAR (``long int``, input) -- array containing integer user data that was passed to
        :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * WK1, WK2, WK3  (``realtype``, input) -- array containing temporary workspace
        of same size as Y 
      * IER (``int``, output) -- return flag (0 if success, >0 if a recoverable error
        occurred, <0 if an unrecoverable error occurred) 
      
   **Notes:**
   Typically this routine will use only NEQ, MU, ML, T, Y, and
   BJAC. It must load the MDIM by N array BJAC with the Jacobian
   matrix at the current :math:`(t,y)` in band form.  Store in
   BJAC(k,j) the Jacobian element :math:`J_{i,j}` with :math:`k = i
   - j + MU + 1` (or :math:`k = 1, \ldots ML+MU+1`) and :math:`j =
   1, \ldots, N`. 


If the above routine uses difference quotient approximations, it may
need to use the error weight array EWT and current stepsize H
in the calculation of suitable increments. The array EWT can be
obtained by calling :f:func:`FARKGETERRWEIGHTS()` using one of the work
arrays as temporary storage for EWT. It may also need the unit
roundoff, which can be obtained as the optional output ROUT(6),
passed from the calling program to this routine using either RPAR
or a common block. 

If the :f:func:`FARKBJAC()` routine is provided, then, following the
call to either :f:func:`FARKBAND()` or :f:func:`FARKLAPACKBAND()`, the
user must call the routine FARKBANDSETJAC. 


.. f:subroutine:: FARKBANDSETJAC(FLAG, IER)
   
   Interface to the :c:func:`ARKBandSetJacFn()` function,
   specifying to use the user-supplied routine :f:func:`FARKBJAC()` for
   the Jacobian approximation.
      
   **Arguments:** 
      * FLAG (``int``, input) -- any nonzero value specifies to use
        :f:func:`FARKBJAC()`  
      * IER (``int``, output) -- return flag (0 if success, :math:`\ne 0` if an error
        occurred) 




[**S**][**P**] SPGMR treatment of the linear systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the Scaled Preconditioned GMRES solution of the linear systems,
the user must call the FARKSPGMR routine.


.. f:subroutine:: FARKSPGMR(IPRETYPE, IGSTYPE, MAXL, DELT, IER)
   
   Interfaces with the :c:func:`ARKSpgmr()` and
   ARKSpilsSet* routines to specify use of the SPGMR iterative
   linear solver.
      
   **Arguments:** 
      * IPRETYPE (``int``, input) -- preconditioner type : 
         * 0 = none 
         * 1 = left only
         * 2 = right only
         * 3 = both sides
      * IGSTYPE (``int``, input) -- Gram-schmidt process type : 
         * 1 = modified G-S
         * 2 = classical G-S
      * MAXL (``int``; input) -- maximum Krylov subspace dimension (0 for default) .
      * DELT (``realtype``, input) -- linear convergence tolerance factor (0.0 for default) .
      * IER (``int``, output) -- return flag (0 if success, -1 if a memory allocation
        error occurred, -2 for an illegal input) 


For descriptions of the optional user-supplied routines for use with
:f:func:`FARKSPGMR()` see the section :ref:`FInterface.SpilsUserSupplied`.



[**S**][**P**] SPBCG treatment of the linear systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the Scaled Preconditioned Bi-CGStab solution of the linear systems,
the user must call the FARKSPBCG routine.


.. f:subroutine:: FARKSPBCG(IPRETYPE, MAXL, DELT, IER)
   
   Interfaces with the :c:func:`ARKSpbcg()` and
   ARKSpilsSet* routines to specify use of the SPBCG iterative
   linear solver.
      
   **Arguments:**  The arguments are the same as those with the
   same names for :f:func:`FARKSPGMR()`. 


For descriptions of the optional user-supplied routines for use with
:f:func:`FARKSPBCG()` see the section :ref:`FInterface.SpilsUserSupplied`.





[**S**][**P**] SPTFQMR treatment of the linear systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the Scaled Preconditioned TFQMR solution of the linear systems,
the user must call the FARKSPTFQMR routine.


.. f:subroutine:: FARKSPTFQMR(IPRETYPE, MAXL, DELT, IER)
   
   Interfaces with the :c:func:`ARKSptfqmr()` and
   ARKSpilsSet* routines to specify use of the SPTFQMR iterative
   linear solver.
      
   **Arguments:**  The arguments are the same as those with the same names
   for :f:func:`FARKSPGMR()`.


For descriptions of the optional user-supplied routines for use with
:f:func:`FARKSPTFQMR()` see the next section.



[**S**][**P**] PCG treatment of the linear systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the Preconditioned Conjugate Gradient solution of symmetric linear
systems, the user must call the FARKPCG routine.


.. f:subroutine:: FARKPCG(IPRETYPE, MAXL, DELT, IER)
 
   Interfaces with the :c:func:`ARKPcg()` and
   ARKSpilsSet* routines to specify use of the PCG iterative
   linear solver.
      
   **Arguments:**  The arguments are the same as those with the
   same names for :f:func:`FARKSPGMR()`. 


For descriptions of the optional user-supplied routines for use with
:f:func:`FARKPCG()` see the section :ref:`FInterface.SpilsUserSupplied`.





.. _FInterface.SpilsUserSupplied:

[**S**][**P**] User-supplied routines for SPGMR/SPBCG/SPTFQMR/PCG
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With treatment of the linear systems by any of the Krylov iterative
solvers, there are three optional user-supplied routines --
:f:func:`FARKJTIMES()`, :f:func:`FARKPSET()` and :f:func:`FARKPSOL()`.
The specifications of these functions are given below.

As an option when using the SPGMR, SPBCG, SPTFQMR or PCG linear
solvers, the user may supply a routine that computes the product of
the system Jacobian :math:`J = \frac{\partial f_I}{\partial y}` and a
given vector :math:`v`.  If supplied, it must have the following form:


.. f:subroutine:: FARKJTIMES(V, FJV, T, Y, FY, H, IPAR, RPAR, WORK, IER)
   
   Interface to provide a user-supplied
   Jacobian-times-vector product approximation function (of type
   :c:func:`ARKSpilsJacTimesVecFn()`), to be used by one of the Krylov
   iterative linear solvers.
      
   **Arguments:** 
      * V (``realtype``, input) -- array containing the vector to multiply
      * FJV  (``realtype``, output) -- array containing resulting product vector
      * T    (``realtype``, input) -- current value of the independent variable
      * Y    (``realtype``, input) -- array containing dependent state variables
      * FY   (``realtype``, input) -- array containing dependent state derivatives
      * H    (``realtype``, input) -- current step size 
      * IPAR (``long int``, input) -- array containing integer user data that was passed to
        :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input) -- array containing real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * WORK (``realtype``, input) -- array containing temporary workspace of same size as
        Y   
      * IER  (``int``, output) -- return flag  (0 if success, :math:`\ne 0` if an error)
         
   **Notes:**
   Typically this routine will use only NEQ, T, Y, V, and FJV.  It
   must compute the product vector :math:`Jv`, where :math:`v` is
   given in V, and the product is stored in FJV. 
   

If this routine has been supplied by the user, then, following the
call to :f:func:`FARKSPGMR()`, :f:func:`FARKSPBCG()`,
:f:func:`FARKSPTFQMR()` or :f:func:`FARKPCG()`, the user must call the
routine FARKSPILSSETJAC with FLAG :math:`\ne 0` to specify use of the
user-supplied Jacobian-times-vector function.


.. f:subroutine:: FARKSPILSSETJAC(FLAG, IER)
   
   Interface to the function :c:func:`ARKSpilsSetJacTimesVecFn()` to specify use of the
   user-supplied Jacobian-times-vector function :f:func:`FARKJTIMES()`.
      
   **Arguments:** 
      * FLAG (``int``, input) -- flag denoting to use FARKJTIMES routine 
      * IER  (``int``, output) -- return flag  (0 if success, :math:`\ne 0` if an error)


If preconditioning is to be performed during the Krylov solver
(i.e. the solver was set up with IPRETYPE :math:`\ne 0`), then the
user must also call the routine FARKSPILSSETPREC with FLAG
:math:`\ne 0`. 


.. f:subroutine:: FARKSPILSSETPREC(FLAG, IER)
   
   Interface to the function :c:func:`ARKSpilsSetPreconditioner()` to specify use of the
   user-supplied preconditioner setup and solve functions,
   :f:func:`FARKPSET()` and :f:func:`FARKPSOL()`, respectively.
      
   **Arguments:** 
      * FLAG (``int``, input) -- flag denoting use of user-supplied
        preconditioning routines  
      * IER  (``int``, output) -- return flag  (0 if success, :math:`\ne 0` if an error)
         

In addition, the user must provide the following two routines to
implement the preconditioner setup and solve functions to be used
within the solve.


.. f:subroutine:: FARKPSET(T,Y,FY,JOK,JCUR,GAMMA,H,IPAR,RPAR,V1,V2,V3,IER)
   
   User-supplied preconditioner setup routine (of type
   :c:func:`ARKSpilsPrecSetupFn()`). 
      
   **Arguments:** 
      * T (``realtype``, input) -- current value of the independent variable
      * Y (``realtype``, input) -- current dependent state variable array 
      * FY (``realtype``, input) -- current dependent state variable derivative array 
      * JOK (``int``, input) -- flag indicating whether Jacobian-related data needs to be 
        recomputed:

         * 0 = recompute, 
         * 1 = reuse with the current value of GAMMA.

      * JCUR (``realtype``, output) -- return flag to denote if Jacobian data was recomputed
        (1=yes, 0=no)  
      * GAMMA (``realtype``, input) -- Jacobian scaling factor 
      * H (``realtype``, input) -- current step size 
      * IPAR (``long int``, input/output) -- array containing integer user data that was passed to
        :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input/output) -- array containing real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * V1, V2, V3 (``realtype``, input) -- arrays containing temporary workspace of
        same size as Y 
      * IER  (``int``, output) -- return flag  (0 if success, >0 if a recoverable
        failure, <0 if a non-recoverable failure) 
      
   **Notes:**
   This routine must set up the preconditioner P to be used in the
   subsequent call to :f:func:`FARKPSOL()`.  The preconditioner (or
   the product of the left and right preconditioners if using both)
   should be an approximation to the matrix  :math:`M - \gamma J`,
   where :math:`M` is the system mass matrix, :math:`\gamma` is the
   input GAMMA, and :math:`J = \frac{\partial f_I}{\partial y}`. 
   
   
.. f:subroutine:: FARKPSOL(T,Y,FY,R,Z,GAMMA,DELTA,LR,IPAR,RPAR,VT,IER)
   
   User-supplied preconditioner solve routine (of type
   :c:func:`ARKSpilsPrecSolveFn()`). 
      
   **Arguments:** 
      * T (``realtype``, input) -- current value of the independent variable
      * Y (``realtype``, input) -- current dependent state variable array 
      * FY (``realtype``, input) -- current dependent state variable derivative array 
      * R (``realtype``, input) -- right-hand side array 
      * Z (``realtype``, output) -- solution array 
      * GAMMA (``realtype``, input) -- Jacobian scaling factor 
      * DELTA (``realtype``, input) -- desired residual tolerance 
      * LR (``int``, input) -- flag denoting to solve the right or left preconditioner
        system:

         * 1 = left preconditioner
         * 2 = right preconditioner

      * IPAR (``long int``, input/output) -- array containing integer user data that was passed to
        :f:func:`FARKMALLOC()` 
      * RPAR (``realtype``, input/output) -- array containing real user data that was passed to
        :f:func:`FARKMALLOC()` 
      * VT (``realtype``, input) -- array containing temporary workspace of same size as Y  
      * IER  (``int``, output) -- return flag  (0 if success, >0 if a recoverable
        failure, <0 if a non-recoverable failure) 
      
   **Notes:**
   Typically this routine will use only NEQ, T, Y, GAMMA, R, LR,
   and Z.  It must solve the preconditioner linear system :math:`Pz
   = r`.  The preconditioner (or the product of the left and right
   preconditioners if both are nontrivial) should be an
   approximation to the matrix  :math:`M - \gamma J`, where
   :math:`M` is the system mass matrix, :math:`\gamma` is the input
   GAMMA, and :math:`J = \frac{\partial f_I}{\partial y}`. 


Notes:

(a) If the user's :f:func:`FARKJTIMES()` or :f:func:`FARKPSET()` routine
    uses difference quotient approximations, it may need to use the
    error weight array EWT, the current stepsize H, and/or the
    unit roundoff, in the calculation of suitable increments. Also, If
    :f:func:`FARKPSOL()` uses an iterative method in its solution, the
    residual vector :math:`\rho = r - Pz` of the system should be made
    less than :math:`\delta =` DELTA in the weighted l2 norm, i.e.
    
    .. math::
       \left(\sum_i \left(\rho_i * EWT_i\right)^2 \right)^{1/2} < \delta.

(b) If needed in :f:func:`FARKJTIMES()`, :f:func:`FARKPSOL()`, or
    :f:func:`FARKPSET()`, the error weight array EWT can be
    obtained by calling :f:func:`FARKGETERRWEIGHTS()` using one of the
    work arrays as temporary storage for EWT. 

(c) If needed in :f:func:`FARKJTIMES()`, :f:func:`FARKPSOL()`, or
    :f:func:`FARKPSET()`, the unit roundoff can be obtained as the
    optional output ROUT(6) (available after the call to
    :f:func:`FARKMALLOC()`) and can be passed using either the RPAR
    user data array or a common block. 




.. _FInterface.Solution:

Problem solution
-----------------------

Carrying out the integration is accomplished by making calls to
:f:func:`FARKODE()`.


.. f:subroutine:: FARKODE(TOUT, T, Y, ITASK, IER)
   
   Fortran interface to the C routine :c:func:`ARKode()`
   for performing the solve, along with many of the ARK*Get*
   routines for reporting on solver statistics.
      
   **Arguments:** 
      * TOUT (``realtype``, input) -- next value of :math:`t` at which a solution is
        desired 
      * T (``realtype``, output) -- current value of independent variable reached by the solver
      * Y (``realtype``, output) -- array containing dependent state variables on output
      * ITASK (``int``, input) -- task indicator :
         * 1 = normal mode (overshoot TOUT and interpolate)
         * 2 = one-step mode (return after each internal step taken)
         * 3 = normal `tstop` mode (like 1, but integration never
           proceeds past TSTOP, which must be specified through a
           preceding call to :f:func:`FARKSETRIN()` using the key
           STOP_TIME)
         * 4 = one step `tstop` (like 2, but integration never goes past
           TSTOP) 
      * IER (int, output) -- completion flag : 
	 * 0 = success, 
	 * 1 = tstop return, 
	 * 2 = root return, 
	 * values -1 ... -10 are failure modes (see :c:func:`ARKode()` and
           :ref:`Constants`).
      
   **Notes:**
   The current values of the optional outputs are immediately
   available in IOUT and ROUT upon return from this function (see
   :ref:`FInterface.IOUTTable` and :ref:`FInterface.ROUTTable`). 
   


.. _FInterface.AdditionalOutput:

Additional solution output
---------------------------------------

After a successful return from :f:func:`FARKODE()`, the routine
:f:func:`FARKDKY()` may be used to obtain a derivative of the solution,
of order up to 3, at any :math:`t` within the last step taken. 


.. f:subroutine:: FARKDKY(T, K, DKY, IER)
   
   Fortran interface to the C routine :f:func:`ARKDKY()`
   for interpolating output of the solution or its derivatives at any
   point within the last step taken.
      
   **Arguments:** 
      * T (``realtype``, input) -- time at which solution derivative is desired,
        within the interval :math:`[t_n-h,t_n]`, .
      * K (``int``, input) -- derivative order :math:`(0 \le k \le 3)` 
      * DKY (``realtype``, output) -- array containing the computed K-th derivative of
        :math:`y`  
      * IER (``int``, output) -- return flag (0 if success, <0 if an illegal argument)



.. _FInterface.ReInit:

Problem reinitialization
---------------------------------------

To re-initialize the ARKode solver for the solution of a new
problem of the same size as one already solved, the user must call
:f:func:`FARKREINIT()`. 


.. f:subroutine:: FARKREINIT(T0, Y0, IMEX, IATOL, RTOL, ATOL, IER)
   
   Re-initializes the Fortran interface to the ARKode solver.
      
   **Arguments:**  The arguments have the same names and meanings as those of
   :f:func:`FARKMALLOC()`.
      
   **Notes:**
   This routine performs no memory allocation, instead using the
   existing memory created by the previous :f:func:`FARKMALLOC()`
   call.  The call to specify the linear system solution method may
   or may not be needed. 


Following a call to :f:func:`FARKREINIT()`, a call to specify the
linear system solver must be made if the choice of linear solver is
being changed. Otherwise, a call to reinitialize the linear solver
last used may or may not be needed, depending on changes in the inputs
to it. 

In the case of the BAND solver, for any change in the
half-bandwidth parameters, call :f:func:`FARKBAND()` (or
:f:func:`FARKLAPACKBAND()`) again described above.

In the case of SPGMR, for a change of inputs other than MAXL,
the user may call the routine :f:func:`FARKSPGMRREINIT()` to
reinitialize SPGMR without reallocating its memory, as follows: 



.. f:subroutine:: FARKSPGMRREINIT(IPRETYPE, IGSTYPE, DELT, IER)
   
   Re-initializes the Fortran interface to the SPGMR
   linear solver.
      
   **Arguments:**  The arguments have the same names and meanings as those of
   :f:func:`FARKSPGMR()`.
   


However, if MAXL is being changed, then the user should call
:f:func:`FARKSPGMR()` instead.

In the case of SPBCG, for a change in any inputs, the user can
reinitialize SPBCG without reallocating its memory by calling
:f:func:`FARKSPBCGREINIT()`, as follows:


.. f:subroutine:: FARKSPBCGREINIT(IPRETYPE, MAXL, DELT, IER)
   
   Re-initializes the Fortran interface to the SPBCG
   linear solver.
      
   **Arguments:**  The arguments have the same names and meanings as those of
   :f:func:`FARKSPBCG()`.



In the case of SPTFQMR, for a change in any inputs, the user can
reinitialize SPTFQMR without reallocating its memory by calling
:f:func:`FARKSPTFQMRREINIT()`, as follows:


.. f:subroutine:: FARKSPTFQMRREINIT(IPRETYPE, MAXL, DELT, IER)
   
   Re-initializes the Fortran interface to the SPBTFQMR
   linear solver.
      
   **Arguments:**  The arguments have the same names and meanings as those of
   :f:func:`FARKSPTFQMR()`.


In the case of PCG, for a change in any inputs, the user can
reinitialize PCG without reallocating its memory by calling
:f:func:`FARKPCGREINIT()`, as follows:


.. f:subroutine:: FARKPCGREINIT(IPRETYPE, MAXL, DELT, IER)
   
   Re-initializes the Fortran interface to the PCG
   linear solver.
      
   **Arguments:**  The arguments have the same names and meanings as those of
   :f:func:`FARKPCG()`.





.. _FInterface.Deallocation:

Memory deallocation
---------------------------------------

To free the internal memory created by :f:func:`FARKMALLOC()`, the user
may call :f:func:`FARKFREE()`, as follows:


.. f:subroutine:: FARKFREE()
   
   Frees the internal memory created by :f:func:`FARKMALLOC()`.
      
   **Arguments:** None.