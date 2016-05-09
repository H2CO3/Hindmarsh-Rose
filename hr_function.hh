//
// hr_function.hh
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 08/05/2016
//

#ifndef HR_FUNCTION_HH
#define HR_FUNCTION_HH

#include <unordered_map>

#include "util.hh"


typedef std::unordered_map<char, double> HRParams;

int hindmarsh_rose(double t, const double *RESTRICT xs, double *RESTRICT dxdts, void *RESTRICT data);

#endif
