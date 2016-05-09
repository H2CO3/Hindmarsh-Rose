//
// main.cc
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 05/05/2016
//

#include <vector>
#include <memory>
#include <unordered_map>

#include <cstdio>
#include <cmath>
#include <cassert>

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/scale.h>
#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/fixed.h>
#include <gtkmm/adjustment.h>

#include "ODESolver.hh"
#include "HRPlot.hh"


// Little helper class for storing parameter scales and their corresponding labels
class LabelAndScale {
	static Gtk::Scale make_scale(double lower, double upper, double default_val) {
		int prec = std::abs(upper - lower) < 0.1 ? 3 : 2;
		double step = std::pow(10, -prec);
		auto adj = Gtk::Adjustment::create(default_val, lower, upper, step, step);
		Gtk::Scale scale(adj, Gtk::ORIENTATION_HORIZONTAL);
		scale.set_digits(prec);
		scale.show();
		return scale;
	}

public:
	char name;
	Gtk::Scale scale;
	Gtk::Label label;

	LabelAndScale(char p_name, double minval, double maxval, double defval):
		name(p_name),
		scale(make_scale(minval, maxval, defval)),
		label(std::string("") + name)
		{}
};

int main(int argc, char *argv[])
{
	auto app = Gtk::Application::create(argc, argv, "org.h2co3.nemlin.hr");

	Gtk::Window window;
	window.set_title("Hindmarsh-Rose Neuron Model");
	window.set_default_size(1280, 768);

	Gtk::Fixed view;
	view.show();

	HRPlot plot;
	plot.show();
	plot.set_size_request(640, 540);

	view.put(plot, 500, 50);
	window.add(view);

	// b = 2.687, r = 0.01, s = 4, X = -1.3, I = 2.95
	auto params = std::unordered_map<char, double>{};

	std::vector<std::shared_ptr<LabelAndScale>> widgets {
		// Parameters
		{ std::make_shared<LabelAndScale>('a', -3.0, +3.0, +1.0) },
		{ std::make_shared<LabelAndScale>('b', +1.0, +5.0, +2.7) },
		{ std::make_shared<LabelAndScale>('c', -3.0, +3.0, +1.0) },
		{ std::make_shared<LabelAndScale>('d', -2.0, +9.0, +5.0) },
		{ std::make_shared<LabelAndScale>('r', 5e-4, 0.04, 1e-2) },
		{ std::make_shared<LabelAndScale>('s',  0.0, +8.0, +4.0) },
		{ std::make_shared<LabelAndScale>('X', -5.0, +2.0, -1.3) },
		{ std::make_shared<LabelAndScale>('I', -9.0, +9.0, +2.9) },

		// Initial conditions and integration time
		{ std::make_shared<LabelAndScale>('x', -9.0, +9.0,  0.0) },
		{ std::make_shared<LabelAndScale>('y', -9.0, +9.0,  0.0) },
		{ std::make_shared<LabelAndScale>('z', -9.0, +9.0,  0.0) },
		{ std::make_shared<LabelAndScale>('t', 20.0, +9e3,  9e2) },
	};

	// Draw scales that set parameters
	int y = 30;
	const int control_height = 45;

	for (auto &w : widgets) {
		char name = w->name;
		auto &label = w->label;
		auto &scale = w->scale;

		label.show();
		view.put(label, 20, y + 20);

		params[name] = scale.get_value();
		view.put(scale, 50, y);
		scale.show();
		scale.set_size_request(250, 15);

		scale.signal_value_changed().connect(
			// name must be captured by value!!!
			[&scale, &params, &plot, name]{
				params[name] = scale.get_value();
				plot.set_params(params);
			}
		);

		y += control_height;
	}

	// Draw checkboxes that enable the drawing of individual functions
	auto checkboxes = std::array<std::pair<unsigned, std::shared_ptr<Gtk::CheckButton>>, 3> {{
		{ HRPlot::FN_X, std::make_shared<Gtk::CheckButton>("x(t) (membrane potential)") },
		{ HRPlot::FN_Y, std::make_shared<Gtk::CheckButton>("y(t) (fast channels, spiking)") },
		{ HRPlot::FN_Z, std::make_shared<Gtk::CheckButton>("z(t) (slow channels, bursting)") },
	}};

	unsigned enabled_fns = 0;

	for (auto &cb : checkboxes) {
		unsigned fn_id = cb.first;
		auto &button = cb.second;

		view.put(*button, 15, y);
		button->show();
		button->set_size_request(50, 15);

		button->signal_toggled().connect([fn_id, &enabled_fns, &button, &plot] {
			if (button->get_active()) {
				enabled_fns |= fn_id;
			} else {
				enabled_fns &= ~fn_id;
			}

			plot.set_enabled_functions(enabled_fns);
		});

		y += control_height;
	}

	// Enable drawing only X (aka first function aka membrane potential) by default
	checkboxes[0].second->set_active(true);

	// Should the X-Y phase space be plotted?
	Gtk::CheckButton phase_space_checkbox("Draw X-Y Phase Space");
	phase_space_checkbox.show();
	phase_space_checkbox.set_size_request(50, 15);
	view.put(phase_space_checkbox, 15, y);

	phase_space_checkbox.signal_toggled().connect([&plot, &phase_space_checkbox] {
		plot.set_plot_phase_space(phase_space_checkbox.get_active());
	});

	y += control_height;

	plot.set_params(params);
	plot.set_enabled_functions(enabled_fns);

	return app->run(window);
}
