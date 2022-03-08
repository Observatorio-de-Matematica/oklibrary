// Oliver Kullmann, 27.2.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

#include <iostream>
#include <vector>
#include <algorithm>
#include <vector>
#include <sstream>

#include <cassert>

#include <ProgramOptions/Environment.hpp>

#include <Transformers/Generators/Random/Numbers.hpp>
#include <Transformers/Generators/Random/Distributions.hpp>

#include "DimacsTools.hpp"

#include "ConflictGraphs.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.1.3",
        "28.2.2022",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/Bicliques/TestConflictGraphs.cpp",
        "GPL v3"};

  using namespace ConflictGraphs;
  using namespace DimacsTools;

  template <class X>
  constexpr bool eqp(const X& lhs, const X& rhs) noexcept {
    return lhs == rhs;
  }
}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv))
  return 0;

  {typedef std::vector<int> v_t;
   assert(empty_intersection(v_t{}, v_t{}));
   assert(empty_intersection(v_t{1}, v_t{}));
   assert(empty_intersection(v_t{}, v_t{1}));
   assert(not empty_intersection(v_t{1}, v_t{1}));
   assert(not empty_intersection(v_t{1}, v_t{0,1,2}));
   assert(not empty_intersection(v_t{0,1,2}, v_t{1}));
   const auto empty_int = [](const v_t& v1, const v_t& v2){
     v_t res;
     std::ranges::set_intersection(v1, v2, std::back_inserter(res));
     return res.empty();
   };
   RandGen::RandGen_t g({77});
   const auto size_dist = RandGen::UniformRange(g, 10);
   const auto val1_dist = RandGen::UniformRange(g, 10);
   const auto val2_dist = RandGen::UniformRange(g, 10, 5);
   for (unsigned i = 0; i < 10000; ++i) {
     const auto N1 = size_dist(); const auto N2 = size_dist();
     v_t v1; v1.reserve(N1); v_t v2; v2.reserve(N2);
     for (unsigned j = 0; j < N1; ++j) v1.push_back(val1_dist());
     for (unsigned j = 0; j < N2; ++j) v2.push_back(val2_dist());
     assert(empty_int(v1, v2) == empty_intersection(v1, v2));
   }
  }


  {Clause C;
   assert(eqp(ewcompl(C), {}));
   C.push_back(Lit{1,1});
   assert(eqp(ewcompl(C), {Lit{1,-1}}));
  }
  {ClauseList F;
   assert(eqp(ewcompl(F), {}));
   F.push_back({Lit{5,1}, Lit{3,-1}, Lit{0,2}});
   assert(eqp(ewcompl(F), {{Lit{5,-1}, Lit{3,1}, Lit{0,-2}}}));
  }

  {std::stringstream ss;
   ss.str("p cnf 0 0\n");
   const auto F = read_strict_Dimacs(ss);
   const auto G = conflictgraph_bydef(F);
   assert(G.n() == 0);
   assert(G.m() == 0);
   assert(not G.with_names());
  }

  {std::stringstream ss;
   ss.str("p cnf 0 3\n0\n0\n0\n");
   const auto F = read_strict_Dimacs(ss);
   const auto G = conflictgraph_bydef(F);
   assert(G.n() == 3);
   assert(G.m() == 0);
   assert(not G.with_names());
  }

  {std::stringstream ss;
   ss.str("p cnf 3 3\n1 0\n1 0\n-1 0\n");
   const auto F = read_strict_Dimacs(ss);
   const auto G = conflictgraph_bydef(F);
   assert(G.n() == 3);
   assert(G.m() == 2);
   assert(not G.with_names());
   assert(eqp(G.graph(), {{2},{2},{0,1}}));
  }

}