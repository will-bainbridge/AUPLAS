AUPLAS
======

* Author:	Will Bainbridge
* Date:		August, 2011
* GitHub:	<https://github.com/maninthemail/AUPLAS>

LICENSE
-------

AUPLAS is provided under the terms of the Eclipse Public License v1.0. See LICENSE or <http://www.eclipse.org/legal/epl-v10.html> for details.

DESCRIPTION
-----------

* AUPLAS will be a PDE solver, written in C, following on from IMMASHIN <https://github.com/maninthemail/IMMASHIN>. It is not yet functional.
* The equations to be solved are specified at runtime in an "accumulation equals divergences" format.
* The control volumes used for each variable/zone in the system are arbitrarily centred (staggered) on either volumes or faces in the mesh.
* Gradients are calculated using a Least-Squares approach. The stencils used and the orders of the interpolating polynomials are arbitrary and determined at runtime.
* Fluxes are calculated by directly integrating differentiated interpolating polynomials over the control volume faces rather than interpolating volume-centred gradients.
* Time integration is performed over an arbitrary number of sub-steps. Each divergence has implicit/explicit contributions specified ar runtime. The implicit conribution is calculated by newtonian iteration, requiring calculation and solution of the flux jacobian matrix system at each iterative step. Sparse matrix methods are employed.
* Whilst the solver has been written with generality in mind, to date the program has been applied predominantly to fluid dynamics problems.

TO DO
-----

* Have zone values read by solve function rather than preprocess so that they can be changed without preprocessing again
* Implement expressions for the zone values so that derived boundary conditions (such as total pressure) may be used
* Allow for specification of normal gradients on boundaries
* Implement upwinding and gradient/flux limiters

INSTALLATION
------------

Unpack the project

	tar -xf AUPLAS.tar.gz
	cd AUPLAS

Create a third party directory and download and unpack the required software into this directory

	mkdir thirdparty
	cd thirdparty
	wget ...
	tar -xf ...

Currently the program requires UMFPACK and ILUPACK to be installed here. BLAS and LAPACK are also necessary. See the makefile for the expected paths and versions.

Move back into the base directory and build the sources

	cd ..
	make

USAGE
-----

Move into one of the example directories

	cd cavity

Preprocess and solve the case and then postprocess the resulting data

	../preprocess cavity.navier.input
	../solve cavity.navier.input [optional-initial-datafile]
	../postprocess cavity.navier.input [datafile]

Gnuplot or Paraview can then be used to view the resulting files
