// Oleg Zaikin, 23.5.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  Test cases for lookahead-reduction and solvers.

  Namespace Cases, abbreviated "CS";

BUG:

TODOS:

0. Add cases needed for testing lookahead-reduction.
  - DONE Square.
  - DONE Trivial (empty) Latin square.
  - Reduced variations.
  - Row latin squares.

1. Providing the number of solutions when a formula is not known.
  - For a square, there is a simple formula for the number of solutions.
  - For a Latin square, there is no such formula.
  - For now no need to consider such complicated cases -- concentrating
    on simple variations, as in TestEncoding.cpp.

2. Statistics for lookahead-reduction.
  - Add for each case a function that returns ReductionStatistics.

*/

#ifndef CASES_tqVXGkU1YS
#define CASES_tqVXGkU1YS

#include <sstream>
#include <string>
#include <memory>

#include <cassert>

#include <gecode/int.hh>
#include <gecode/search.hh>

#include <Numerics/FloatingPoint.hpp>

#include "Conditions.hpp"
#include "Parsing.hpp"
#include "PartialSquares.hpp"
#include "Encoding.hpp"
#include "Constraints.hpp"
#include "Options.hpp"
#include "LookaheadReduction.hpp"

namespace Cases {

  namespace GC = Gecode;

  namespace CD = Conditions;
  namespace PG = Parsing;
  namespace FP = FloatingPoint;
  namespace PS = PartialSquares;
  namespace EC = Encoding;
  namespace CT = Constraints;
  namespace OP = Options;
  namespace LR = LookaheadReduction;

  using size_t = Conditions::size_t;


  class GenericMolsNB : public CT::GenericMols0 {
    struct Void : GC::Brancher {
      Void(const GC::Home home) : GC::Brancher(home) {}
      Void(GC::Space& home, Void& b) : GC::Brancher(home,b) {}
      GC::Brancher* copy(GC::Space& home) {return new (home) Void(home,*this);}
      bool status(const GC::Space& s) const noexcept {
        return not GcVariables::empty(static_cast<const GenericMols0&>(s).V);
      }
      GC::Choice* choice(GC::Space&) { assert(0); return nullptr; }
      GC::Choice* choice(const GC::Space&, GC::Archive&) {
        assert(0); return nullptr;
      }
      GC::ExecStatus commit(GC::Space&, const GC::Choice&, unsigned) {
        assert(0); return GC::ExecStatus(0);
      }
    };
  public :
    GenericMolsNB(const EC::EncCond& enc) : GenericMols0(enc) {
      new (*this) Void(*this);
    }
  };

  typedef std::unique_ptr<GenericMolsNB> space_ptr_t;


  EC::EncCond encoding(const std::string condstr, const std::string psstr,
    const size_t N) noexcept {
    std::istringstream in_cond(condstr);
    std::istringstream in_ps(psstr);
    const CD::AConditions ac = PG::ReadAC()(in_cond);
    const PS::PSquares ps(N, in_ps);
    const EC::EncCond enc(ac, ps);
    return enc;
  }


  struct Square {
    const size_t N;
    const EC::EncCond e;
    Square(const size_t N_, const std::string psstr = "") :
      N(N_), e(encoding("squares A\n", psstr, N)) {}
    //size_t solc() const noexcept { return FP::pow(N, N*N); }
    space_ptr_t space() const noexcept {
      space_ptr_t m(new GenericMolsNB(e));
      m->status();
      return m;
    };
    LR::ReductionStatistics laredstats(const OP::LAR lar) const noexcept {
      const space_ptr_t m = space();
      LR::ReductionStatistics s(m->V);
      s.inc_rounds();
      for (auto i=0; i < FP::pow(N, 3); ++i) s.inc_probes();
      if (pruning(lar)) s.maxprune(FP::pow(N, 3));
      return s;
    }
  };

  struct LS {
    const size_t N;
    const EC::EncCond e;
    LS(const size_t N_, const std::string psstr = "") :
      N(N_), e(encoding("squares A\nls A\n", psstr, N)) {}
    space_ptr_t space() const noexcept {
      space_ptr_t m(new GenericMolsNB(e));
      m->status();
      return m;
    };
  };

}

#endif
