function h = h_estimate_PI(Y1, Y2, h_old, r_tol, a_tol, p)
% Usage: h = h_estimate_PI(Y1, Y2, h_old, r_tol, a_tol, p)
%
% Adaptive time step estimation routine, that attempts to guarantee a 
% local truncation error satisfying the bound
%    norm(local error,inf) < norm(rtol*y_i + atol_i,inf)
%
% Inputs:  Y1 -- first time-evolved solution (more accurate)
%          Y2 -- second time-evolved solution (same size as Y1)
%       h_old -- previous time step size (scalar)
%       r_tol -- desired relative tolerance (scalar)
%       a_tol -- desired absolute tolerance (vector, size of Y1)
%           p -- order of accuracy for predictor
%
% Output:   h -- new time step
%
% Daniel R. Reynolds
% Department of Mathematics
% Southern Methodist University
% November 2011
% All Rights Reserved

% $$$ % set an heuristic safety factor, growth factor
% $$$ safety = 0.75;
% $$$ dt_growth = 10;
% $$$ 
% $$$ % compute error estimate ratio, bound from below 
% $$$ Eratio = max(norm((Y1 - Y2)./(r_tol*Y1 + a_tol),inf), eps);
% $$$ % $$$ fprintf('h_estimate:  err = %g,  rerr = %g,  Eratio^p = %g,  p = %g',...
% $$$ % $$$     norm((Y1 - Y2),inf), ...
% $$$ % $$$     norm((Y1 - Y2)./(r_tol*Y1 + a_tol),inf), ...
% $$$ % $$$     Eratio^(-1/p), p);
% $$$ 
% $$$ % compute estimated optimal time step size
% $$$ h = h_old * Eratio^(-1/p);
% $$$ % $$$ fprintf(',  h_est = %g',h);
% $$$ 
% $$$ % enforce safety factor and growth factor
% $$$ h = safety*h;                  % in case estimate is off
% $$$ h = min(dt_growth*h_old, h);   % in case growth is too large
% $$$ % $$$ fprintf(',  h = %g\n',h);



% set variables
safety = 0.9;
dt_growth = 10;
alpha = 0.7/p;
beta  = 0.4/p;
persistent Eratio;
if isempty(Eratio)
   Eratio = 1;
end

% update old error estimates
Eratio_old = Eratio;

% compute new error estimate ratio, bound from below 
Eratio = max(norm((Y1 - Y2)./(r_tol*Y1 + a_tol),inf), eps);

% compute updated time step
h = safety * h_old * Eratio^(-alpha) * Eratio_old^(beta);

% enforce growth factor
h = min(dt_growth*h_old, h);


% end of function