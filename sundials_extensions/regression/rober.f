c-----------------------------------------------------------------------
c-----------------------------------------------------------------------
c
c     This file is part of the Test Set for IVP solvers
c     http://www.dm.uniba.it/~testset/
c
c        Problem ROBERTSON
c        ODE of dimension 3
c
c     DISCLAIMER: see
c     http://www.dm.uniba.it/~testset/disclaimer.php
c
c     The most recent version of this source file can be found at
c     http://www.dm.uniba.it/~testset/src/problems/rober.f
c
c     This is revision
c     $Id: rober.F,v 1.2 2006/10/02 10:29:14 testset Exp $
c
c
c     NOTE: Daniel R. Reynolds, January 2013
c     This file has been modified from the original to create 
c     operator-split versions of the feval and jeval routines, named 
c     feval_e, feval_i and jeval_i corresponding to the explicit and 
c     implicit components of the right-hand side function and the 
c     Jacobian of the implicit RHS function.  
c     
c     In addition, the prob() routine has been modified to explicitly
c     specify the length of the input arrays to remove all possibility
c     of segmentation faults.
c
c     All other routines remain untouched.
c
c-----------------------------------------------------------------------
      integer function pidate()
      pidate = 20060828
      return
      end
c-----------------------------------------------------------------------
      subroutine prob(fullnm,problm,type,
     +                neqn,ndisc,t,
     +                numjac,mljac,mujac,
     +                nummas,mlmas,mumas,
     +                ind)
      character fullnm*17, problm*5, type*3
      integer neqn,ndisc,mljac,mujac,mlmas,mumas,ind(1)
      double precision t(0:1)
      logical numjac, nummas

      fullnm = 'Problem ROBERTSON'
      problm = 'rober'
      type   = 'ODE'
      neqn   = 3
      ndisc  = 0
      t(0)   = 0d0
      t(1)   = 1d11
      numjac = .false.
      mljac  = neqn
      mujac  = neqn

      return
      end
c-----------------------------------------------------------------------
      subroutine init(neqn,t,y,yprime,consis)
      integer neqn
      double precision t,y(neqn),yprime(neqn)
      logical consis

      y(1) = 1d0
      y(2) = 0d0
      y(3) = 0d0

      return
      end
c-----------------------------------------------------------------------
      subroutine settolerances(neqn,rtol,atol,tolvec)
      integer neqn 
      logical tolvec
      double precision rtol(neqn), atol(neqn)
       
      tolvec  = .false.
      

      return
      end
c-----------------------------------------------------------------------
      subroutine setoutput(neqn,solref,printsolout,
     +                    nindsol,indsol)

      logical solref, printsolout
      integer neqn, nindsol
      integer indsol(neqn)

c the reference solution is available
      solref = .true.  

c output file is required
      printsolout = .true.

c default values if printsolout is .true.
      nindsol = neqn
c only nindsol component of indsol are referenced
      do i=1,nindsol
          indsol(i) = i
      end do  

  

      return
      end

c-----------------------------------------------------------------------
      subroutine feval(neqn,t,y,yprime,f,ierr,rpar,ipar)
      integer neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),f(neqn),rpar(*)

      f(1) = -0.04d0*y(1) + 1d4*y(2)*y(3)
      f(2) = 0.04d0*y(1) - 1d4*y(2)*y(3) - 3d7*y(2)**2
      f(3) = 3d7*y(2)**2
      
      return
      end
c-----------------------------------------------------------------------
      subroutine feval_e(neqn,t,y,yprime,f,ierr,rpar,ipar)
      integer neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),f(neqn),rpar(*)

      f(1) = -0.04d0*y(1)
      f(2) = 0.04d0*y(1)
      f(3) = 0.d0
      
      return
      end
c-----------------------------------------------------------------------
      subroutine feval_i(neqn,t,y,yprime,f,ierr,rpar,ipar)
      integer neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),f(neqn),rpar(*)

      f(1) = 1d4*y(2)*y(3)
      f(2) = -1d4*y(2)*y(3) - 3d7*y(2)**2
      f(3) = 3d7*y(2)**2
      
      return
      end
c-----------------------------------------------------------------------
      subroutine jeval(ldim,neqn,t,y,yprime,dfdy,ierr,rpar,ipar)
      integer ldim,neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),dfdy(ldim,neqn),rpar(*)

      integer i,j

      do 20 j=1,neqn
         do 10 i=1,neqn
            dfdy(i,j)=0d0
   10    continue
   20 continue

      dfdy(1,1) = -0.04d0
      dfdy(1,2) = 1d4*y(3)
      dfdy(1,3) = 1d4*y(2)
      dfdy(2,1) = 0.04d0
      dfdy(2,2) = -1d4*y(3)-6d7*y(2)
      dfdy(2,3) = -1d4*y(2)
      dfdy(3,2) = 6d7*y(2)

      return
      end
c-----------------------------------------------------------------------
      subroutine jeval_i(ldim,neqn,t,y,yprime,dfdy,ierr,rpar,ipar)
      integer ldim,neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),dfdy(ldim,neqn),rpar(*)
      integer i,j

      do 20 j=1,neqn
         do 10 i=1,neqn
            dfdy(i,j)=0.d0
   10    continue
   20 continue

      dfdy(1,2) = 1d4*y(3)
      dfdy(1,3) = 1d4*y(2)
      dfdy(2,2) = -1d4*y(3)-6d7*y(2)
      dfdy(2,3) = -1d4*y(2)
      dfdy(3,2) = 6d7*y(2)

      return
      end
c-----------------------------------------------------------------------
      subroutine meval(ldim,neqn,t,y,yprime,dfddy,ierr,rpar,ipar)
      integer ldim,neqn,ierr,ipar(*)
      double precision t,y(neqn),yprime(neqn),dfddy(ldim,neqn),rpar(*)
c
c     dummy subroutine
c
      return
      end
c-----------------------------------------------------------------------
      subroutine solut(neqn,t,y)
      integer neqn
      double precision t,y(neqn)
c
c computed using double precision RADAU on an 
c     Alphaserver DS20E, with a 667 MHz EV67 processor.
c          
c          uround = 1.01d-19
c          rtol = atol = h0 = 1.1d-18
c
c
         
       y(1) =  0.2083340149701255D-07           
       y(2) =  0.8333360770334713D-13           
       y(3) =  0.9999999791665050D+00            

      return
      end