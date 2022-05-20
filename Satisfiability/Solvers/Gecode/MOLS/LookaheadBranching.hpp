// Oleg Zaikin, 31.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  Look-ahead branching for the Gecode library

  Namespace LookaheadBranching, abbreviated "LB".

BUGS:

TODOS:

0. What is GC::me_failed good for?
    - What does it mean to "fail" here??
      Answer: it means that the operation can not be executed.
      For example, if val is *not* in domain of Gecode variable var, then
      GC::me_failed(var.eq(home, val))
      will return ES_FAILED, otherwise ES_OK.
      In this case the function eq() just checks whether val in domain of var,
      i.e. the equality constraint in fact is not posted here - just the
      possibility of this posting is checked.
    - Is status called by that function?
      Answer: No. Neither the operation nor propagation is perormed.
    - It seems conceptually cleaner to just use GC::rel (as in "probe").
    - We can also call "status" in there if needed.
      Answer: if the operation is possible (me_failed() returns ES_OK), then
      then *after* the commit() function a space is cloned, the operation
      is performed, and finally propagation is performed. All these actions
      are done outside the brancher-class.
      If la-reduction is used in commit(), then once SAT or UNSAT leaf is
      formed (and taken into account in own statistics), ES_FAILED can be
      returned, so no child-node will be created.

1. Likely all the old classes here should be abondoned.
    - Exactly two classes are needed, for rla and la, and that should
      be all.
    - All the existing classes seem to lack focus.

2. What is the point of IntView ?
    - It seems we never need this?
    Comment: In Gecode, variables only offer operations for accessing but not
    removing values. In opposite, variable views provide both operations. Views
    are used e.g. in propagators, where some variables' values can be removed.
    In our circumstances, it seems that variables views can be avoided, because
    no direct removal happens. Values of variables can be accessed in similar
    way as for view, i.e. via IntVarValues, for example
    for (IntVarValues j(x[var]); j(); ++j)
    where x[var] is IntVar but not IntView.

3. Why does "new_vars" use a *vector* of weights?
    - Now using a pointer; the whole concept needs revision, once
      one sees the whole approach.
    Comment: sure, a pointer is proper.

4. Measure-based distances
    - The number of eliminated values is Delta GV::sumdomsizes.
    - More generally Delta domsizes is used.
    - The weight for dom-size 1 is zero; this is part of the initialisation of
      the weight-vector (from the parameters).

*/

#ifndef LOOKAHEADBRANCHING_wXJWMxXz3R
#define LOOKAHEADBRANCHING_wXJWMxXz3R

#include <vector>
#include <algorithm>

#include <cassert>
#include <cstdlib>

#include <gecode/int.hh>
#include <gecode/search.hh>

#include <Numerics/FloatingPoint.hpp>
#include <Numerics/Tau.hpp>

#include "Conditions.hpp"
#include "Options.hpp"
#include "Constraints.hpp"
#include "GcVariables.hpp"
#include "LookaheadReduction.hpp"

namespace LookaheadBranching {

  namespace FP = FloatingPoint;
  namespace GC = Gecode;
  namespace LR = LookaheadReduction;
  namespace OP = Options;
  namespace CD = Conditions;
  namespace CT = Constraints;
  namespace GV = GcVariables;

  using size_t = CD::size_t;
  typedef std::vector<int> values_t;
  typedef FP::float80 float_t;
  typedef std::vector<float_t> vec_t;
  // A branching tuple, i.e. a tuple of distances:
  typedef std::vector<float_t> bt_t;

  // Array of values of an integer variable:
  typedef GC::Int::IntView IntView;
  // Array of array of values of Gecode integer variables:
  typedef GC::ViewArray<IntView> IntViewArray;
  // Value iterator for an integer variable:
  typedef GC::IntVarValues IntVarValues;

  // Converting int to size_t:
  inline constexpr size_t tr(const int size, [[maybe_unused]] const size_t bound = 0) noexcept {
    assert(bound <= std::numeric_limits<int>::max());
    assert(size >= int(bound));
    return size;
  }


  /* Distance and measure functions

     PRELIMINARY
  */
  float_t new_vars(const GC::IntVarArray& V, const GC::IntVarArray& Vn,
                   const vec_t* const wghts, const size_t depth) noexcept {
    const size_t n = tr(V.size(), 1);
    float_t s = 0;
    const float_t w1 = FP::exp((*wghts)[0] * depth);
    for (size_t i = 0; i < n; ++i) {
      const size_t ds = tr(V[i].size(), 1), dsn = tr(Vn[i].size(), 1);
      if (dsn == ds) continue;
      assert(1 <= dsn and dsn < ds);
      if (dsn == 1) s += w1;
      else {
        assert(dsn-1 < wghts->size());
        s += (*wghts)[dsn-1];
      }
    }
    return s;
  }
  float_t domsizes(const GC::IntVarArray& V, const vec_t* const wghts) noexcept {
    const size_t n = tr(V.size(), 1);
    float_t s = 0;
    for (size_t i = 0; i < n; ++i) {
      const size_t ds = tr(V[i].size(), 1);
      assert(ds >= 1);
      assert(ds-1 < wghts->size());
      s += (*wghts)[ds-1];
    }
    return s;
  }


  struct RlaBranching : GC::Brancher {
    RlaBranching(const GC::Home home) : GC::Brancher(home) {}
    RlaBranching(GC::Space& home, RlaBranching& b) : GC::Brancher(home,b) {}
    GC::Brancher* copy(GC::Space& home) {
      return new (home) RlaBranching(home,*this);
    }
    std::size_t dispose(GC::Space&) noexcept { return sizeof(*this); }

    bool status(const GC::Space& s) const noexcept {
      return not GcVariables::empty(static_cast<const CT::GenericMols0&>(s).V);
    }

    struct C : GC::Choice {
      typedef GV::values_t values_t;
      const values_t br; // br[0] is the variable (if existent)
      C(const RlaBranching& b, const values_t branching) noexcept :
      GC::Choice(b, width(branching)), br(branching) {}
      static unsigned width(const values_t& br) noexcept {
        const unsigned size = br.size();
        if (size == 0) return 1;
        assert(size >= 2);
        return size == 2 ? 2 : size-1;
      }
    };

    GC::Choice* choice(GC::Space&) {
      return nullptr; // XXX
    }
    GC::Choice* choice(const GC::Space&, GC::Archive&) {
      throw std::runtime_error("RlaMols::choice(Archive): not implemented.");
    }
    GC::ExecStatus commit(GC::Space& s, const GC::Choice& c0, const unsigned a) {
      const C& c = static_cast<const C&>(c0);
      const size_t w = c.br.size();
      if (w == 0) return GC::ExecStatus::ES_FAILED;
      const int v = c.br[0];
      assert(v >= 0);
      CT::GenericMols0* const node = &(static_cast<CT::GenericMols0&>(s));
      assert(v < node->V.size());
      if (w == 2)
        GC::rel(s, node->V[v], a==0 ? GC::IRT_EQ : GC::IRT_NQ, c.br[1]);
      else {
        assert(a+1 < w);
        GC::rel(s, node->V[v], GC::IRT_EQ, c.br[a+1]);
      }
      return GC::ES_OK;
    }
  };


  /* DEPRECATED from here on */

  template<class CustomBranching>
  CustomBranching best_branching(
    std::vector<CustomBranching>& branchings) noexcept {
    assert(not branchings.empty());
    CustomBranching best_br;
    for (auto& br : branchings) best_br = std::min(best_br, br);
    return best_br;
  }

  template<class CustomBranching>
  std::vector<CustomBranching> best_branchings(
    std::vector<CustomBranching>& tau_brs) noexcept {
    std::vector<CustomBranching> branchings;
    assert(not tau_brs.empty());
    CustomBranching br = best_branching<CustomBranching>(tau_brs);
    branchings = {br};
    return branchings;
  }

  // A node in the backtracking tree. All classes that describe problems
  // (like TwoMOLS) should be inherited from this class.
  class Node : public GC::Space {
    // Node's id:
    //size_t ndid;
    // Parent node's id. For the root node, id is 1, while parent id == 0.
    //size_t prntid;
    // Node's depth in the backtracking tree:
    size_t dpth;

  public:
    //Node() : ndid(1), prntid(0), dpth(0) { assert(valid()); }
    Node() : dpth(0) {}

    size_t depth() const noexcept { return dpth; }
    //size_t id() const noexcept { assert(valid()); return ndid; }
    //size_t parentid() const noexcept { assert(valid()); return prntid; }

    /*
    void update_id(const size_t id, const size_t pid) noexcept {
      ndid = id;
      prntid = pid;
      assert(valid());
    }
    */
    void increment_depth() noexcept { ++dpth; }

    // Root node is a special case: id == 1, parent id == 0:
    /*bool valid() const noexcept {
      return dpth > 0;
      //return (ndid > prntid) and
      //        ( (ndid == 1 and prntid == 0) or (ndid > 1 and prntid > 0) );
    }*/
  };

  // Binary branching: for a given variable var and its value val, two branches
  // of the kind var==val and var!=val are formed.
  //  - var : variable.
  //  - value : variable value.
  //  - tuple : branching tuple, where each element corresponds to the
  //    branch-distance.
  //  - ltau :  value of the ltau function for the branching tuple.
  struct BinBranching {
    int var;
    int value;
    bt_t tuple;
    float_t ltau;

    BinBranching(const int v=0, const int val=0, const bt_t tpl={})
      : var(v), value(val), tuple(tpl), ltau(FP::pinfinity) {
      assert(valid());
      calc_ltau();
    }

   bool valid() const noexcept {
     return var >= 0 and not tuple.empty() and ltau >= 0;
   }

    bool operator <(const BinBranching& a) const noexcept {
      return ltau < a.ltau;
    }

    void calc_ltau() noexcept {
      assert(valid());
      assert(not tuple.empty());
      ltau = Tau::ltau(tuple);
      assert(valid());
    }

    size_t branches_num() const noexcept { assert(valid()); return 2; }

  };

  template <class CustomisedBinBrancher>
  struct BinBranchingChoice : public GC::Choice {
    BinBranching br;
    size_t parentid;
    bool valid() const noexcept { return br.valid(); }
    BinBranchingChoice(const CustomisedBinBrancher& b,
                       const BinBranching& br = BinBranching(),
                       const size_t parentid = 0)
      : GC::Choice(b, br.branches_num()), br(br), parentid(parentid) {
        assert(valid());
      }
  };


  // A base customised brancher inherited from Gecode::Brancher.
  // It contains common functionality to reduce code duplications.
  // All lookahead customised branchers should be inherited from it.
  class BaseBrancher : public GC::Brancher {
  protected:
    // Array of variables:
    IntViewArray x;
    // Index of the first unassigned variable:
    mutable int start;

    static bool valid(const IntViewArray x) noexcept { return x.size() > 0; }
    static bool valid(const int s, const IntViewArray x) noexcept {
      return s >= 0 and valid(x) and s < x.size() and x[s].size() >= 2;
    }
  public:
    bool valid() const noexcept { return valid(start, x); }

    BaseBrancher(const GC::Home home, const IntViewArray& x)
      : GC::Brancher(home), x(x), start(0) { assert(valid(start, x)); }
    BaseBrancher(GC::Space& home, BaseBrancher& b)
      : GC::Brancher(home,b), start(b.start) {
      assert(valid(b.x));
      x.update(home, b.x);
      assert(valid(start, x));
    }

    virtual bool status(const GC::Space&) const noexcept {
      assert(valid(start, x));
      for (auto i = start; i < x.size(); ++i)
        if (not x[i].assigned()) { start = i; return true; }
      return false;
    }

    virtual GC::Choice* choice(const GC::Space&, GC::Archive&) noexcept {
      return new BinBranchingChoice<BaseBrancher>(*this);
    }

    template <class ModSpace>
    GC::ExecStatus commit(GC::Space& home, const GC::Choice&,
                          const unsigned) noexcept {
      ModSpace* m = &(static_cast<ModSpace&>(home));
      m->increment_depth();
      return GC::ES_OK;
    }

  };

  // A binary Loookahead brancher. For a variable var and its value val,
  // branching is formed by two branches: var==val and var!=val. The best
  // branching is chosen via the tau-function.
  template <class ModSpace>
  class BinLookahead : public BaseBrancher {
  public:

    using BaseBrancher::BaseBrancher;

    static void post(GC::Home home, const IntViewArray& x) noexcept {
      new (home) BinLookahead(home, x);
    }
    virtual GC::Brancher* copy(GC::Space& home) noexcept {
      return new (home) BinLookahead(home, *this);
    }

    // Apply lookahead-reduction:
    virtual bool status(GC::Space& home) noexcept {
      ModSpace* m = &(static_cast<ModSpace&>(home));
      assert(m->valid());
      assert(m->status() == GC::SS_BRANCH);
      assert(valid(start, x));
      // Find the first unassigned variable, if exists:
      bool issolved = true;
      for (auto i = start; i < x.size(); ++i)
        if (not x[i].assigned()) { start = i; issolved = false; break; }
      // If all variables are assigned, no branching is needed:
      if (issolved) return false;
      // La-reduction parameters:
      const OP::RT rt = m->runtype();
      const GC::IntPropLevel pl = m->proplevel();
      const OP::LAR lar = m->laredtype();
      // Lookahead-reduction:
      LR::ReductionStatistics reduct_stat =
        LR::lareduction<ModSpace>(m, rt, pl, lar);
      // A leaf was found during the la-reduction, so no branching is needed:
      if (reduct_stat.leafcount() > 0) return false;
      // At least one unassigned variable, la-reduction hasn't found a leaf:
      return true;
    }

    virtual GC::Choice* choice(GC::Space& home) noexcept {
      ModSpace* m = &(static_cast<ModSpace&>(home));
      // Since la-reduction was called beforehand in the status() function,
      // no leaves can be found here, so only branching is chosen:
      assert(m->status() == GC::SS_BRANCH);
      const GC::IntPropLevel pl = m->proplevel();
      const vec_t wghts = m->weights();
      const size_t dpth = m->depth();
      std::vector<BinBranching> tau_brs;
      BinBranching best_br;
      // Form all branchings:
      assert(valid(start, x));
      for (int var = start; var < x.size(); ++var) {
        const IntView view = x[var];
        if (view.assigned()) continue;
        assert(view.size() >= 2);
        for (IntVarValues j(view); j(); ++j) {
          const int val = j.val();
          const auto subm_eq =
            LR::child_node<ModSpace>(m, var, val, pl, true);
          [[maybe_unused]] const auto subm_eq_st = subm_eq->status();
          // A leaf (SAT or UNSAT) is impossible here:
          assert(subm_eq_st == GC::SS_BRANCH);
          const float_t dist1 = distance(m->var(), subm_eq->var(), wghts, dpth);
          assert(dist1 > 0);
          const auto subm_neq =
            LR::child_node<ModSpace>(m, var, val, pl, false);
          [[maybe_unused]] const auto subm_neq_st = subm_neq->status();
          assert(subm_neq_st == GC::SS_BRANCH);
          const float_t dist2 = distance(m->var(), subm_neq->var(), wghts, dpth);
          assert(dist2 > 0);
          BinBranching br(var, val, {dist1,dist2});
          tau_brs.push_back(br);
        }
      }
      assert(not tau_brs.empty());
      // Choose the best branchibg:
      best_br = best_branching<BinBranching>(tau_brs);

      [[maybe_unused]] const auto var = best_br.var;
      assert(var >= 0 and var >= start);
      assert(not x[var].assigned() or m->status() == GC::SS_FAILED);
      return new BinBranchingChoice<BinLookahead>(*this, best_br);
    }

    virtual GC::ExecStatus commit(GC::Space& home, const GC::Choice& c,
                                  const unsigned branch) noexcept {
      BaseBrancher::commit<ModSpace>(home, c, branch);
      [[maybe_unused]] ModSpace* m = &(static_cast<ModSpace&>(home));
      assert(m->status() == GC::SS_BRANCH);
      typedef BinBranchingChoice<BinLookahead> BrChoice;
      const BrChoice& brc = static_cast<const BrChoice&>(c);
      const BinBranching& br = brc.br;
      const auto& var = br.var;
      const auto& val = br.value;
      assert(var >= 0);
      assert(branch == 0 or branch == 1);
      if ( (branch == 0 and GC::me_failed(x[var].eq(home, val))) or
           (branch == 1 and GC::me_failed(x[var].nq(home, val))) ) {
        return GC::ES_FAILED;
      }
      return GC::ES_OK;
    }

  };

  template <class ModSpace>
  inline void post_la_branching(ModSpace& s, GC::IntVarArgs V,
                                const OP::BRT brt) noexcept {
    GC::Home home = s;
    assert(not home.failed());
    const IntViewArray x(home, V);
    if (brt == OP::BRT::binbr) {
      BinLookahead<ModSpace>::post(home, x);
    }
    else if (brt == OP::BRT::enumbr) {
      // XXX
    }
  }

}

#endif
