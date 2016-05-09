//
// util.hh
// A visualisation of the Hindmarsh-Rose neuron model
//
// Created by Arpad Goretity
// on 06/05/2016
//

#ifndef UTIL_HH
#define UTIL_HH

// We really love aliasing-based optimizations in this project.
// Too bad C++ hasn't standardized 'restrict' yet.
#if defined(__GNUC__) || defined(__GNUG__)
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif

#endif
