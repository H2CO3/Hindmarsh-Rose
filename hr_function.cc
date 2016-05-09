//
// hr_function.cc
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 08/05/2016
//

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include "hr_function.hh"

int hindmarsh_rose(double t, const double *RESTRICT xs, double *RESTRICT dxdts, void *RESTRICT data)
{
	auto &params = *static_cast<HRParams *>(data);

	const double
		a = params['a'],
		b = params['b'],
		c = params['c'],
		d = params['d'],
		r = params['r'],
		s = params['s'],
		X = params['X'],
		I = params['I'];

	double x = xs[0];
	double y = xs[1];
	double z = xs[2];

	double dxdt = x * x * (b - a * x) + y - z + I;
	double dydt = c - d * x * x - y;
	double dzdt = r * (s * (x - X) - z);

	dxdts[0] = dxdt;
	dxdts[1] = dydt;
	dxdts[2] = dzdt;

	return GSL_SUCCESS;
}
