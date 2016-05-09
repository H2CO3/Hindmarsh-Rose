//
// ODESolver.hh
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 08/05/2016
//

#ifndef ODESOLVER_HH
#define ODESOLVER_HH

#include <array>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>


template<unsigned Dim>
class ODESolver {

	std::array<double, Dim> x;
	const double h0;

	gsl_odeiv2_system ode;
	gsl_odeiv2_step *stepper;
	gsl_odeiv2_control *control;
	gsl_odeiv2_evolve *evolver;

public:
	ODESolver(
		double abs_tol = 1e-4,
		double rel_tol = 1e-4
	) :  h0(abs_tol * rel_tol), ode {} {

		// Set up ODE solver
		// ode.function = fn;
		ode.jacobian = nullptr;
		ode.dimension = Dim;
		// ode.params = this;

		// Runge-Kutta-Fehlberg method of order 4-5
		stepper = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rkf45, Dim);
		control = gsl_odeiv2_control_standard_new(abs_tol, rel_tol, 1, 1);
		evolver = gsl_odeiv2_evolve_alloc(Dim);
	}

	~ODESolver() {
		gsl_odeiv2_evolve_free(evolver);
		gsl_odeiv2_control_free(control);
		gsl_odeiv2_step_free(stepper);
	}

	template<typename Fn>
	void solve(
		std::array<double, Dim> x0,
		double t_max,
		double dt_max,
		void *data,
		std::vector<double> *t_out,
		std::vector<std::array<double, Dim>> *x_out,
		Fn fn
	) {
		int status;
		double t = 0.0;
		double h = h0;

		x = x0;
		ode.function = fn;
		ode.params = data;

		const std::size_t approx_max_steps = 2 * t_max / dt_max;
		t_out->clear();
		t_out->reserve(approx_max_steps);
		x_out->clear();
		x_out->reserve(approx_max_steps);

		// Solve the differential equation, firing the callback with
		// a maximal time interval of dt_max.
		// If the solver adjusts the step size (h) so that it exceeds
		// dt_max, we interpolate through the minimal number of steps
		// using a cubic spline.
		bool first = true;

		// values of the independent and dependent variables and derivatives in the previous step
		double prev_t;
		std::array<double, Dim> prev_x;
		std::array<double, Dim> prev_dxdt;

		do {
			t_out->push_back(t);
			x_out->push_back(x);

			status = gsl_odeiv2_evolve_apply(
				evolver,
				control,
				stepper,
				&ode,
				&t,
				t_max,
				&h,
				&x[0]
			);

			// obtain derivatives (needed by the spline)
			std::array<double, Dim> dxdt;
			fn(t, &x[0], &dxdt[0], data);

			if (first) {
				first = false;
				prev_t = t;
				prev_x = x;
				prev_dxdt = dxdt;
				continue;
			}

			// if solver's step size exceeds maximal step size, interpolate a spline
			double dt = t - prev_t;

			if (dt > dt_max) {
				auto spline = [=](double p) {
					std::array<double, Dim> ipol_x;

					for (std::size_t i = 0; i < ipol_x.size(); i++) {
						double dx = x[i] - prev_x[i];
						double a = prev_dxdt[i] * dt - dx;
						double b = -dxdt[i] * dt + dx;

						ipol_x[i] = (1 - p) * prev_x[i] + p * x[i] + p * (1 - p) * (a * (1 - p) + b * p);
					}

					return ipol_x;
				};

				std::size_t n_steps = std::ceil(dt / dt_max);

				// start from 1 in order not to repeat the first point
				for (std::size_t i = 1; i < n_steps; i++) {
					double p = double(i) / n_steps;
					t_out->push_back(prev_t + p * dt);
					x_out->push_back(spline(p));
				}
			}

			prev_t = t;
			prev_x = x;
			prev_dxdt = dxdt;
		} while (status == GSL_SUCCESS && t < t_max);

		t_out->push_back(t);
		x_out->push_back(x);
	}
};

#endif
