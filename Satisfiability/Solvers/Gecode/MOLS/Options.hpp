// Oliver Kullmann, 26.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Options for Gecode

  Namespace Options, abbreviated "OP".

TODOS

0. Write documentation.

1. Provide the choice of a random branching-variable:
    - This needs a bit of design how to use our random-number-generator.

*/

#ifndef OPTIONS_JwU0BOX6Tw
#define OPTIONS_JwU0BOX6Tw

#include <array>
#include <ostream>
#include <exception>
#include <string>

#include <gecode/int.hh>

#include <ProgramOptions/Environment.hpp>

namespace Options {

  namespace GC = Gecode;

  // Run-Type:
  enum class RT {
    sat_solving = 0,
    sat_decision = 1,
    unique_solving = 2,
    unique_s_with_log = 3,
    unique_decision = 4,
    unique_d_with_log = 5,
    count_solutions = 6,
    count_with_log = 7,
    enumerate_solutions = 8,
    enumerate_with_log = 9
  };
  constexpr int RTsize = int(RT::enumerate_with_log) + 1;
  constexpr bool valid(const RT rt) noexcept {
    return int(rt) < RTsize;
  }
  constexpr bool count_only(const RT rt) noexcept {
    return rt == RT::count_solutions or rt == RT::count_with_log
      or rt == RT::enumerate_with_log;
  }
  constexpr bool with_file_output(const RT rt) noexcept {
    return rt == RT::sat_solving or rt == RT::unique_solving
      or rt == RT::enumerate_solutions;
  }
  constexpr bool with_solutions(const RT rt) noexcept {
    return rt == RT::sat_solving or
      rt == RT::unique_solving or rt == RT::unique_s_with_log or
      rt == RT::enumerate_solutions or rt == RT::enumerate_with_log;
  }
  constexpr bool with_log(const RT rt) noexcept {
    return rt == RT::unique_s_with_log or rt == RT::unique_d_with_log or
      rt == RT::count_with_log or rt == RT::enumerate_with_log;
  }
  constexpr bool test_sat(const RT rt) noexcept {
    return rt == RT::sat_solving or rt == RT::sat_decision;
  }
  constexpr bool test_unique(const RT rt) noexcept {
    return rt == RT::unique_solving or rt == RT::unique_s_with_log or
      rt == RT::unique_decision or rt == RT::unique_d_with_log;
  }
  constexpr size_t with_stop(const RT rt) noexcept {
    if (test_sat(rt)) return 1;
    else if (test_unique(rt)) return 2;
    else return 0;
  }


  // Propagation levels for Gecode-constraints:
  enum class PropO {
    dom=0, // domain propagation (strongest)
    def=1, // default propagation (different for different constraints)
    val=2, // values propagation (weakest)
    bnd=3 // bounds propagation
  };
  constexpr int PropOsize = int(PropO::bnd) + 1;
  GC::IntPropLevel prop_level(const PropO po) noexcept {
    switch (po) {
    case PropO::dom: return GC::IPL_DOM;
    case PropO::def: return GC::IPL_DEF;
    case PropO::val: return GC::IPL_VAL;
    case PropO::bnd: return GC::IPL_BND;
    default : return GC::IPL_DOM;}
  }

  // Variable-selection for Gecode-branching ("branching-heuristic variables").
  // According to Section 8.5.1 of 'Modeling and Programming with Gecode.
  // 6.2.0.', the degree of a variable is the number of propagators depending
  // on it.
  enum class BHV {
    mindom = 0, // smallest domain
    maxdeg = 1, // largest degree
    maxdegdom = 2, // largest degree/domain
    first = 3, // first open
    mindeg = 4, // smallest degree
    maxdom = 5, // largest domain
    mindegdom = 6, // smallest degree/domain
  };
  constexpr int BHVsize = int(BHV::mindegdom) + 1;
  GC::IntVarBranch var_branch(const BHV bvar) {
    switch (bvar) {
    case BHV::mindom: return GC::INT_VAR_SIZE_MIN();
    case BHV::maxdeg: return GC::INT_VAR_DEGREE_MAX();
    case BHV::maxdegdom: return GC::INT_VAR_DEGREE_SIZE_MAX();
    case BHV::first: return GC::INT_VAR_NONE();
    case BHV::mindeg: return GC::INT_VAR_DEGREE_MIN();
    case BHV::maxdom: return GC::INT_VAR_SIZE_MAX();
    case BHV::mindegdom: return GC::INT_VAR_DEGREE_SIZE_MIN();
    default : throw std::runtime_error("Options::var_branch: UNKNOWN bvar="
                                       + std::to_string(int(bvar)));
    }
  }

  // The branching-type:
  enum class BRT {
    bin = 0, // binary branching
    enu = 1 // enumerative branching
  };
  constexpr int BRTsize = int(BRT::enu) + 1;
  // The Gecode-branching-orders:
  enum class GBO {
    asc = 0, // ascending
    desc = 1 // descending
  };
  constexpr int GBOsize = int(GBO::desc) + 1;

  // Values-section for Gecode-branching (also determining the branching-type
  // "binary" vs "enumerativ"); for us "branching-heuristic order":
  enum class BHO {
    binvalmin = 0, // INT_VAL_MIN()
    binvalmax = 1, // INT_VAL_MAX()
    enumvalmin = 2, // INT_VALUES_MIN()
    enumvalmax = 3 // INT_VALUES_MAX()
  };
  constexpr int BHOsize = int(BHO::enumvalmax) + 1;
  constexpr BHO translate(const BRT bt, const GBO bo) noexcept {
    if (bt == BRT::bin)
      if (bo == GBO::asc) return BHO::binvalmin;
      else return BHO::binvalmax;
    else
      if (bo == GBO::asc) return BHO::enumvalmin;
      else return BHO::enumvalmax;
  }
  GC::IntValBranch val_branch(const BHO bord) {
    switch (bord) {
    case BHO::binvalmin: return GC::INT_VAL_MIN();
    case BHO::binvalmax: return GC::INT_VAL_MAX();
    case BHO::enumvalmin: return GC::INT_VALUES_MIN();
    case BHO::enumvalmax: return GC::INT_VALUES_MAX();
    default : throw std::runtime_error("Options::val_branch: UNKNOWN bord="
                                       + std::to_string(int(bord)));
    }
  }

  // Algorithmic options for lookahead-reduction, in two dimensions:
  // relaxed vs eager and pruning vs nonpruning
  enum class LAR {
    rel_pr = 0,
    rel_npr = 1,
    eag_pr = 2,
    eag_npr = 3
  };
  constexpr int LARsize = int(LAR::eag_npr) + 1;
  constexpr bool eager(const LAR lar) noexcept { return int(lar) >= 2; }
  constexpr bool pruning(const LAR lar) noexcept { return int(lar) % 2 == 0; }

}
namespace Environment {
  template <> struct RegistrationPolicies<Options::RT> {
    static constexpr const char* sname = "rt";
    static constexpr int size = Options::RTsize;
    static constexpr std::array<const char*, size>
      string {"sats", "satd",
        "uniques", "+uniques", "uniqued", "+uniqued",
        "count", "+count",
        "enum", "+enum"};
    static constexpr std::array<const char*, size>
      estring {"sat-solving", "sat-decision",
        "unique-solving", "unique-sol-with-log",
        "unique-decision", "unique-dec-with-log",
        "count-solutions", "count-sols-with-log",
        "enumerate-solutions", "enum-sols-with-log"};
  };
  template <> struct RegistrationPolicies<Options::PropO> {
    static constexpr const char* name = "propagation-level";
    static constexpr const char* sname = "pl";
    static constexpr int size = Options::PropOsize;
    static constexpr std::array<const char*, size>
      string {"dom", "def", "val", "bnd"};
    static constexpr std::array<const char*, size>
      estring {"domain-prop", "default-prop", "values-prop", "bounds-prop"};
  };
  template <> struct RegistrationPolicies<Options::BHV> {
    static constexpr const char* name = "variable-heuristic";
    static constexpr const char* sname = "bv";
    static constexpr int size = Options::BHVsize;
    static constexpr std::array<const char*, size>
    string {"mindom", "maxdeg", "maxdegdom",
        "first", "mindeg", "maxdom", "mindegdom"};
    static constexpr std::array<const char*, size>
      estring {"min-dom-var", "max-deg-var", "max-deg/dom-var",
        "first-var", "min-deg-var", "max-dom-var", "min-deg/dom-var"};
  };
  template <> struct RegistrationPolicies<Options::BRT> {
    static constexpr const char* name = "branching-type";
    static constexpr const char* sname = "bt";
    static constexpr int size = Options::BRTsize;
    static constexpr std::array<const char*, size>
      string {"bin", "enu"};
    static constexpr std::array<const char*, size>
      estring {"binary-branching", "enumerative-branching"};
  };
  template <> struct RegistrationPolicies<Options::GBO> {
    static constexpr const char* name = "order-heuristic";
    static constexpr const char* sname = "bo";
    static constexpr int size = Options::GBOsize;
    static constexpr std::array<const char*, size>
      string {"asc", "desc"};
    static constexpr std::array<const char*, size>
      estring {"ascending-order", "descending-order"};
  };
  template <> struct RegistrationPolicies<Options::BHO> {
    static constexpr int size = Options::BHOsize;
    static constexpr std::array<const char*, size>
      string {"bmin", "bmax", "emin", "emax"};
    static constexpr std::array<const char*, size>
      estring {"bin-branch-min", "bin-branch-max", "enum-branch-min",
        "enum-branch-max"};
  };
  template <> struct RegistrationPolicies<Options::LAR> {
    static constexpr const char* name = "la-reduction-type";
    static constexpr const char* sname = "lar";
    static constexpr int size = Options::LARsize;
    static constexpr std::array<const char*, size>
      string {"relpr", "relnpr", "eagpr", "eagnpr"};
    static constexpr std::array<const char*, size>
      estring {"relaxed-pruning", "relaxed-nonpruning",
        "eager-pruning", "eager-nonpruning"};
  };
}
namespace Options {
  std::ostream& operator <<(std::ostream& out, const RT rt) {
    return out << Environment::W2(rt);
  }
  std::ostream& operator <<(std::ostream& out, const PropO po) {
    return out << Environment::W2(po);
  }
  std::ostream& operator <<(std::ostream& out, const BHV bvar) {
    return out << Environment::W2(bvar);
  }
  std::ostream& operator <<(std::ostream& out, const BRT bt) {
    return out << Environment::W2(bt);
  }
  std::ostream& operator <<(std::ostream& out, const GBO bo) {
    return out << Environment::W2(bo);
  }
  std::ostream& operator <<(std::ostream& out, const BHO bord) {
    return out << Environment::W2(bord);
  }
  std::ostream& operator <<(std::ostream& out, const LAR lar) {
    return out << Environment::W2(lar);
  }

}

#endif
