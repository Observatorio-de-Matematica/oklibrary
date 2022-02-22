// Oliver Kullmann, 22.2.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

#include <iostream>
#include <sstream>

#include <cassert>

#include <ProgramOptions/Environment.hpp>

#include "Bicliques2SAT.hpp"
#include "Graphs.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.0.2",
        "22.2.2022",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/Bicliques/TestBicliques2SAT.cpp",
        "GPL v3"};

  using namespace Bicliques2SAT;
  using namespace Graphs;

  template <class X>
  constexpr bool eqp(const X& lhs, const X& rhs) noexcept {
    return lhs == rhs;
  }
}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv))
  return 0;

  {std::stringstream in;
   const std::string inp = "a b c d\nb c d\nc d";
   in << inp;
   const auto G = make_AdjVecUInt(in, GT::und);
   assert(G.n() == 4);
   assert(G.m() == 6);

   VarEncoding enc(G, 1);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc.left(i,0) == 1+i);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc.right(i,0) == 5+i);
   for (unsigned i = 0; i < 6; ++i)
     assert(enc.edge(i,0) == 9+i);
   assert(enc.V == 4);
   assert(enc.E == 6);
   assert(enc.B == 1);
   assert(enc.nb == 8);
   assert(enc.ne == 6);
   assert(enc.n == 14);

   VarEncoding enc2(G, 2);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc2.left(i,0) == 1+i);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc2.right(i,0) == 5+i);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc2.left(i,1) == 9+i);
   for (unsigned i = 0; i < 4; ++i)
     assert(enc2.right(i,1) == 13+i);
   for (unsigned i = 0; i < 6; ++i)
     assert(enc2.edge(i,0) == 17+i);
   for (unsigned i = 0; i < 6; ++i)
     assert(enc2.edge(i,1) == 23+i);
   assert(enc2.V == 4);
   assert(enc2.E == 6);
   assert(enc2.B == 2);
   assert(enc2.nb == 16);
   assert(enc2.ne == 12);
   assert(enc2.n == 28);
  }

}
