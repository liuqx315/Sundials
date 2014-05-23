function dy = f_test(t, y)
% usage: dy = f_test(t, y)
%
%------------------------------------------------------------
% Programmer(s):  Daniel R. Reynolds @ SMU
%------------------------------------------------------------
% Copyright (c) 2013, Southern Methodist University.
% All rights reserved.
% For details, see the LICENSE file.
%------------------------------------------------------------

% model parameters
I1 = 0.5;
I2 = 2;
I3 = 3;

f = 0.0;
if ((t >= 3*pi) & (t <= 4*pi))
   f = 0.25*sin(t)*sin(t);
end

% form the ODE RHS [Euler's equations of motion for a rigid body]
dy = [(I2-I3)/I1*y(2)*y(3);
      (I3-I1)/I2*y(3)*y(1);
      (I1-I2)/I3*y(1)*y(2) + f];

% end function