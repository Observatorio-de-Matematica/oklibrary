// Oleg Zaikin, 29.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

Look-ahead reduction for the Gecode library.

BUGS:

TODOS:

0. DONE (If one solution is enough, the statistics is returned after
     processing of the variable where the solution was found)
   Use the run-type parameter.
    - If sat-decision or sat-solving, return when a solutions if found.
    - If count or enumeration, collect all soluitons.

1. Lookahead-reduction statistics.
    - DONE This is a return value of the reduction function.
    - The reduction-statistics is used in the choice() function of a customised
      brancher to update the global statistics.
    - Statistics collected here (the "important events"):
      - DONE the propagation-counter
      - DONE the number of eliminated values
      - DONE the quotient eliminated-values / all-values
      - the number of successful prunings
      - DONE the number of probings
      - the quotient prunings/probings
      - DONE the number of rounds
      - the final size of the pruning-set
      - DONE the total time for the reduction
      - DONE the number of satisfying assignments found.

2. Find all solutions in a lookahead reduction function.
    - DONE All solutions which are found during the reduction, are counted.
    - If it is needed, save the solutions to the local statistics.
    - DONE The corresponding branches are cut off so Gecode is not aware of any
      found solutions.

3. Pruning.
    - Should work in all types of reductions.


*/

#ifndef LOOKAHEADREDUCTION_fJ5peeEDiH
#define LOOKAHEADREDUCTION_fJ5peeEDiH

#include <vector>
#include <memory>

#include <cassert>

#include <gecode/int.hh>
#include <gecode/search.hh>

#include <SystemSpecifics/Timing.hpp>
#include <Numerics/FloatingPoint.hpp>

#include "Conditions.hpp"
#include "Options.hpp"

namespace LookaheadReduction {

  namespace FP = FloatingPoint;
  namespace GC = Gecode;
  namespace OP = Options;
  namespace CD = Conditions;

  using size_t = CD::size_t;
  typedef FP::float80 float_t;
  typedef std::int64_t signed_t;

  // Array of values of an integer variable:
  typedef GC::Int::IntView IntView;
  // Array of array of values of Gecode integer variables:
  typedef GC::ViewArray<IntView> IntViewArray;
  // Value iterator for an integer variable:
  typedef GC::IntVarValues IntVarValues;

  typedef std::vector<int> values_t;

  enum class BranchingStatus { unsat=0, sat=1, single=2, branching=3 };

  // Make a copy of a given problem and assign either var==val or var!=val:
  template<class ModSpace>
  std::unique_ptr<ModSpace> child_node(ModSpace* const m,
                                       const int v, const int val,
                                       const GC::IntPropLevel pl,
                                       const bool eq = true) noexcept {
    assert(m->valid());
    assert(m->valid(v));
    assert(m->status() == GC::SS_BRANCH);
    // Clone space:
    std::unique_ptr<ModSpace> c(static_cast<ModSpace*>(m->clone()));
    assert(c->valid());
    assert(c->valid(v));
    assert(c->status() == GC::SS_BRANCH);
    // Add an equality constraint for the given variable and its value:
    if (eq) GC::rel(*(c.get()), (c.get())->var(v), GC::IRT_EQ, val, pl);
    else GC::rel(*(c.get()), (c.get())->var(v), GC::IRT_NQ, val, pl);
    return c;
  }

  // Statistics of the main lookahead-reduction actions:
  struct ReductionStatistics {
  private :
    size_t props_ = 0; // the propagation-counter
    size_t vals_ = 0; // the number of all-values
    size_t elimvals_ = 0; // the number of eliminated values
    float_t quotelimvals_ = 0.0; // the quotient eliminated-values / all-values
    size_t pruns_ = 0; // the number of successful prunings
    size_t probes_ = 0; // the number of probings
    float_t quotprun_ = 0.0; // the quotient prunings / probings
    size_t rounds_ = 0; // the number of rounds
    size_t prunsetsize_ = 0; // the final size of the pruning-set
    float_t time_ = 0.0; // the total time for the reduction
    size_t sols_ = 0; // the number of satisfying assignments found.
    void update_quotelimvals() noexcept {
      assert(vals_ > 0);
      quotelimvals_ = float_t(elimvals_)/vals_;
    }
    void update_quotprun() noexcept {
      assert(probes_ > 0);
      quotprun_ = float_t(pruns_)/probes_;
    }
  public:
    void increment_props() noexcept { ++props_; }
    void update_allvalues(const IntViewArray& x) noexcept {
      assert(x.size() > 0);
      size_t sum = 0;
      for (signed_t var = 0; var < x.size(); ++var) sum += x[var].size();
      assert(sum > 0);
      vals_ = sum;
    }
    void increment_elimvals() noexcept { ++elimvals_; update_quotelimvals(); }
    void increment_pruns() noexcept { ++pruns_; update_quotprun();}
    void increment_probes() noexcept { ++probes_; update_quotprun();}
    void increment_rounds() noexcept { ++rounds_; }
    void update_prunsetsize(const size_t size) noexcept {
      prunsetsize_ = size;
    }
    void update_time(const float_t t) noexcept { time_ = t; }
    void increment_sols() noexcept { ++sols_; }
    size_t props() const noexcept { return props_; }
    size_t elimvals() const noexcept { return elimvals_; }
    float_t quotelimvals() const noexcept { return quotelimvals_;}
    size_t pruns() const noexcept { return pruns_; }
    size_t probes() const noexcept { return probes_; }
    float_t quotprun() const noexcept { return quotprun_; }
    size_t rounds() const noexcept { return rounds_; }
    size_t prunsetsize() const noexcept { return prunsetsize_; }
    float_t time() const noexcept { return time_; }
    size_t sols() const noexcept { return sols_; }
  };

  // Lookahead-reduction.
  // Consider a variable var and its domain {val1, ..., valk}.
  // For all i, if var==vali is inconsistent, then these constraints
  // (var==vali) are collected and then (after the loop for all i) the
  // corresponding constraints are applied and a Gecode propagation is
  // performed. In such a way, all impossible values of a variable are removed.
  template<class ModSpace>
  ReductionStatistics lareduction(GC::Space& home,
                        const OP::RT rt,
                        const GC::IntPropLevel pl,
                        const OP::LAR lar) noexcept {
    ReductionStatistics stat;
    ModSpace* m = &(static_cast<ModSpace&>(home));
    assert(m->status() == GC::SS_BRANCH);
    bool repeat = false;
    stat.update_allvalues(m->var());
    Timing::UserTime timing;
    const Timing::Time_point t0 = timing();
    do {
      repeat = false;
      stat.increment_rounds();
      // Get current array of variables:
      const IntViewArray x = m->var();
      // Iterate over all unassigned variables:
      for (signed_t var = 0; var < x.size(); ++var) {
        const IntView view = x[var];
        if (view.assigned()) continue;
        assert(view.size() >= 2);
        // All such val that var!=val:</font>
        values_t noteqvalues;

        // Collect all values of the variable to a vector:
        std::vector<int> values;
        for (IntVarValues j(view); j(); ++j) values.push_back(j.val());
        // Iterate over all values of the current variable:
        for (auto const& val : values) {
          assert(m->status() == GC::SS_BRANCH);
          // Make a copy of the current problem, and assign var==val:
          const auto subm = child_node<ModSpace>(m, var, val, pl, true);
          stat.increment_probes();
          // Call Gecode propagation:
          const auto status = subm->status();
          stat.increment_props();
          // If either a SAT leaf or an UNSAT leaf is found:
          if (status != GC::SS_BRANCH) {
            assert(status == GC::SS_SOLVED or status == GC::SS_FAILED);
            // In either case the branch must be excluded,
            // so Gecode will not be aware of UNSAT or SAT leaves.
            stat.increment_elimvals();
            noteqvalues.push_back(val);
            // SAT leaf:
            if (status == GC::SS_SOLVED) {
              // Add a solution to the statistics:
              stat.increment_sols();
            }
          }
        }

        // Apply all var!=val assignments for the variable in one batch:
        if (not noteqvalues.empty()) {
          for (auto& val : noteqvalues) {
            GC::rel(home, x[var], GC::IRT_NQ, val, pl);
          }
          // Call a propagation:
          const auto status = home.status();
          stat.increment_props();
          // Check if the problem is solved:
          if (status == GC::SS_SOLVED) {
            // Add the solution to the statistics:
            stat.increment_sols();
          }
          else if (status == GC::SS_FAILED) {
            // The problem is solved and the answer is UNSAT:
            // XXX
          }
          // If super-eager, and any propagation was done in the
          // variables-loop, then restart the variables-loop immediately
          // after processing the variable. If no propagation was done during
          // variables-loop, finish the reduction. Do not update the flag
          // repeatred, so it is remains false.
          // If eager, and any propagation was done during variables-loop,
          // set the flag repeatred to true, process the variables-loop
          // entirely, and then repeat the variables-loop.
          if (lar == OP::LAR::supeager) { assert(not repeat); break; }
          else if (lar == OP::LAR::eager) repeat = true;
        }

        // If one solution is enough, do not process other variables:
        if (rt == OP::RT::sat_decision or rt == OP::RT::sat_solving) {
          assert(stat.sols() > 0);
          return stat;
        }
      } // for (int var = start; var < x.size(); ++var) {
    } while (repeat);

    const Timing::Time_point t1 = timing();
    const Timing::Time_point t = t1 - t0;
    stat.update_time(t);

    return stat;
  }

}

#endif
