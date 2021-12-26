// Oleg Zaikin, 22.12.2021 (Swansea)
/* Copyright 2021 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  USAGE:

  EXAMPLES:

MOLS> cat data/weights/testN6 | ./Euler_BBOpt_debug 1 1 1 1 data/weights/ParaN6 val dom
  (1,100,100,100),78


More precisely:

MOLS> cat data/weights/testN6 | /usr/bin/time ./Euler_BBOpt_debug 1 1 1 1 data/weights/ParaN6 val dom
(1,100,100,100),78
18.56user 0.54system 0:19.11elapsed 100%CPU (0avgtext+0avgdata 69928maxresident)k
0inputs+0outputs (0major+16693minor)pagefaults 0swaps

With output of SearchStats:

MOLS> cat data/weights/testN6 | ./Euler_BBOpt 1 1 1 1 data/weights/ParaN6 val dom
2 3 4 5 6 2 12 6 la val one eager prun dom 0.6552 0 253 110 87 14 143 143 0 342 253 4580 46145 0.6511 0.0164 0.1196 0.0000 Euler_BBOpt 0.2.7
1 3 4 5 6 2 12 6 la val one eager prun dom 0.5589 0 221 94 73 10 127 127 0 326 221 3876 38288 0.5552 0.0148 0.0991 0.0000 Euler_BBOpt 0.2.7
100 3 4 5 6 2 12 6 la val one eager prun dom 2.6171 0 1182 390 189 1 792 792 0 1176 1182 25374 182800 2.5993 0.0705 0.4409 0.0000 Euler_BBOpt 0.2.7
1 1 4 5 6 2 12 6 la val one eager prun dom 0.8396 0 369 145 100 12 224 224 0 456 369 6736 58371 0.8337 0.0212 0.1503 0.0000 Euler_BBOpt 0.2.7
1 100 4 5 6 2 12 6 la val one eager prun dom 0.8421 0 223 100 81 16 123 123 0 327 223 4780 57100 0.8383 0.0241 0.1523 0.0000 Euler_BBOpt 0.2.7
1 100 1 5 6 2 12 6 la val one eager prun dom 0.8310 0 218 98 80 15 120 120 0 318 218 4792 57181 0.8272 0.0251 0.1501 0.0000 Euler_BBOpt 0.2.7
1 100 100 5 6 2 12 6 la val one eager prun dom 0.6503 0 204 92 76 13 112 112 0 319 204 3991 44966 0.6471 0.0153 0.1182 0.0000 Euler_BBOpt 0.2.7
1 100 100 1 6 2 12 6 la val one eager prun dom 0.6638 0 216 98 82 13 118 118 0 320 216 4134 46050 0.6604 0.0164 0.1207 0.0000 Euler_BBOpt 0.2.7
1 100 100 100 6 2 12 6 la val one eager prun dom 0.4730 0 132 54 37 11 78 78 0 227 132 2230 30844 0.4706 0.0102 0.0823 0.0000 Euler_BBOpt 0.2.7
(1.0000,100.0000,100.0000,100.0000),78.0000

   Remark: In debug-mode the first line is printed two more times, due to
   asserts checking the result.


TODOS:

1. There is an enormous code-duplication, which needs to be cleaned-up.

BUGS:

0. MEMORY LEAK

See the BUG-report in Euler.cpp.
Most likely the look-ahead-"spaces" are never killed (not during a
solver-run, and also not after a solver-run has finished).

1. Segmentation fault

MOLS> cat ./data/weights/testN7 | ./Euler_BBOpt 10 2 1 1 ./data/weights/ParaN7_3 eq dom
140 260 86 0.1 124 7 2 22 7 la eq one eager prun dom 74.9325 0 4755 2377 2377 0 2378 2378 0 5676 4755 1035149 4374641 74.8384 2.9571 13.8248 0.0000 Euler_BBOpt 0.2.7
Segmentation fault (core dumped)
MOLS> cat ./data/weights/testN7 | ./Euler_BBOpt_debug 10 2 1 1 ./data/weights/ParaN7_3 eq dom
140 260 86 0.1 124 7 2 22 7 la eq one eager prun dom 116.1668 0 4755 2377 2377 0 2378 2378 0 5676 4755 1035149 4374641 116.0501 3.4152 35.0643 0.0000 Euler_BBOpt_debug 0.2.7
Euler_BBOpt_debug: ../../../../Satisfiability/Solvers/Gecode/Lookahead.hpp:1530: Lookahead::LookaheadEq<ModSpace>::LookaheadEq(Gecode::Home, const IntViewArray&, Lookahead::option_t, const vec_t&) [with ModSpace = {anonymous}::TWO_MOLS; Lookahead::IntViewArray = Gecode::ViewArray<Gecode::Int::IntView>; Lookahead::option_t = std::tuple<Lookahead::BrTypeO, Lookahead::BrSourceO, Lookahead::BrSolutionO, Lookahead::BrEagernessO, Lookahead::BrPruneO>; Lookahead::vec_t = std::__debug::vector<long double>]: Assertion `valid(start, x)' failed.
Aborted (core dumped)

Running just Euler:
MOLS> cat ./data/weights/testN7 | ./Euler_debug 0 0 eq "" dom 140,260,86,0.1,124 
N k m1 m2 brt brsrc brsol bregr brpr prp t sat nds inds inds2 inds3 lvs ulvs sol 1chld chcs taus sbps chct taut sbpt ptime prog vers
7 2 22 7 la eq one eager prun dom 115.8517 0 4755 2377 2377 0 2378 2378 0 5676 4755 1035149 4374641 115.7369 3.3979 34.7865 0.0002 Euler_debug 0.11.7

So it seems a problem of the new start; apparently only for eq ?!
Running the above with valgrind shows a lot of undefined behaviour!
The first seems to happen in line 1520 of Lookahead.hpp:
      return s >= 0 and valid(x) and s < x.size();
called from line 1530 (assert in constructor), which in turn is called from
line 1541 (the post-function): the problem is likely the undefined variable
start !


2. Non-parallisable

MOLS> cat data/weights/testN6 | ./Euler_BBOpt_debug 1 1 1 2 data/weights/ParaN6 val dom
Segmentation fault (core dumped)

All the global variables need to be removed.

More details:

MOLS> cat ./data/weights/testN6 | valgrind ./Euler_BBOpt_debug 1 1 1 2 data/weights/Para0 val dom
==15737== Memcheck, a memory error detector
==15737== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==15737== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==15737== Command: ./Euler_BBOpt_debug 1 1 1 2 data/weights/Para0 la,val dom
==15737== 

Euler_BBOpt_debug: ../../../../Satisfiability/Solvers/Gecode/Statistics.hpp:44: void Statistics::SearchStat::reset(): Assertion `valid()' failed.

The problem here is the global variable

  // XXX no global variables in header-files !!! XXX
  Statistics::SearchStat global_stat;

in Lookahead.cpp (line 235).
This needs to be removed.

General remark: Only in very special case should global variables be used.

*/


#include <iostream>
#include <string>
#include <memory>
#include <tuple>
#include <iomanip>

#include <cassert>

#include <ProgramOptions/Environment.hpp>
#include <SystemSpecifics/Timing.hpp>
#include <Numerics/NumInOut.hpp>
#include <Numerics/Optimisation.hpp>
#include <Transformers/Generators/Random/LatinSquares.hpp>
#include <Transformers/Generators/Random/LSRG.hpp>
#include <Solvers/Gecode/Lookahead.hpp>
#include <Solvers/Gecode/Statistics.hpp>

#include "Euler.hpp"

namespace {

  using namespace Euler;

  class TWO_MOLS : public GC::Space {
    const LS::ls_dim_t N;
    const LA::option_t alg_options;
    const gecode_option_t gecode_options;
    const Optimisation::vec_t wghts;
    GC::IntVarArray x, y, z, V;
    inline LA::size_t x_index(const LA::size_t i) const noexcept { return i; }
    inline LA::size_t y_index(const LA::size_t i) const noexcept { return i + LA::tr(x.size()); }
    inline LA::size_t z_index(const LA::size_t i) const noexcept {
      return i + LA::tr(x.size()) + LA::tr(y.size());
    }

    inline GC::IntPropLevel prop_level(const gecode_option_t gc_options) const noexcept {
      GC::IntPropLevel ipl = GC::IPL_DEF;
      const auto gc_option = std::get<PropO>(gc_options);
      switch( gc_option ) {
      case PropO::val:
          ipl = GC::IPL_VAL; break;
      case PropO::bnd:
          ipl = GC::IPL_BND; break;
      case PropO::dom:
          ipl = GC::IPL_DOM; break;
      case PropO::def:
          ipl = GC::IPL_DEF; break;
      default:
          ipl = GC::IPL_DOM; break;
      }
      return ipl;
    }
  public:
    TWO_MOLS(const LS::ls_dim_t N, const LA::option_t alg_options,
             const gecode_option_t gecode_options,
             const gecode_intvec_t ls1_partial = {},
             const gecode_intvec_t ls2_partial = {},
             const Optimisation::vec_t wghts = {}) :
      N(N), alg_options(alg_options), gecode_options(gecode_options),
      x(*this, N*N, 0, N - 1),
      y(*this, N*N, 0, N - 1),
      z(*this, N*N, 0, N - 1),
      V(*this, x.size() + y.size() + z.size(), 0, N - 1) {
      assert(valid());
      // Determine propagation level:
      GC::IntPropLevel prp_lvl = prop_level(gecode_options);
      // Use an umbrella variable array for all variables:
      for (LA::size_t i = 0; i < LA::tr(x.size()); ++i) V[x_index(i)] = x[i];
      for (LA::size_t i = 0; i < LA::tr(y.size()); ++i) V[y_index(i)] = y[i];
      for (LA::size_t i = 0; i < LA::tr(z.size()); ++i) V[z_index(i)] = z[i];
      // Known cells of partially filled Latin squares:
      if (not ls1_partial.empty() and not ls2_partial.empty()) {
        assert(ls1_partial.size() == N*N and ls2_partial.size() == N*N);
        for(LS::ls_dim_t i = 0; i < N; ++i) {
          for(LS::ls_dim_t j = 0; j < N; ++j) {
            assert(i*N + j < ls1_partial.size());
            if (ls1_partial[i*N + j] >= 0)
              dom(*this, x[i*N + j], ls1_partial[i*N + j],
                  ls1_partial[i*N + j], prp_lvl);
            assert(i*N + j < ls2_partial.size());
            if (ls2_partial[i*N + j] >= 0)
              dom(*this, y[i*N + j], ls2_partial[i*N + j],
                  ls2_partial[i*N + j], prp_lvl);
          }
        }
      }

      // Latin property in rows of X:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t rows_x;
        for (LS::ls_dim_t j = 0; j < N; ++j) rows_x.push_back(x[i*N + j]);
        GC::distinct(*this, rows_x, prp_lvl);
      }
      // Latin property in cols of X:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t cols_x;
        for (LS::ls_dim_t j = 0; j < N; ++j) cols_x.push_back(x[j*N + i]);
        GC::distinct(*this, cols_x, prp_lvl);
      }
      // Latin property in rows of Y:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t rows_y;
        for (LS::ls_dim_t j = 0; j < N; ++j) rows_y.push_back(y[i*N + j]);
        GC::distinct(*this, rows_y, prp_lvl);
      }
      // Latin property in cols of Y:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t cols_y;
        for (LS::ls_dim_t j = 0; j < N; ++j) cols_y.push_back(y[j*N + i]);
        GC::distinct(*this, cols_y, prp_lvl);
      }
      // Row uniqueness of Z:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t rows_z;
        for (LS::ls_dim_t j = 0; j < N; ++j) rows_z.push_back(z[i*N + j]);
        GC::distinct(*this, rows_z, prp_lvl);
      }
      // Column uniqueness of Z:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t cols_z;
        for (LS::ls_dim_t j = 0; j < N; ++j) cols_z.push_back(z[j*N + i]);
        GC::distinct(*this, cols_z, prp_lvl);
      }
      // Enforce element constraints on Z, X, Y:
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        gecode_intvarvec_t Zvec_i;
        for (LS::ls_dim_t j = 0; j < N; ++j) Zvec_i.push_back(z[i*N + j]);
        for (LS::ls_dim_t j = 0; j < N; ++j) {
          GC::element(*this, GC::IntVarArgs(Zvec_i), x[i*N + j],
                      y[i*N + j], prp_lvl);
        }
      }

      if (not this->failed()) {
        assert(wghts.size() == N-2);
        LA::post_branching<TWO_MOLS>(*this, V, alg_options, wghts);
      }

    }

    TWO_MOLS(TWO_MOLS& T) : GC::Space(T), N(T.N), alg_options(T.alg_options),
             gecode_options(T.gecode_options), wghts(T.wghts) {
      assert(T.valid());
      x.update(*this, T.x);
      y.update(*this, T.y);
      z.update(*this, T.z);
      V.update(*this, T.V);
      assert(valid(V));
    }
    virtual GC::Space* copy() {
      return new TWO_MOLS(*this);
    }

    inline bool valid () const noexcept {return N > 0 and valid(V);}
    inline bool valid (const GC::IntVarArray V) const noexcept {
      return x.size() > 0 and V.size() == x.size() + y.size() + z.size();
    }
    inline bool valid (const LA::size_t i) const noexcept {return i<LA::tr(V.size());}

    inline GC::IntVar at(const LA::size_t i) const noexcept {
      assert(valid()); assert(valid(i));
      return V[i];
    }
    inline GC::IntVarArray at() const noexcept { assert(valid()); return V; }

    LA::option_t branching_options() const noexcept { assert(valid()); return alg_options; }

    void print() {
      assert(valid());
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        for (LS::ls_dim_t j = 0; j < N; ++j) {
          std::cout << x[i*N + j];
          if (j < N-1) std::cout << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
      for (LS::ls_dim_t i = 0; i < N; ++i) {
        for (LS::ls_dim_t j = 0; j < N; ++j) {
          std::cout << y[i*N + j];
          if (j < N-1) std::cout << " ";
        }
        std::cout << std::endl;
      }
    }
    void print(std::ostream& os) const noexcept {
      assert(valid(V)); os << V << std::endl;
    }

  };

}

namespace {

  const Environment::ProgramInfo proginfo{
        "0.2.8",
        "26.12.2021",
        __FILE__,
        "Oliver Kullmann and Oleg Zaikin",
        "https://github.com/OKullmann/oklibrary/blob/master/Programming/Numerics/Euler_BBOpt.cpp",
        "GPL v3"};

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
      "> "<<proginfo.prg<< " M R S T optFILE [parameters for opt]\n\n"
      " - M = partition-size\n"
      " - R = number of rounds (running through all coordinates once)\n"
      " - S = number of shrinkings (on top of the rounds)\n"
      " - T = number of threads\n"
      " - optFILE : contains a line with 5 numbers per coordinate (allows formatting)\n"
      " - all following arguments are passed to the functor to be optimised.\n"
 ;
    return true;
  }

  namespace LA = Lookahead;

  const std::string error = "ERROR[" + proginfo.prg + "]: ";

  struct Func {
     // Finding an Euler square:
    LS::ls_dim_t N, k, m1, m2;
    LA::option_t alg_options;
    gecode_option_t gecode_options;
    gecode_intvec_t ls1_partial;
    gecode_intvec_t ls2_partial;
    const Timing::UserTime timing;

    void init(const int argc, const char* const argv[]) noexcept {
      Environment::Index index;
      // Read algorithm options:
      alg_options = argc <= index ? LA::option_t{} :
        Environment::translate<LA::option_t>()(argv[index-1], LA::sep);
      // Read gecode options:
      gecode_options = argc <= index ?
        Euler::gecode_option_t{Euler::PropO::dom} :
        Environment::translate<Euler::gecode_option_t>()(argv[index++],
                                                         LA::sep);
      N = read_N(error);
      k = read_k(error);
      assert(N > 0 and k > 0);
      ls1_partial = read_partial_ls(N);
      ls2_partial = read_partial_ls(N);
      assert(not ls1_partial.empty() and not ls2_partial.empty());
      m1 = given_cells(ls1_partial); m2 = given_cells(ls2_partial);
    }

    Optimisation::y_t func(const Optimisation::vec_t& v) noexcept {
      assert(not v.empty());
      assert(v.size() == N-2);
      const std::shared_ptr<TWO_MOLS> p(new TWO_MOLS(N,
        alg_options, gecode_options, ls1_partial, ls2_partial, v));
      const Timing::Time_point t1 = timing();
      const Statistics::SearchStat stat = LA::solve<TWO_MOLS>(p);
      const Timing::Time_point t2 = timing();
      const double solving_time = t2 - t1;
      assert(p.use_count() == 1);
for (const auto x : v) std::cerr << x << " ";
print_stat(N, k, m1, m2, 0, solving_time,
           alg_options, gecode_options, stat, proginfo);
      const auto leaves = stat.solutions + stat.unsat_leaves;
      return leaves;
    }
  };

}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  FloatingPoint::fullprec_float80(std::cout);
  const auto result = Optimisation::bbopt_rounds_app(argc, argv, Func());
  std::cout << result << "\n";
}
