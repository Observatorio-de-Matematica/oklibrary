// Oleg Zaikin, 23.5.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

Test test cases for solvers.

BUG:

TODOS:

*/

#include <cassert>

#include <ProgramOptions/Environment.hpp>

#include "Cases.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.0.10",
        "24.5.2022",
        __FILE__,
        "Oleg Zaikin and Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Solvers/Gecode/MOLS/TestCases.cpp",
        "GPL v3"};

  namespace CS = Cases;

}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv))
  return 0;

  {CS::Square<2> A;
   assert(A.enc().N == 2);
   assert(A.enc().num_vars == 2 * 2);
   assert(A.space() != nullptr);
  }
  {CS::Square<2> A("A\n0 *\n* *\n");
   assert(A.enc().N == 2);
   assert(A.enc().num_vars == 2 * 2);
   assert(A.space() != nullptr);
  }

  {CS::Square<3> A;
   assert(A.enc().N == 3);
   assert(A.enc().num_vars == 3 * 3);
   assert(A.space() != nullptr);
  }
  {CS::Square<3> A("A\n0 * *\n* * *\n* * *\n");
   assert(A.enc().N == 3);
   assert(A.enc().num_vars == 3 * 3);
   assert(A.space() != nullptr);
  }

  {CS::TrivialLatinSquare A(2);
   assert(A.enc().N == 2);
   assert(A.enc().num_vars == 4);
   assert(A.space() != nullptr);
  }
  {CS::TrivialLatinSquare A(2, "A\n0 *\n* *\n");
   assert(A.enc().N == 2);
   assert(A.enc().num_vars == 4);
   assert(A.space() != nullptr);
  }

  {CS::TrivialLatinSquare A(3);
   assert(A.enc().N == 3);
   assert(A.enc().num_vars == 3 * 3);
   assert(A.space() != nullptr);
  }
  {CS::TrivialLatinSquare A(3, "A\n0 * *\n* * *\n* * *\n");
   assert(A.enc().N == 3);
   assert(A.enc().num_vars == 3 * 3);
   assert(A.space() != nullptr);
  }

}
