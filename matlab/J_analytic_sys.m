function J = J_analytic_sys(t, y)
% usage: J = J_analytic_sys(t, y)
%
% Daniel R. Reynolds
% Department of Mathematics
% Southern Methodist University
% August 2012
% All Rights Reserved

% model parameters
global Pdata;
lam = Pdata.lam;

% form the ODE Jacobian
V = [1 -1 1; -1 2 1; 0 -1 2];
Vinv = 0.25*[5 1 -3; 2 2 -2; 1 1 1];
D = [-0.5 0 0; 0 -0.1 0; 0 0 lam];
J = V*D*Vinv;

% end function