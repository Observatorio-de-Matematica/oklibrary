// Oliver Kullmann, 12.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Encoding the LS-MOLS-conditions, for Gecode

  The following Gecode constraints are enough for encoding all
  LS-MOLS-conditions parsed in Conditions.hpp:
  - distinct(IntVarArray X) : the all-different constraint for the array X.
  - element(IntVarArray X, IntVar y, IntVar z) : z is the y-th element of the
      array X, i.e. X_y = z.
  - rel(IntVar x, IntRelType irt, int val) : if irt is IRT_EQ, then x=val;
      if irt is IRT_NQ, then x!=val.

  These constraints can be used to encode special types of Latin squares:
  1. rls       : N distinct conditions - one for each row.
  2. cls       : N distinct conditions - one for each column.
  3. ls        : 2*N distinct conditions - one for each row and column.
  4. diag      : 1 distinct condition for the main diagonal.
  5. antidiag  : 1 distinct condition for the main antidiagonal.
  6. uni       : N rel-eq conditions on the main diagonal's elements.
  7. antiuni   : N rel-eq conditions on the main antidiagonal's elements.
  8. idem      : N rel-eq conditions on the main diagonal's elements.
  9. antiidem  : N rel-eq conditions on the main antidiagonal's elements.
  10. rred     : N rel-eq conditions on the first row's elements.
  11. orred    : N rel-eq conditions on the first row's elements.
  12. cred     : N rel-eq conditions on the first columns's elements.
  13. ocred    : N rel-eq conditions on the first columns's elements.
  14. symm     : XXX
  15. antisymm : XXX

*/

#ifndef ENCODING_HqEmYk6s0p
#define ENCODING_HqEmYk6s0p

#include <vector>
#include <istream>

#include <gecode/int.hh>

#include "Conditions.hpp"
#include "PartialSquares.hpp"

namespace Encoding {

  namespace GC = Gecode;
  namespace CD = Conditions;
  namespace PS = PartialSquares;

  typedef CD::size_t size_t;


  struct EncCond {

    const CD::AConditions& ac;
    const size_t N;
    const size_t num_vars;
    const GC::IntPropLevel pl;
    typedef GC::Space* SP;
    const SP s;

    static bool valid(const size_t N) noexcept {
      return N >= 2 and N <= 10000;
    }

    EncCond(const CD::AConditions& ac,
            const size_t N, const GC::IntPropLevel pl,
            const SP s) noexcept
      : ac(ac), N(N), num_vars(ac.num_squares() * N^2), pl(pl), s(s) {
      assert(valid(N)); assert(s);
    }

    typedef std::vector<PS::PSquare> list_psquares_t;
    const list_psquares_t& psquares() const noexcept { return psquares_; }
    size_t read_psquares(std::istream& in) {
      // XXX
    }


    typedef GC::IntVarArray VA;
    typedef std::vector<GC::IntVar> vv_t;

    void post_unary(const VA& va) const {

      // Compilation-tests:
      vv_t vv;
      vv.push_back(va[0]);
      GC::distinct(*s, vv, pl); // just a compilation-test
      // XXX
    }
    void post_equations(const VA& va) const {
      // XXX
    }
    void post_prod_equations(const VA& va) const {
      // XXX
    }
    void post_psquares(const VA& va) const {
      // XXX
    }


    // the VA is default-constructed in the calling-class, and updated
    // by the result obtained from post:
    VA post() const {
      VA va(*s, num_vars, 0, N-1);
      post_unary(va);
      post_equations(va);
      post_prod_equations(va);
      post_psquares(va);
      return va;
    }


    typedef CD::Square Square;
    size_t index(const Square s, size_t i, size_t j) const noexcept {
      assert(ac.valid(s));
      assert(i < N and j < N);
      // XXX
    }

  private :

    list_psquares_t psquares_;

  };

}

#endif
