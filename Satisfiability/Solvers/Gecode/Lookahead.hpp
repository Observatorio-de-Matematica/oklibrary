// Oleg Zaikin, 5.4.2021 (Irkutsk)
/* Copyright 2021 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

    Lookahead for the Gecode library

 TODOS:

1. Is it appropriate to pass Gecode::IntVarArray by copy?

  - Copying would only be appropriate if the internal data stored
    is just a pointer, plus a bit more data.
  - The base class Gecode::VarArray
      https://www.gecode.org/doc-latest/reference/kernel_2data_2array_8hpp_source.html
    seems to contain exactly two data-members:
      int n;
      Var* x;
  - Thus possibly copying is safe and efficient.

*/

#ifndef LOOKAHEAD_lNFKYYpHQ8
#define LOOKAHEAD_lNFKYYpHQ8

#include <iostream>
#include <limits>
#include <vector>
#include <memory>

#include <cmath>
#include <cassert>

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/gist.hh>

#include <Numerics/FloatingPoint.hpp>

namespace Lookahead {

  namespace FP = FloatingPoint;
  namespace GC = Gecode;

  typedef unsigned size_t;
  typedef FP::float80 float_t;
  typedef std::uint64_t count_t;
  typedef GC::ViewArray<GC::Int::IntView> IntView;
  typedef std::vector<int> values_t;

  inline bool show_usage(const Environment::ProgramInfo proginfo,
                         const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg << " [visual]\n\n" <<
    "visual    : \"-gist\" (run Gist to visualise the search tree).\n\n";
    return true;
  }

  inline constexpr size_t tr(const int size, [[maybe_unused]] const size_t bound = 0) noexcept {
    assert(bound <= std::numeric_limits<int>::max());
    assert(size >= int(bound));
    return size;
  }

  inline float_t mu0(const GC::IntVarArray V) noexcept {
    float_t s = 0;
    for (const auto& v : V) {
      const auto is = tr(v.size(), 1);
      s += is - 1;
    }
    return s;
  }

  inline float_t mu1(const GC::IntVarArray V) noexcept {
    float_t s = 0;
    for (const auto& v : V) {
      const auto is = tr(v.size(), 1);
      s += FloatingPoint::log2(is);
    }
    return s;
  }

  template <class NaryBrancher>
  struct VarVal : public GC::Choice {
    int pos;
    values_t values;

    inline bool valid(const values_t v) const noexcept {
      return not v.empty();
    }
    inline bool valid(const int p, const values_t v) const noexcept {
      return p >= 0 and valid(v);
    }
    inline bool valid() const noexcept {
      return valid(pos, values);
    }

    VarVal(const NaryBrancher& b, const int p, const values_t V)
      : GC::Choice(b, V.size()), pos(p), values(V) { assert(valid(pos, values)); }

    virtual void archive(GC::Archive& e) const {
      assert(valid(pos, values));
      GC::Choice::archive(e);
      size_t width = values.size();
      assert(width > 0);
      e << width << pos;
      for (auto v : values) e << v;
      assert(tr(e.size()) == width + 2);
    }

  };

  class NarySizeMin : public GC::Brancher {
    IntView x;
    mutable int start;

    inline bool valid(const IntView x) const noexcept {
      return x.size() > 0;
    }
    inline bool valid(const int s, const IntView x) const noexcept {
      return s >= 0 and valid(x) and s < x.size();
    }

  public:

    inline bool valid() const noexcept {
      return valid(start, x);
    }

    NarySizeMin(const GC::Home home, const IntView& x)
      : GC::Brancher(home), x(x), start(0) { assert(valid(start, x)); }
    NarySizeMin(GC::Space& home, NarySizeMin& b)
      : GC::Brancher(home,b), start(b.start) {
      assert(valid(b.x));
      x.update(home, b.x);
      assert(valid(start, x));
    }

    static void post(GC::Home home, const IntView& x) {
      new (home) NarySizeMin(home, x);
    }
    virtual GC::Brancher* copy(GC::Space& home) {
      return new (home) NarySizeMin(home, *this);
    }
    virtual bool status(const GC::Space&) const {
      assert(valid(start, x));
      for (auto i = start; i < x.size(); ++i)
        if (not x[i].assigned()) { start = i; return true; }
      return false;
    }

    virtual GC::Choice* choice(GC::Space&) {
      assert(valid(start, x));
      int pos = start;
      size_t width = tr(x[pos].size());
      assert(width > 0);
      for (auto i = start + 1; i < x.size(); ++i)
        if (not x[i].assigned() and x[i].size() < width) {
          pos = i; width = tr(x[pos].size());
          assert(width > 0);
        }
      assert(pos >= start);
      values_t values;
      for (GC::Int::ViewValues i(x[pos]); i(); ++i)
        values.push_back(i.val());
      assert(pos >= 0 and not values.empty());
      return new VarVal<NarySizeMin>(*this, pos, values);
    }
    virtual GC::Choice* choice(const GC::Space&, GC::Archive& e) {
      assert(valid(start, x));
      size_t width; int pos;
      assert(e.size() >= 3);
      e >> width >> pos;
      assert(width > 0 and pos >= 0);
      assert(tr(e.size()) == width + 2);
      int v; values_t values;
      for (size_t i = 0; i < width; ++i) {
        e >> v; values.push_back(v);
      }
      assert(pos >= 0 and not values.empty());
      return new VarVal<NarySizeMin>(*this, pos, values);
    }

    virtual GC::ExecStatus commit(GC::Space& home, const GC::Choice& c,
                                  const unsigned branch) {
      typedef VarVal<NarySizeMin> VarVal;
      const VarVal& pv = static_cast<const VarVal&>(c);
      assert(pv.valid());
      const auto values = pv.values;
      const auto pos = pv.pos;
      assert(pos >= 0 and not values.empty());
      assert(branch < values.size());
      return GC::me_failed(x[pos].eq(home, values[branch])) ?
             GC::ES_FAILED : GC::ES_OK;
    }

  };


  enum class BranchingO { binarysizeminvalmin=0, narysizeminvalmin=1/*, naryla=2*/ };

  inline void post_narysizemin(GC::Home home, const GC::IntVarArgs& x) {
    assert(not home.failed());
    const IntView y(home, x);
    NarySizeMin::post(home, y);
  }

  inline void post_branching(GC::Home home, const GC::IntVarArgs& V,
                             const BranchingO b) noexcept {
    assert(not home.failed());
    switch (b) {
    case BranchingO::binarysizeminvalmin :
      GC::branch(home, V, GC::INT_VAR_SIZE_MIN(), GC::INT_VAL_MIN());
      break;
    case BranchingO::narysizeminvalmin : {
      const IntView y(home, V);
      NarySizeMin::post(home, y);
      break;
    }
    //case BranchingO::naryla :
    //  XXX
    default :
      GC::branch(home, V, GC::INT_VAR_SIZE_MIN(), GC::INT_VAL_MIN());
      break;
    }
  }

  struct SearchStat {
    count_t solutions;
    GC::Search::Statistics engine;

    SearchStat() : solutions(0), engine() {}

    void print() const noexcept {
      using std::setw;
      const auto w = setw(10);
      std::cout << engine.node << w << engine.fail << w << solutions << "\n";
    }
  };

  template <class ModSpace>
  SearchStat find_all_solutions(const std::shared_ptr<ModSpace> m,
                                const bool print = false) noexcept {
    assert(m->valid());
    typedef std::shared_ptr<ModSpace> node_ptr;
    GC::DFS<ModSpace> e(m.get());
    SearchStat stat;
    while (const node_ptr s{e.next()}) {
      if (print) s->print();
      ++stat.solutions;
    }
    stat.engine = e.statistics();
    return stat;
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

#endif
