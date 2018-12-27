// Oliver Kullmann, 6.7.2018 (Swansea)
/* Copyright 2018 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/* TODOS

1. Add parallelisation

   1. Simply using powers of 2 as the number of parallel computations, and
      parallelising by depth accordingly, that is the basic idea.

      Basic code:

        #include <future>

        ACLS G(F); G.set(bv, false);
        auto handle0 = std::async(std::launch::async, [&G, this]{ return this->operator()(std::move(G)); });
        F.set(bv, true);
        const Statistics stats1 = operator()(std::move(F));
        const Statistics stats0 = handle0.get();

        stats.solutions += stats0.solutions + stats1.solutions;

      In Programming/Teaching/PhilosophicalCpp/Part02_ThreadParallelism
      examples for implementing parallelisms are given.

   2. Currently the two branches actually have to be performed in order,
      due to
              const auto right_index = T.index()+1;
      The main problem here is the access to the global variable T, which
      needed synchronisation.
   3. So first the output of the tree-information needs to be improved/changed,
      so that parallel construction becomes possible.
   4. Likely it is best to introduce a policy-argument (a template parameter)
      about parallelisation. If turned on, then we assume that only relatively
      small trees are handled, and then a different tree-handling might be
      used, which uses a traditional node-based approach, which is inherently
      parallelisable.

2. Collecting data

   1. We have summary-statistics, handled by class Statistics.
   2. We should also output the command-line parameters, which likely should
      be the responsibility of ExpQueens.cpp. For the different heuristics
      and active clause-sets, we also need identifying abbreviations.
   3. We need also some summary on inferences.
   4. In general, the active clause-set should provide the "measure"-object,
      the branching-function should provide the "distance"-object.
   5. The summary-statistics considers these numbers, and perhaps always
      shows a standard statistical evaluation:
       min, max, arithmetical mean, standard deviation.

*/

#ifndef BACKTRACKING_Abfe1fM3Q7
#define BACKTRACKING_Abfe1fM3Q7

#include <utility>
#include <ostream>
#include <algorithm>
#include <type_traits>

#include <cmath>
#include <cassert>

#include "ChessBoard.hpp"
#include "Trees.hpp"

namespace Backtracking {

  template <class T>
  struct Statistics : T {
    using Count_t = ChessBoard::Count_t;
    using Var_uint = ChessBoard::Var_uint;
    Count_t solutions;
    Count_t nodes;
    Var_uint height;
    Count_t maxusat_nodes; // maximum size of subtree with unsatisfiable root
    Count_t maxsat_nodes; // maximum size of subtree with no unsatisfiable node
    Var_uint hs;
  };
  template <class T>
  inline constexpr Statistics<T> operator +(const Statistics<T>& s1, const Statistics<T>& s2) noexcept {
    Statistics<T> s{};
    s.solutions = s1.solutions + s2.solutions;
    s.nodes = 1 + s1.nodes + s2.nodes;
    s.height = std::max(s1.height, s2.height) + 1;
    if (s1.solutions == 0 and s2.solutions == 0)
      s.maxusat_nodes = 1 + s1.maxusat_nodes + s2.maxusat_nodes;
    else
      s.maxusat_nodes = std::max(s1.maxusat_nodes, s2.maxusat_nodes);
    if (s1.maxusat_nodes == 0 and s2.maxusat_nodes == 0)
      s.maxsat_nodes = 1 + s1.maxsat_nodes + s2.maxsat_nodes;
    else
      s.maxsat_nodes = std::max(s1.maxsat_nodes, s2.maxsat_nodes);
    if (s1.hs == s2.hs) s.hs = s1.hs + 1;
    else s.hs = std::max(s1.hs, s2.hs);
    if constexpr (not std::is_empty_v<T>) s.combine(s1, s2);
    return s;
  }
  template <class T>
  inline constexpr Statistics<T> satstats(const typename Statistics<T>::Var_uint n, const typename Statistics<T>::Var_uint nset) noexcept {
    return {{}, typename Statistics<T>::Count_t(std::pow(2, n - nset)), 1, 0, 0, 1, 0};
  }
  template <class T>
  inline constexpr Statistics<T> unsatstats(T v = T()) noexcept {
    return {v, 0, 1, 0, 1, 0, 0};
  }
  template <class T>
  std::ostream& operator <<(std::ostream& out, const Statistics<T>& stats) {
    out <<
         "c solutions                             " << stats.solutions << "\n"
         "c nodes                                 " << stats.nodes << "\n"
         "c height                                " << stats.height << "\n"
         "c max_unodes                            " << stats.maxusat_nodes << "\n"
         "c max_snodes                            " << stats.maxsat_nodes << "\n"
         "c HortonStrahler                        " << stats.hs << "\n"
         "c q=leaves/sols                         " << std::defaultfloat << double(stats.nodes+1) / 2 / (stats.solutions) << "\n";
         if constexpr (not std::is_empty_v<T>) stats.output(out);
    return out;
  }


  /* The concept of class X as UNSAT-test is:
   - static function test(const Board&) returns the decision-statistics,
     which is convertible to bool, being true if unsat was detected.
   - It the class contains no data, then indeed it is considered as no-op.
   - The decision-statistics-object can be output via member-function
     output.
   - combine(X,X) takes the statistics-objects from the left and right branch,
     and stores the combination-result in the object.
  */
  // Empty prototype for class providing additional unsat-tests:
  struct EmptyUSAT {
    //static bool test(const ChessBoard::Board&) noexcept { return false; }
    //void combine(EmptyUSAT, EmptyUSAT) noexcept {};
    //std::ostream& output(std::ostream&) const;
  };
  static_assert(std::is_empty_v<EmptyUSAT>);
  static_assert(std::is_pod_v<Statistics<EmptyUSAT>>);

  // Simplest case of additional unsat-test: not enough (anti)diagonals:
  struct NotEnoughDiags {
    using Count_t = ChessBoard::Count_t;
    using Var_uint = ChessBoard::Var_uint;
    Count_t diag_unsat_count;
    NotEnoughDiags() : diag_unsat_count(0) {}
    NotEnoughDiags(const bool b) : diag_unsat_count(b) {}
    static bool test(const ChessBoard::Board& B) noexcept {
      const auto N = B.N - B.t_rank().p;
      {Var_uint open_d = 0;
       for (const auto r : B.d_rank()) {
         if (r.o != 0) ++open_d;
         if (open_d >= N) goto antidiag;
       }
       return false;
      }
      antidiag :
      Var_uint open_ad = 0;
      for (const auto r : B.ad_rank()) {
        if (r.o != 0) ++open_ad;
        if (open_ad >= N) return false;
      }
      return true;
    }
    void combine(const NotEnoughDiags n1, const NotEnoughDiags n2) noexcept {
      diag_unsat_count = n1.diag_unsat_count + n2.diag_unsat_count;
    }
    void output(std::ostream& out) const {
      out << "c not_enough_diags                      " << diag_unsat_count << "\n";
    }
  };
  static_assert(not std::is_empty_v<NotEnoughDiags>);


  template <class ActiveClauseSet, class Branching_t, class Tree_t = Trees::NoOpTree, class USAT_test = EmptyUSAT, class Statistics_t = Backtracking::Statistics<USAT_test>>
  struct CountSat {
    Tree_t T;

    using ACLS = ActiveClauseSet;
    using Branching = Branching_t;
    using Tree = Tree_t;
    using USAT = USAT_test;
    using Statistics = Statistics_t;

    using coord_t = typename ACLS::coord_t;
    using Var = typename ACLS::Var;

    CountSat() = default;

    CountSat(const coord_t N) { Branching::init(N); }
    template <class Binit>
    CountSat(Binit bi) {
      Branching::init(bi);
    }
    template <class Binit>
    CountSat(const coord_t N, const Binit& bi) {
      Branching::init(N,bi);
    }

    Statistics operator()(ACLS F) {
      const auto root_info = T.root_info();
      using NT = Trees::NodeType;
      if (F.satisfied()) {
        T.add(root_info, NT::sl);
        return satstats<USAT_test>(F.n(), F.nset());
      }
      if (F.falsified()) {
        T.add(root_info, NT::ul);
        return unsatstats<USAT_test>();
      }
      if constexpr (not std::is_empty_v<USAT>) {
        const auto usat_test{USAT::test(F.board())};
        if (usat_test) return unsatstats<USAT_test>(usat_test);
      }
      const Var bv = Branching(F)();
      assert(not ChessBoard::singular(bv));

      ACLS G(F); G.set(bv, false);
      const Statistics stats0 = operator()(std::move(G));
      F.set(bv, true);
      const auto after_left_info = T.after_left_info(root_info);
      const Statistics stats1 = operator()(std::move(F));

      T.add(root_info, after_left_info,
        (stats0.solutions == 0 and stats1.solutions == 0) ? NT::ui : NT::si);
      return stats0 + stats1;
    }

  };

}

#endif
