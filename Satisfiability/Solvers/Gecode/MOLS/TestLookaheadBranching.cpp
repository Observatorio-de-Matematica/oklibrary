// Oleg Zaikin, 31.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

TODOS:

1. Urgently unit-tests are needed, for all components.
   Same for the reduction-components.

2. Urgently Makefile needs to be updated to include all dependencies.

*/

#include <iostream>
#include <sstream>

#include <cassert>

#include <ProgramOptions/Environment.hpp>

#include "LookaheadBranching.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.0.2",
        "27.4.2022",
        __FILE__,
        "Oleg Zaikin and Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Solvers/Gecode/MOLS/TestLookaheadBranching.cpp",
        "GPL v3"};

  namespace LAB = LookaheadBranching;
}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv))
  return 0;

}
