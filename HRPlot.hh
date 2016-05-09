//
// HRPlot.hh
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 06/05/2016
//

#ifndef HRPLOT_HH
#define HRPLOT_HH

#include <gtkmm/drawingarea.h>

#include "hr_function.hh"


class HRPlot : public Gtk::DrawingArea {
private:
	HRParams params;
	unsigned enabled_functions;

	// These are instance variables for efficiency reasons
	// (so that they don't need to be reallocated each time
	// 'solve()' is called.)
	std::vector<double> t;
	std::vector<std::array<double, 3>> x;

	double ver_max;
	double ver_scale;
	double t_max;
	double px_per_time;

	// Returns the maximum of X, Y or Z values (index: 0 => X, 1 => Y, 2 => Z).
	double get_max_abs(unsigned index) const {
		auto abs_max = *std::max_element(
			x.begin(),
			x.end(),
			[=](auto x1, auto x2) { return std::abs(x1[index]) < std::abs(x2[index]); }
		);
		return std::abs(abs_max[index]);
	};

	// This only considers enabled (visible) functions.
	double get_ver_scale() const {
		std::array<double, 3> abs_max_3 { 0.0, 0.0, 0.0 };

		for (unsigned i = 0; i < 3; i++) {
			if (enabled_functions & (1 << i)) {
				abs_max_3[i] = get_max_abs(i);
			}
		}

		const double abs_max = *std::max_element(abs_max_3.begin(), abs_max_3.end());
		const double padding = 10;
		const double ver_scale = (ver_max - padding) / abs_max;

		return ver_scale;
	}

	void draw_background(const Cairo::RefPtr<Cairo::Context> &ctx) {
		ctx->set_source_rgb(0.0, 0.0, 0.0);
		ctx->set_line_width(2);
		ctx->rectangle(0, 0, get_allocated_width(), get_allocated_height());
		ctx->fill();
	}

	void draw_functions(const Cairo::RefPtr<Cairo::Context> &ctx) {
		ctx->set_line_width(1);

		const struct {
			double r;
			double g;
			double b;
		} colors[] = {
			{ 0.5, 0.2, 1.0 },
			{ 0.8, 0.2, 0.4 },
			{ 0.6, 0.9, 0.3 },
		};

		for (int i = 0; i < 3; i++) {
			if (not (enabled_functions & (1u << i))) {
				continue;
			}

			ctx->set_source_rgb(colors[i].r, colors[i].g, colors[i].b);
			ctx->move_to(t[0] * px_per_time, ver_max - x[0][i] * ver_scale);

			for (int j = 1; j < t.size(); j++) {
				ctx->line_to(t[j] * px_per_time, ver_max - x[j][i] * ver_scale);
			}

			ctx->stroke();
		}
	}

	void draw_axes(const Cairo::RefPtr<Cairo::Context> &ctx) {
		const int num_hor_divs = 10;
		const int num_ver_divs = 10;

		ctx->set_source_rgb(1.0, 1.0, 1.0);
		ctx->set_line_width(2);

		// Horizontal axis and labels
		ctx->move_to(0, ver_max);
		ctx->line_to(get_allocated_width(), ver_max);

		for (int i = 1; i < num_hor_divs; i++) {
			auto x_pos = get_allocated_width() / num_hor_divs * i;

			char label[32];
			std::snprintf(label, sizeof label, "%.0f", t_max / num_hor_divs * i);

			Cairo::TextExtents ext;
			ctx->get_text_extents(label, ext);

			ctx->move_to(x_pos - ext.width / 2, ver_max - 5 - ext.height);
			ctx->show_text(label);

			ctx->move_to(x_pos, ver_max - 5);
			ctx->line_to(x_pos, ver_max + 5);
		}

		// Vertical axis and labels
		ctx->move_to(0, 0);
		ctx->line_to(0, get_allocated_height());

		for (int i = 1; i < num_ver_divs; i++) {
			if (i == num_ver_divs / 2) {
				continue;
			}

			auto y_pos = get_allocated_height() / num_ver_divs * i;
			auto y_max = ver_max / ver_scale;

			char label[32];
			std::snprintf(label, sizeof label, "%.2f", 2 * y_max / num_ver_divs * (i - num_ver_divs / 2));

			Cairo::TextExtents ext;
			ctx->get_text_extents(label, ext);

			ctx->move_to(10, y_pos - (ext.height + ext.y_bearing) / 2);
			ctx->show_text(label);

			ctx->move_to(0, y_pos);
			ctx->line_to(5, y_pos);
		}

		ctx->stroke();
	}

public:
	enum {
		FN_X = 1 << 0,
		FN_Y = 1 << 1,
		FN_Z = 1 << 2
	};

	HRPlot() : Gtk::DrawingArea(), params{}, enabled_functions(FN_X | FN_Y | FN_Z) {}

	void set_params(const HRParams &p_params) {
		params = p_params;
		queue_draw();
	}

	void set_enabled_functions(unsigned p_enabled_functions) {
		enabled_functions = p_enabled_functions;
		queue_draw();
	}

protected:
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &ctx) {
		// Set initial conditions and drawing parameters
		const auto x0 = std::array<double, 3> { params['x'], params['y'], params['z'] };
		const double dt_max = 0.1;

		t_max = params['t']; // get_allocated_width();
		px_per_time = get_allocated_width() / t_max;
		ver_max = get_allocated_height() / 2;

		// Solve the DE, plot solution on the fly
		ODESolver<3> solver;

		solver.solve(
			x0,
			t_max,
			dt_max,
			&params,
			&t,
			&x,
			hindmarsh_rose
		);

		assert(t.size() == x.size() && "# of time and x coordinates does not match");

		// Set Y scale so that the maximal value can be fully drawn.
		// Leave a little padding too.
		// This only considers the enabled (visible) functions.
		ver_scale = get_ver_scale();

		// draw frame with black background
		draw_background(ctx);

		// draw functions
		draw_functions(ctx);

		// draw axes and divisions in white color
		draw_axes(ctx);

		return true;
	}
};

#endif
