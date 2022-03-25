// Oliver Kullmann, 16.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  The Gecode-constraint-classes

*/

#ifndef CONSTRAINTS_hBa0Xe3nKA
#define CONSTRAINTS_hBa0Xe3nKA

#include <istream>

#include <gecode/int.hh>
#include <gecode/search.hh>

#include "../Lookahead.hpp"

#include "Conditions.hpp"
#include "Encoding.hpp"
#include "Parsing.hpp"
#include "PartialSquares.hpp"

namespace Constraints {

  namespace GC = Gecode;
  namespace LA = Lookahead;
  namespace CD = Conditions;
  namespace EC = Encoding;
  namespace PR = Parsing;
  namespace PS = PartialSquares;

  typedef EC::size_t size_t;


  // Version for testing:
  class GenericMols0 : public GC::Space {

    const size_t N;
    typedef GC::IntVarArray VarVec;
    VarVec V;

  public :

    GenericMols0(const CD::AConditions& ac, const PS::PSquares& ps)
      : N(ps.N) {
      assert(included(ps, ac));
      V = EC::EncCond(ac, ps, {}, this).post();
    }

  protected :
    GenericMols0(GenericMols0& gm) noexcept : N(gm.N), V(gm.V) {
      V.update(*this, gm.V);
    }
    // Pure virtual function inherited from GC::Space:
    GC::Space* copy() noexcept { return new GenericMols0(*this); }

  };

  GenericMols0* make_gm0(const size_t N,
                         std::istream& in_cond, std::istream& in_ps) {
    const auto ac = PR::ReadAC()(in_cond);
    // Remark: ac must be constructed first, due to the (global)
    // names of squares.
    return new GenericMols0(ac, PS::PSquares(N, in_ps));
  }

}

#endif
