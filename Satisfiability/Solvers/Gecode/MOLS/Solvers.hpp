// Oliver Kullmann, 17.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Solvers for the LS-MOLS-conditions, for Gecode

  Namespace Solvers, abbreviated "SV".


  Types for handling of solver-results:

   - import of Options::RT
   - typedef listsol_t for a vector of PSquares

   - class BasicSR for the direct solver-results
     function valid(BasicSR)
   - class GBasicSR extends this by (gecode-)statistics.

  Solver for unit-testings (as simple as possible):

   - main function solver_basis
   - helper function solver0 in two overloads

  The pure Gecode-solver:

   - helper-function make_options
   - main function gcsolver_basis
   - helper function solver_gc

  The half look-ahead solver (only la-reduction):

   - main function rlasolver
   - helper function solver_rla

  The full look-ahead solver:

   - main function lasolver
   - helper function solver_la.


TODOS:

-1. Update and complete rla- and la-solver
    - Following the model of the (updated) gc-solver.
    - First rla, then la.
    - Always providing a complete solution (there is no reason to provide
      only some of the rt-modes).

0. DONE (A Gecode solution is total.)
   Is a Gecode "solution" a *total* or a *partial* assignment?
    - In MPG.pdf, Page 19, one finds
      "A search engine ensures that constraint propagation is performed and
       that all variables are assigned as described by the branching(s) of
       the model passed to the search engine."
      That sounds as if only total assignments are produced.
    - Also the test-cases in TestSolvers.cpp, which pose no constraint, return
      only total assignments.
    - OK went through occurrences of "solution" in MPG.pdf, and could not find
      any hint that solutions would not be total. If partial solutions would
      be possible, that one needed quite some discussion about handling that.
      So it seems that indeed solutions are always total.
    - With look-ahead we abort once a "solution" was found: when propagation
      is carried completely, also this should to total.

1. DONE Statistics for the gc-solver
    - DONE At least the user-runtime.
    - DONE Plus possibly everthing Gecode has on offer:
     - Using the statistics() member function of the search engine
       (Section 9.3 of MPG.pdf).
     - Figure 9.5 gives 6 numbers; restart and nogood currently are not used.
     - The object is of type GC::Search::Statistics
       https://www.gecode.org/doc/6.2.0/reference/classGecode_1_1Search_1_1Statistics.html


*/

#ifndef SOLVERS_PNeIRm1Ic7
#define SOLVERS_PNeIRm1Ic7

#include <vector>
#include <istream>
#include <ostream>
#include <exception>
#include <utility>

#include <cassert>

#include <gecode/int.hh>
#include <gecode/search.hh>

#include <SystemSpecifics/Timing.hpp>

#include "Conditions.hpp"
#include "Encoding.hpp"
#include "Constraints.hpp"
#include "Parsing.hpp"
#include "PartialSquares.hpp"
#include "Options.hpp"
#include "LookaheadBranching.hpp"
#include "BasicLatinSquares.hpp"
#include "Verification.hpp"

namespace Solvers {

  namespace GC = Gecode;
  namespace CD = Conditions;
  namespace EC = Encoding;
  namespace CT = Constraints;
  namespace PR = Parsing;
  namespace PS = PartialSquares;
  namespace OP = Options;
  namespace LB = LookaheadBranching;
  namespace BS = BasicLatinSquares;
  namespace VR = Verification;

  using size_t = CD::size_t;


  typedef Options::RT RT;
  typedef std::vector<PS::PSquares> listsol_t;

  // Simplest solver-return:
  struct BasicSR {
    RT rt = RT::sat_decision;
    size_t sol_found = 0;
    listsol_t list_sol;

    bool operator ==(const BasicSR&) const noexcept = default;
    friend std::ostream& operator <<(std::ostream& out, const BasicSR& sr) {
      out << sr.rt << " " << sr.sol_found << "\n";
      Environment::out_line(out, sr.list_sol, "");
      return out;
    }
    static void rh(std::ostream& out) { out << "rt sat"; }
    void rs(std::ostream& out) const {
      using Environment::W0;
      out << W0(rt) << " " << sol_found; }
  };
  inline bool valid(const BasicSR sr) noexcept {
    if (sr.rt == RT::sat_decision)
      return sr.sol_found <= 1 and sr.list_sol.empty();
    else if (sr.rt == RT::sat_solving)
      return sr.sol_found <= 1 and sr.list_sol.size() == sr.sol_found;
    else if (test_unique(sr.rt)) {
      if (sr.sol_found > 2) return false;
      if (sr.rt == RT::unique_decision or sr.rt == RT::unique_d_with_log)
        return sr.list_sol.empty();
      else return sr.sol_found == sr.list_sol.size();
    }
    else if (count_only(sr.rt))
      return sr.list_sol.empty();
    else return sr.sol_found == sr.list_sol.size();
  }

  struct GBasicSR {
    BasicSR b;
    typedef GC::Search::Statistics gc_stats_t;
    gc_stats_t gs;
    double ut = 0;

    bool operator ==(const GBasicSR&) const noexcept = default;
    static void rh(std::ostream& out) {
      BasicSR::rh(out);
      out << " t prop flvs nds h";
    }
    void rs(std::ostream& out) const {
      b.rs(out); out << " ";
      FloatingPoint::out_fixed_width(std::cout, 3, ut);
      out << " " << gs.propagate << " " << gs.fail <<
        " " << gs.node << " " << gs.depth;
    }
  };

  struct rlaSR : GBasicSR {
    using GBasicSR::b;
    using GBasicSR::gs;
    using GBasicSR::ut;
    typedef LB::rlaStats::stats_t stats_t;
    stats_t S;

    bool operator ==(const rlaSR&) const noexcept = default;
  };


  /*
    The solver for the testing of encodings etc.

    This solver is assumed to run with asserts enabled, and thus no need
    for explicit testing of the results.
  */

  BasicSR solver_basis(const EC::EncCond& enc, const RT rt,
                       const GC::IntVarBranch vrb,
                       const GC::IntValBranch vlb) {
    CT::GenericMols0* const gm = new CT::GenericMols0(enc);
    GC::branch(*gm, gm->V, vrb, vlb);
    GC::DFS<CT::GenericMols0> s(gm);
    delete gm;

    BasicSR res{rt};
    if (with_log(rt))
      throw std::runtime_error("ERROR[Solvers]::solver_basis: wrong rt-code="
                               + std::to_string(int(rt)));
    else if (rt == RT::sat_decision) {
      if (CT::GenericMols0* const leaf=s.next()){res.sol_found=1;delete leaf;}
    }
    else if (rt == RT::sat_solving) {
      if (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        res.list_sol.push_back(enc.decode(leaf->V));
        res.sol_found = 1; delete leaf;
      }
    }
    else if (test_unique(rt)) {
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.sol_found;
        if (rt == RT::unique_solving) {
          assert(EC::EncCond::unit(leaf->V));
          res.list_sol.push_back(enc.decode(leaf->V));
        }
        delete leaf;
        if (res.sol_found == 2) break;
      }
    }
    else if (rt == RT::count_solutions) {
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.sol_found; delete leaf;
      }
    }
    else if (rt == RT::enumerate_solutions) {
      while (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        res.list_sol.push_back(enc.decode(leaf->V));
        ++res.sol_found; delete leaf;
      }
    }
    else
      assert(0 == 1);
    return res;
  }

  BasicSR solver0(const EC::EncCond& enc, const RT rt) {
    return solver_basis(enc, rt, GC::INT_VAR_SIZE_MIN(), GC::INT_VAL_MIN());
  }

  BasicSR solver0(const RT rt, const size_t N,
                  std::istream& in_cond, std::istream& in_ps) {
    const auto ac = PR::ReadAC()(in_cond);
    // Remark: ac must be constructed first, due to the (global)
    // names of squares.
    const auto ps = PS::PSquares(N, in_ps);
    return solver0(EC::EncCond(ac, ps), rt);
  }


  /*
    The pure Gecode-solver
  */
  GC::Search::Options make_options(const double t) noexcept {
    GC::Search::Options res; res.threads = t;
    return res;
  }
  GBasicSR gcsolver_basis(const EC::EncCond& enc, const RT rt,
                          const GC::IntVarBranch vrb,
                          const GC::IntValBranch vlb,
                          const double threads,
                          std::ostream* const log) {
    assert(valid(rt));
    assert(not with_log(rt) or log);
    CT::GenericMols0* const gm = new CT::GenericMols0(enc);
    GC::branch(*gm, gm->V, vrb, vlb);
    GC::DFS<CT::GenericMols0> s(gm, make_options(threads));
    delete gm;

    GBasicSR res{rt};
    switch (rt) {
    case RT::sat_decision: {
      if (CT::GenericMols0* const leaf=s.next()) {
        res.b.sol_found = 1;
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution:\n" << sol << "\n";
        delete leaf;
      }
      res.gs = s.statistics(); break;
    }
    case RT::sat_solving: {
      if (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        res.b.list_sol.push_back(sol);
        res.b.sol_found = 1;
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution:\n" << sol << "\n";
        delete leaf;
      }
      res.gs = s.statistics(); break;
    }
    case RT::unique_solving: {
      while (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        res.b.list_sol.push_back(sol);
        ++res.b.sol_found;
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
        if (res.b.sol_found == 2) break;
      }
      res.gs = s.statistics(); break;
    }
    case RT::unique_decision: {
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.b.sol_found;
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
        if (res.b.sol_found == 2) break;
      }
      res.gs = s.statistics(); break;
    }
    case RT::unique_s_with_log: {
      assert(log);
      while (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        ++res.b.sol_found;
        const auto sol = enc.decode(leaf->V);
        *log << res.b.sol_found << "\n" << sol << std::endl;
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
        if (res.b.sol_found == 2) break;
      }
      res.gs = s.statistics(); break;
    }
    case RT::unique_d_with_log: {
      assert(log);
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.b.sol_found;
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
        *log << " " << res.b.sol_found; log->flush();
        if (res.b.sol_found == 2) break;
      }
      res.gs = s.statistics(); break;
    }
    case RT::count_solutions: {
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.b.sol_found; delete leaf;
      }
      res.gs = s.statistics(); break;
    }
    case RT::count_with_log: {
      assert(log);
      while (CT::GenericMols0* const leaf = s.next()) {
        ++res.b.sol_found; delete leaf;
        *log << " " << res.b.sol_found; log->flush();
      }
      res.gs = s.statistics(); break;
    }
    case RT::enumerate_solutions: {
      while (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        const auto sol = enc.decode(leaf->V);
        res.b.list_sol.push_back(sol);
        ++res.b.sol_found;
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
      }
      res.gs = s.statistics(); break;
    }
    case RT::enumerate_with_log: {
      assert(log);
      while (CT::GenericMols0* const leaf = s.next()) {
        assert(EC::EncCond::unit(leaf->V));
        ++res.b.sol_found;
        const auto sol = enc.decode(leaf->V);
        *log << res.b.sol_found << "\n" << sol << std::endl;
        if (not VR::correct(enc.ac, sol))
          std::cerr << "\nERROR[Solvers::gcsolver_basis]: "
            "correctness-checking failed for solution " << res.b.sol_found
                    << ":\n" << sol << "\n";
        delete leaf;
      }
      res.gs = s.statistics(); break;
    }}
    return res;
  }

  GBasicSR solver_gc(const EC::EncCond& enc, const RT rt,
                     const GC::IntVarBranch vrb,
                     const GC::IntValBranch vlb,
                     const double threads = 1,
                     std::ostream* const log = nullptr) {
    Timing::UserTime timing;
    const Timing::Time_point t0 = timing();
    GBasicSR res = gcsolver_basis(enc, rt, vrb, vlb, threads, log);
    const Timing::Time_point t1 = timing();
    res.ut = t1 - t0;
    return res;
  }


  /*
    The solver with look-ahead-reduction and gecode-branching
  */
  struct sol_count_stop : GC::Search::Stop {
    bool stop(const GC::Search::Statistics&, const GC::Search::Options&) {
      return LB::rlaStats::abort.load(std::memory_order_relaxed);
    }
  };
  GC::Search::Options make_options(const double t,
                                   const OP::RT rt) noexcept {
    GC::Search::Options res;
    res.threads = t;
    if (with_stop(rt)) res.stop = new sol_count_stop;
    return res;
  }
  rlaSR rlasolver(const EC::EncCond& enc,
                  const OP::RT rt,
                  const OP::BRT bt, const OP::BHV bv, const OP::GBO bo,
                  const OP::LAR lar,
                  const double threads,
                  std::ostream* const log) {
    assert(valid(rt));
    assert(not with_log(rt) or log);

    Timing::UserTime timing;
    const Timing::Time_point t0 = timing();
    CT::GenericMols0* const m = new CT::GenericMols0(enc);
    const LB::rlaParams P{rt, enc.pl, lar, bv, bt, bo, threads != 1};
    std::unique_ptr<LB::rlaStats> stats(
      new LB::rlaStats(log,
                       log and OP::with_solutions(rt) ? &enc : nullptr,
                       with_stop(rt)));
    new (*m) LB::RlaBranching(*m, P, stats.get());
    GC::DFS<CT::GenericMols0> s(m, make_options(threads, rt));
    delete m;

    rlaSR res{rt};
    {[[maybe_unused]]CT::GenericMols0* const leaf=s.next(); assert(not leaf);}
    res.ut = timing() - t0;
    res.gs = s.statistics();
    res.b.sol_found = stats->sol_count();
    res.S = stats->stats();
    if (with_file_output(rt)) {
      if (res.b.sol_found != stats->sols().size())
        std::cerr << "\nERROR[Solvers::rlasolver]: stated solution-count "
                  << res.b.sol_found << " != real solution-count "
                  << stats->sols().size() << "\n";
      for (size_t i = 0; i < res.b.sol_found; ++i) {
        auto dsol = enc.decode(stats->sols()[i]);
        if (not VR::correct(enc.ac, dsol))
          std::cerr << "\nERROR[Solvers::rlasolver]: "
            "correctness-checking failed for solution " << i
                    << ":\n" << dsol << "\n";
        res.b.list_sol.push_back(std::move(dsol));
      }
      if (not BS::alldiffelem(res.b.list_sol))
        std::cerr << "\nERROR[Solvers::rlasolver]: "
          "there are equal elements in the solution-list\n";
    }
    if (not valid(res.b))
      std::cerr << "\nERROR[Solvers::rlasolver]: "
        "failed basic consistency-check\n";
    return res;
  }

}

#endif
