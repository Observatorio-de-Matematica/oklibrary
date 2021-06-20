// Oleg Zaikin, 5.4.2021 (Irkutsk)
/* Copyright 2021 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

    An implementation of look-ahead for the Gecode library.

 TODOS:

-1. Provide overview on functionality provided.
    - Also each function/class needs at least a short specification.

0. Proper names for variables.
    - According to Gecode, now 'failed leaves' and 'failed branches' are used.
    - One variant is to call them 'unsatisfiable leaves' and 'unsatisfiable
      branches' instead of it.
    - Similarly for 'leaves with solutions' and 'branches with solutions':
      'satisfiable leaves' and 'satisfiable branches'.

1. DONE (variables and values were renamed)
   Proper naming of variables and values
    - DONE ('pos' was replaced by 'var' that reflects the Gecode variable)
      "pos" etc. says nothing about the meaning.
    - DONE (shorter names (binmin, narymin, and naryla) are now used.
      The names of the enumeration Braching0 are far too long (and
      appear "random").

2. Statistics are urgently needed.
    - Basic statistics (number of nodes, inner nodes, failed leaves,
      and solutions) is now calculated if look-ahead branching is used.
    - More statistics will be added soon.

3. Four levels of LA-reduction:
    - Level 0 : (if "DONE", then how?? documentation is needed)
     - no explicit reduction;
     - for every branching unsatisfiable branches are just removed;
     - if a branching of width 0 is created, the problem is (immediately)
       recognised as unsatisfiable;

       In NaryLookahead::choice(Space&), there one finds "report that the
       current branch is failed": likely this refers to the *branching*?
       And what is "the first failed value"? There should be *no* value here.
     - if a branching of width 1 is created, then this branching, as a single-
       child-branching, is immediately excecuted.
    - Level 1 :
     - still no explicit reduction;
     - additionally to level 1, if in a considered branching a branch is found
       unsatisfiable, then the corresponding restriction is applied to the
       current instance (globally).
    - Level 2 :
     - now there is an explicit reduction, which applies the reductions found
       by falsified branches until a fixed-point is reached;
     - then in the separate branching function no checks for unsatisfiability
       etc. are done (since no such case can occur).
    - Level 3 :
     - additionally to level 2, now in a considered branching also the
       intersection of the branches is considered for a common reduction.

4. DONE (Yes, our policy is now passing Gecode::IntVarArray by copy)
   Is it appropriate to pass Gecode::IntVarArray by copy?
   ??? What is the solution?

  - Copying would only be appropriate if the internal data stored
    is just a pointer, plus a bit more data.
  - The base class Gecode::VarArray
      https://www.gecode.org/doc-latest/reference/kernel_2data_2array_8hpp_source.html
    seems to contain exactly two data-members:
      int n;
      Var* x;
  - Thus possibly copying is safe and efficient.

5. DONE (Early abortion of a problem is not possible in choice(GC::Space& home))
   Call of status() likely needs a check for early abortion.
   ??? When "done", then how??
   - In case of early abortion of a problem, the function choice(GC::Space& home)
     is not called by Gecode's search engine, where home is the current problem.
   - This is checked by asserting that the problem status is SS_BRANCH.

6. DONE (Enumerations BrTypeO and BrSourceO are parsed via Environment)
   Use Environment to parse enumeration from argv.

7. Generate examples for which tree sizes when using look-ahead are known.
  - It will allow checking correctness of the look-ahead implementation.
  - By now correctness is checked by hand on several small examples:
      Trivial::Sum; Send-more-money; Send-most-money.

8. Later: we don't want to handle variables, but branchers.
  - We can't restrict to just branching on values.
  - We want access to the given possibilities for branching.

9. Later: general concept of a distance.
  - Now distance is a delta of measures.
  - A general concept of a distance should be properly supported.
  - A distance can be handled as a function of two arguments dist(F,F').

*/

#ifndef LOOKAHEAD_lNFKYYpHQ8
#define LOOKAHEAD_lNFKYYpHQ8

#include <iostream>
#include <limits>
#include <vector>
#include <memory>
#include <functional>

#include <cmath>
#include <cassert>

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/gist.hh>

#include <Numerics/FloatingPoint.hpp>
#include <Numerics/Tau.hpp>

namespace Lookahead {

  namespace FP = FloatingPoint;
  namespace GC = Gecode;

  typedef unsigned size_t;
  typedef FP::float80 float_t;
  typedef std::uint64_t count_t;
  typedef GC::Int::IntView IntView;
  typedef GC::ViewArray<IntView> IntViewArray;
  typedef GC::IntVarValues IntVarValues;
  typedef std::vector<int> values_t;
  typedef std::vector<float_t> tuple_t;
  typedef std::function<float_t(const GC::IntVarArray)> measure_t;

  enum class BrTypeO {mind=0, la=1};
  enum class BrSourceO {eq=0, v=1};
  enum class BrMeasureO {mu0=0, mu1=1};
  enum class BrSolutionO {one=0, all=1};

  constexpr char sep = ',';
  typedef std::tuple<BrTypeO, BrSourceO, BrMeasureO, BrSolutionO> option_t;

  std::ostream& operator <<(std::ostream& out, const BrTypeO brt) {
    switch (brt) {
    case BrTypeO::la : return out << " the best (according to look-ahead) branching is chosen";
    default : return out << " a variable with minimal domain size is chosen for branching";}
  }
  std::ostream& operator <<(std::ostream& out, const BrSourceO brsrs) {
    switch (brsrs) {
    case BrSourceO::eq : return out << " for variable var and the minimal value minval the "
                                    << "branching is (var==minval, var!=minval) ";
    default : return out << " for variable var and the domain values {val1,...,valk} "
                         << "the branching is (var==val1,..., var=valk)";}
  }
  std::ostream& operator <<(std::ostream& out, const BrMeasureO brm) {
    switch (brm) {
    case BrMeasureO::mu0 : return out << " measure instance by mu0";
    default : return out << " measure instance by mu1";}
  }
  std::ostream& operator <<(std::ostream& out, const BrSolutionO brsln) {
    switch (brsln) {
    case BrSolutionO::all : return out << " all solutions";
    default : return out << " one solution";}
  }

  inline bool show_usage(const Environment::ProgramInfo proginfo,
                         const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg << " [branching-options] [visual]\n\n" <<
    " branching-options : " << Environment::WRP<BrTypeO>{} << "\n"
    "                   : " << Environment::WRP<BrSourceO>{} << "\n"
    "                   : " << Environment::WRP<BrMeasureO>{} << "\n"
    "                   : " << Environment::WRP<BrSolutionO>{} << "\n"
    " visual            : \"gist\" (run Gist to visualise the search tree).\n\n"
    " solves a given CP-problem via Gecode solvers and given branching options.\n";
    return true;
  }

struct SearchStat {
    count_t nodes;
    count_t inner_nodes;
    count_t failed_leaves;
    count_t solutions;
    GC::Search::Statistics engine;
    option_t br_options;

    SearchStat() : nodes(0), inner_nodes(0), failed_leaves(0),
                   solutions(0), engine(), br_options() {}

    bool valid() const noexcept {
      return (failed_leaves + solutions + inner_nodes == nodes);
    }

    void reset() noexcept {
      assert(valid());
      nodes = inner_nodes = failed_leaves = solutions = 0;
    }

    void update_nodes() noexcept {
      nodes = inner_nodes + failed_leaves + solutions;
    }

    friend bool operator==(const SearchStat& lhs, const SearchStat& rhs) noexcept {
      return lhs.nodes == rhs.nodes and lhs.inner_nodes == rhs.inner_nodes and
             lhs.failed_leaves == rhs.failed_leaves and lhs.solutions == rhs.solutions;
    }

    void print() const noexcept {
      assert(valid());
      const BrTypeO brt = std::get<BrTypeO>(br_options);
      const BrSourceO brsrc = std::get<BrSourceO>(br_options);
      const BrMeasureO brm = std::get<BrMeasureO>(br_options);
      const BrSolutionO brsln = std::get<BrSolutionO>(br_options);

      using std::setw;
      const auto w = setw(10);
      if (brt == BrTypeO::la) std::cout << nodes << w << inner_nodes << w << failed_leaves;
      else std::cout << engine.node << w << engine.fail;
      std::cout << w << solutions << w << int(brt) << w << int(brsrc) << w << int(brm ) << w
                << int(brsln) << "\n";
    }
  };

  SearchStat global_stat;

  inline constexpr size_t tr(const int size, [[maybe_unused]] const size_t bound = 0) noexcept {
    assert(bound <= std::numeric_limits<int>::max());
    assert(size >= int(bound));
    return size;
  }


  inline float_t mu0(const GC::IntVarArray& V) noexcept {
    float_t s = 0;
    for (const auto& v : V) {
      const auto is = tr(v.size(), 1);
      s += is - 1;
    }
    return s;
  }

  inline float_t mu1(const GC::IntVarArray& V) noexcept {
    float_t s = 0;
    for (const auto& v : V) {
      const auto is = tr(v.size(), 1);
      s += FloatingPoint::log2(is);
    }
    return s;
  }

  template<class ModSpace>
  std::shared_ptr<ModSpace> subproblem(ModSpace* const m, const int v, const int val) noexcept {
    assert(m->valid());
    assert(m->valid(v));
    assert(m->status() == GC::SS_BRANCH);
    // Clone space:
    std::shared_ptr<ModSpace> c(static_cast<ModSpace*>(m->clone()));
    assert(c->valid());
    assert(c->valid(v));
    assert(c->status() == GC::SS_BRANCH);
    // Add an equality constraint for the given variable and its value:
    GC::rel(*(c.get()), (c.get())->at(v), GC::IRT_EQ, val);
    return c;
  }

  enum class BrStatus { failed=0, solved=1, branch=2 };

  template <class CustomisedBrancher>
  struct Branching : public GC::Choice {
    int var;
    values_t values;
    BrStatus status;

    bool valid() const noexcept {
      return var >= 0 and ((status == BrStatus::failed and values.empty()) or
                           (status != BrStatus::failed and not values.empty()));
    }

    Branching(const CustomisedBrancher& b, const int v, const values_t vls,
              const BrStatus st)
      : GC::Choice(b, vls.size()==0 ? 1 : vls.size()), var(v), values(vls),
        status(st) {
      assert(valid());
    }

    virtual void archive(GC::Archive& e) const {
      assert(valid());
      GC::Choice::archive(e);
      size_t width = values.size();
      assert(width > 0);
      e << width << var << int(status);
      for (auto& v : values) e << v;
      assert(tr(e.size()) == width + 3);
    }

  };

  // A customised brancher for finding all solutions. Branchings are formed
  // by assigning all possible values to all unassigned variables. A branching
  // with minimal domain size is chosen as the best branching.
  class MinDomValue : public GC::Brancher {
    IntViewArray x;
    mutable int start;

    static bool valid(const IntViewArray x) noexcept { return x.size() > 0; }
    static bool valid(const int s, const IntViewArray x) noexcept {
      return s >= 0 and valid(x) and s < x.size();
    }

  public:

    bool valid() const noexcept { return valid(start, x); }

    MinDomValue(const GC::Home home, const IntViewArray& x)
      : GC::Brancher(home), x(x), start(0) { assert(valid(start, x)); }
    MinDomValue(GC::Space& home, MinDomValue& b)
      : GC::Brancher(home,b), start(b.start) {
      assert(valid(b.x));
      x.update(home, b.x);
      assert(valid(start, x));
    }

    static void post(GC::Home home, const IntViewArray& x) {
      new (home) MinDomValue(home, x);
    }
    virtual GC::Brancher* copy(GC::Space& home) {
      return new (home) MinDomValue(home, *this);
    }
    virtual bool status(const GC::Space&) const {
      assert(valid(start, x));
      for (auto i = start; i < x.size(); ++i)
        if (not x[i].assigned()) { start = i; return true; }
      return false;
    }

    virtual GC::Choice* choice(GC::Space&) {
      assert(valid(start, x));
      int var = start;
      size_t width = tr(x[var].size());
      assert(width > 0);
      for (int i = start + 1; i < x.size(); ++i)
        if (not x[i].assigned() and x[i].size() < width) {
          var = i; width = tr(x[var].size());
          assert(width > 0);
        }
      assert(var >= start and var >= 0);
      values_t values;
      for (GC::Int::ViewValues i(x[var]); i(); ++i)
        values.push_back(i.val());
      assert(not values.empty());
      ++global_stat.inner_nodes;
      return new Branching<MinDomValue>(*this, var, values, BrStatus::branch);
    }
    virtual GC::Choice* choice(const GC::Space&, GC::Archive& e) {
      assert(valid(start, x));
      size_t width; int var;
      assert(e.size() >= 3);
      e >> width >> var;
      assert(width > 0 and var >= 0);
      assert(tr(e.size()) == width + 2);
      int v; values_t values;
      for (size_t i = 0; i < width; ++i) {
        e >> v; values.push_back(v);
      }
      assert(var >= 0 and not values.empty());
      return new Branching<MinDomValue>(*this, var, values, BrStatus::branch);
    }

    virtual GC::ExecStatus commit(GC::Space& home, const GC::Choice& c,
                                  const unsigned branch) {
      typedef Branching<MinDomValue> Branching;
      const Branching& br = static_cast<const Branching&>(c);
      assert(br.valid());
      const auto& values = br.values;
      const auto var = br.var;
      assert(br.status == BrStatus::branch);
      assert(var >= 0 and not values.empty());
      assert(branch < values.size());
      // Failed leaf:
      if (GC::me_failed(x[var].eq(home, values[branch]))) {
        ++global_stat.failed_leaves;
        return GC::ES_FAILED;
      }
      // Execute branching:
      return GC::ES_OK;
    }

  };


  // A customised brancher for finding all solutions. Branchings are formed
  // by assigning all possible values to all unassigned variables. The best
  // branching is chosen via the tau-function.
  template <class ModSpace>
  class ValueLookaheadAllSln : public GC::Brancher {
    IntViewArray x;
    mutable int start;
    measure_t measure;

    static bool valid(const IntViewArray x) noexcept { return x.size() > 0; }
    static bool valid(const int s, const IntViewArray x) noexcept {
      return s >= 0 and valid(x) and s < x.size();
    }

  public:

    bool valid() const noexcept { return valid(start, x); }

    ValueLookaheadAllSln(const GC::Home home, const IntViewArray& x,
      const measure_t measure) : GC::Brancher(home), x(x), start(0), measure(measure) {
    assert(valid(start, x)); }
    ValueLookaheadAllSln(GC::Space& home, ValueLookaheadAllSln& b)
      : GC::Brancher(home,b), start(b.start), measure(b.measure) {
      assert(valid(b.x));
      x.update(home, b.x);
      assert(valid(start, x));
    }

    static void post(GC::Home home, const IntViewArray& x, const measure_t measure) {
      new (home) ValueLookaheadAllSln(home, x, measure);
    }
    virtual GC::Brancher* copy(GC::Space& home) {
      return new (home) ValueLookaheadAllSln(home, *this);
    }
    virtual bool status(const GC::Space&) const {
      assert(valid(start, x));
      for (auto i = start; i < x.size(); ++i)
        if (not x[i].assigned()) { start = i; return true; }
      return false;
    }

    virtual GC::Choice* choice(GC::Space& home) {
      assert(valid(start, x));
      assert(start < x.size());
      float_t ltau = FP::pinfinity;
      int var = start;
      values_t values;
      BrStatus status = BrStatus::branch;

      ModSpace* m = &(static_cast<ModSpace&>(home));
      assert(m->status() == GC::SS_BRANCH);

      const auto msr = measure(m->at());

      // For remaining variables (all before 'start' are assigned):
      for (int v = start; v < x.size(); ++v) {
        // v is a variable, view is the values in Gecode format:
        const IntView view = x[v];
        // Skip assigned variables:
        if (view.assigned()) continue;
        assert(view.size() >= 2);
        tuple_t tuple; values_t vls;
        // For all values of the current variable:
        for (IntVarValues j(view); j(); ++j) {
          // Assign value, propagate, and measure:
          const int val = j.val();
          auto c = subproblem<ModSpace>(m, v, val);
          auto subm = c.get();
          auto subm_st = c.get()->status();
          // Skip failed branches:
          if (subm_st != GC::SS_FAILED) {
            // Calculate delta of measures:
            float_t dlt = msr - measure(subm->at());
            assert(dlt > 0);
            vls.push_back(val);
            if (subm_st == GC::SS_SOLVED) status = BrStatus::solved;
            else tuple.push_back(dlt);
          }
        }
        // If branching of width 1, immediately execute:
        if (tuple.size() == 1) {
          assert(not vls.empty());
          var = v; values = vls; break;
        }
        // If branching of width 0, the problem is unsatisfiable:
        else if (tuple.empty() and vls.empty()) {
          var = v; values = vls; status = BrStatus::failed; break;
        }
        // If all subproblems are satisfiable, count solutions:
        else if (tuple.empty() and not vls.empty()) {
          var = v; values = vls; status = BrStatus::solved; break;
        }
        // Calculate ltau and update the best value if needed:
        const float_t lt = Tau::ltau(tuple);
        if (lt < ltau) {
          var = v; values = vls; ltau = lt;
        }
      }
      if (status != BrStatus::failed) ++global_stat.inner_nodes;
      assert(var >= 0 and var >= start);
      assert(not x[var].assigned());
      assert((status == BrStatus::failed and values.empty()) or
             (status != BrStatus::failed and not values.empty()));
      return new Branching<ValueLookaheadAllSln>(*this, var, values, status);
    }

    virtual GC::Choice* choice(const GC::Space&, GC::Archive& e) {
      assert(valid(start, x));
      size_t width; int var;
      assert(e.size() >= 3);
      size_t st;
      e >> width >> var >> st;
      assert(var >= 0);
      BrStatus status = static_cast<BrStatus>(st);
      assert(var >= 0);
      assert((status == BrStatus::failed and width == 0) or
             (status != BrStatus::failed and width > 0));
      int v; values_t values;
      for (size_t i = 0; i < width; ++i) {
        e >> v; values.push_back(v);
      }
      assert(var >= 0);
      return new Branching<ValueLookaheadAllSln>(*this, var, values, status);
    }

    virtual GC::ExecStatus commit(GC::Space& home, const GC::Choice& c,
                                  const unsigned branch) {
      typedef Branching<ValueLookaheadAllSln> Branching;
      const Branching& br = static_cast<const Branching&>(c);
      assert(br.valid());
      const auto status = br.status;
      const auto var = br.var;
      const auto& values = br.values;
      assert(status == BrStatus::failed or branch < values.size());
      // If failed branching, stop executing:
      if (status == BrStatus::failed or
          GC::me_failed(x[var].eq(home, values[branch]))) {
        ++global_stat.failed_leaves;
        return GC::ES_FAILED;
      }
      // Execute branching:
      return GC::ES_OK;
    }

  };

  template <class ModSpace>
  inline void post_branching(GC::Home home, const GC::IntVarArgs& V,
                             const option_t options) noexcept {
    assert(not home.failed());
    const BrTypeO brt = std::get<BrTypeO>(options);
    const BrSourceO brsrc = std::get<BrSourceO>(options);
    const BrMeasureO brm = std::get<BrMeasureO>(options);
    const BrSolutionO brsln = std::get<BrSolutionO>(options);
    const IntViewArray y(home, V);
    if (brt == BrTypeO::mind) {
      if (brsrc == BrSourceO::eq)
        GC::branch(home, V, GC::INT_VAR_SIZE_MIN(), GC::INT_VAL_MIN());
      else if (brsrc == BrSourceO::v and brsln == BrSolutionO::one) {
        //MinDomValueOneSln::post(home, y);
      }
      else if (brsrc == BrSourceO::v and brsln == BrSolutionO::all) {
        MinDomValue::post(home, y);
      }
    }
    else if (brt == BrTypeO::la) {
      measure_t measure = (brm == BrMeasureO::mu0) ? mu0 : mu1;
      if (brsrc == BrSourceO::eq) {
        // XXX
      }
      else if (brsrc == BrSourceO::v and brsln == BrSolutionO::one) {
        // XXX
      }
      else if (brsrc == BrSourceO::v and brsln == BrSolutionO::all) {
        ValueLookaheadAllSln<ModSpace>::post(home, y, measure);
      }
    }
  }

  template <class ModSpace>
  void find_all_solutions(const std::shared_ptr<ModSpace> m,
                                const bool print = false) noexcept {
    assert(m->valid());
    typedef std::shared_ptr<ModSpace> node_ptr;
    GC::DFS<ModSpace> e(m.get());
    while (const node_ptr s{e.next()}) {
      if (print) s->print();
      ++global_stat.solutions;
    }
    global_stat.engine = e.statistics();
  }
  template <class ModSpace>
  void find_one_solution(const std::shared_ptr<ModSpace> m,
                                const bool print = false) noexcept {
    assert(m->valid());
    typedef std::shared_ptr<ModSpace> node_ptr;
    GC::DFS<ModSpace> e(m.get());
    if (const node_ptr s{e.next()}) {
      if (print) s->print();
      ++global_stat.solutions;
    }
    global_stat.engine = e.statistics();
  }
  template <class ModSpace>
  SearchStat solve(const std::shared_ptr<ModSpace> m,
                   const bool print = false) noexcept {
    assert(m->valid());
    global_stat.reset();
    global_stat.br_options = m->branching_options();
    auto const st = m->status();
    if (st == GC::SS_FAILED) global_stat.failed_leaves = 1;
    const option_t options = m->branching_options();
    const BrSolutionO brsln = std::get<BrSolutionO>(options);
    switch (brsln) {
    case BrSolutionO::all : find_all_solutions(m, print); break;
    default : find_one_solution(m, print);}
    global_stat.update_nodes();
    if (print) global_stat.print();
    return global_stat;
  }

  template <class ModSpace>
  void visualise(const std::shared_ptr<ModSpace> m) noexcept {
    assert(m->valid());
    GC::Gist::Print<ModSpace> p("Print solution");
    GC::Gist::Options o;
    o.inspect.click(&p);
    GC::Gist::dfs(m.get(),o);
  }

}

namespace Environment {
  template <>
  struct RegistrationPolicies<Lookahead::BrTypeO> {
    static constexpr int size = int(Lookahead::BrTypeO::la)+1;
    static constexpr std::array<const char*, size> string
    {"mind", "la"};
  };
  template <>
  struct RegistrationPolicies<Lookahead::BrSourceO> {
    static constexpr int size = int(Lookahead::BrSourceO::v)+1;
    static constexpr std::array<const char*, size> string
    {"eq", "v"};
  };
  template <>
  struct RegistrationPolicies<Lookahead::BrMeasureO> {
    static constexpr int size = int(Lookahead::BrMeasureO::mu1)+1;
    static constexpr std::array<const char*, size> string
    {"mu0", "mu1"};
  };
  template <>
  struct RegistrationPolicies<Lookahead::BrSolutionO> {
    static constexpr int size = int(Lookahead::BrSolutionO::all)+1;
    static constexpr std::array<const char*, size> string
    {"one", "all"};
  };
}

#endif
