function dy = EStab_ionization_2D(t, y)
% usage: dy = EStab_ionization_2D(t, y)
%
%------------------------------------------------------------
% Programmer(s):  Daniel R. Reynolds @ SMU
%------------------------------------------------------------
% Copyright (c) 2013, Southern Methodist University.
% All rights reserved.
% For details, see the LICENSE file.
%------------------------------------------------------------

% access model parameters
global Pdata;
a = Pdata.a; 
b = Pdata.b; 
d1 = Pdata.d1;
d2 = Pdata.d2;

% determine spatial size, etc
n = length(y);
m = floor(sqrt(n/3));
m2 = m*m;
if (m2*3 ~= n)
   error('incompatible inputs, need 3 species defined over a square 2D domain')
end
m2 = m*m;
dx = 1/(m-1);

% extract current states
E  = y(1:m2);
e  = y(m2+1:2*m2);
HI = y(2*m2+1:3*m2);

% initialize RHS terms
dE  = zeros(m2,1);
de  = zeros(m2,1);
dHI = zeros(m2,1);

% enforce stationary boundary conditions
%du(1) = 0;  du(m) = 0;  dv(1) = 0;  dv(m) = 0;

% diffusion components
%du(2:m-1) = d1/dx/dx*(u(3:m)+u(1:m-2)-2*u(2:m-1));
%dv(2:m-1) = d2/dx/dx*(v(3:m)+v(1:m-2)-2*v(2:m-1));

% reaction components
%du(2:m-1) = du(2:m-1) + a - (b+1)*u(2:m-1) + u(2:m-1).*u(2:m-1).*v(2:m-1);
%dv(2:m-1) = dv(2:m-1) + b*u(2:m-1) - u(2:m-1).*u(2:m-1).*v(2:m-1);

% combine together into output
dy = [dE; de; dHI];

% end function
