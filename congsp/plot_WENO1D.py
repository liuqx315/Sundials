#!/usr/bin/env python
# matplotlib-based plotting script for heat1D.c example
# Daniel R. Reynolds, reynolds@smu.edu

# imports
import sys
import pylab as plt
import numpy as np

# load mesh data file
mesh = np.loadtxt('WENO1D_mesh.txt', dtype=np.double)

# load solution data file
data = np.loadtxt('WENO1D.txt', dtype=np.double)

# determine number of time steps, mesh size
nt,nx = np.shape(data)

# determine maximum temperature
maxtemp = 1.1*data.max()

# generate plots of results
for tstep in range(nt):

    # set string constants for output plots, current time, mesh size
    pname = 'WENO1D.' + repr(tstep).zfill(3) + '.png'
    tstr  = repr(tstep)
    nxstr = repr(nx)

    # plot current solution and save to disk
    plt.figure(1)
    plt.plot(mesh,data[tstep,:])
    plt.xlabel('x')
    plt.ylabel('solution')
    plt.title('u(x) at output ' + tstr + ', mesh = ' + nxstr)
    plt.axis((0.0, 2.0, -maxtemp, maxtemp))
    plt.grid()
    plt.savefig(pname)
    plt.close()


##### end of script #####