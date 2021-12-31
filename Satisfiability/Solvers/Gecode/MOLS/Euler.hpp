// Oliver Kullmann, 25.12.2021 (Swansea)
/* Copyright 2021 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

TODOS:

1. Remove references to iostream
    - Functions should always take a std::istream& or std::ostream& parameter.

*/

#ifndef EULER_g2rGlQAGDl
#define EULER_g2rGlQAGDl

#include <ostream>
#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include <array>
#include <iomanip>

#include <cstdlib>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/search.hh>

#include <ProgramOptions/Environment.hpp>
#include <Numerics/NumInOut.hpp>
#include <Transformers/Generators/Random/LatinSquares.hpp>
#include <Transformers/Generators/Random/LSRG.hpp>

#include "../Lookahead.hpp"

namespace Euler {

  namespace GC = Gecode;
  namespace LS = LatinSquares;
  namespace RG = RandGen;
  namespace LA = Lookahead;

  typedef std::vector<int> gecode_intvec_t;
  typedef std::vector<GC::IntVar> gecode_intvarvec_t;
  typedef std::vector<std::string> partial_ls_t;

  constexpr LS::ls_dim_t N_default = 0;
  constexpr LS::ls_dim_t k_default = 2;


  // Pgopagation level for Gecode constraints.
  // def: default propagation (can be different for different
  //      constraints).
  // val: value propagation (the fastest and the simplest one).
  // bnd: bound propagation (average speed and performance).
  // dom: domain propagation (the slowest and the most effective).
  enum class PropO {def=0, val=1, bnd=2, dom=3};
  constexpr int PropOsize = 4;

  enum class HeO {show=0, noshow=1};
  constexpr int HeOsize = 2;
  enum class StatO {show=0, noshow=1};
  constexpr int StatOsize = 2;
  enum class SolO {show=0, noshow=1};
  constexpr int SolOsize = 2;
}
namespace Environment {
  template <> struct RegistrationPolicies<Euler::PropO> {
    static constexpr int size = Euler::PropOsize;
    static constexpr std::array<const char*, size> string {"def", "val", "bnd", "dom"};
  };
  template <> struct RegistrationPolicies<Euler::HeO> {
    static constexpr int size = Euler::HeOsize;
    static constexpr std::array<const char*, size> string {"+head", "-head"};
  };
  template <> struct RegistrationPolicies<Euler::StatO> {
    static constexpr int size = Euler::StatOsize;
    static constexpr std::array<const char*, size> string {"+stat", "-stat"};
  };
  template <> struct RegistrationPolicies<Euler::SolO> {
    static constexpr int size = Euler::SolOsize;
    static constexpr std::array<const char*, size> string {"+sol", "-sol"};
  };
}
namespace Euler {
  constexpr char sep = ',';

  typedef std::tuple<PropO> gecode_option_t;
  std::ostream& operator <<(std::ostream& out, const PropO m) {
    switch (m) {
    case PropO::val : return out << "values-prop";
    case PropO::bnd : return out << "bounds-prop";
    case PropO::dom : return out << "domain-prop";
    default : return out << "default-prop";}
  }

  typedef std::tuple<HeO, StatO, SolO> output_option_t;
  std::ostream& operator <<(std::ostream& out, const HeO m) {
    switch (m) {
    case HeO::show : return out << "show-header";
    default : return out << "noshow-header";}
  }
  std::ostream& operator <<(std::ostream& out, const StatO m) {
    switch (m) {
    case StatO::show : return out << "show-statistics";
    default : return out << "noshow-statistics";}
  }
  std::ostream& operator <<(std::ostream& out, const SolO m) {
    switch (m) {
    case SolO::show : return out << "show-solutions";
    default : return out << "noshow-solutions";}
  }


  LS::ls_dim_t read_N(const std::string& s, const std::string& error) noexcept {
    if (s.empty()) return N_default;
    const LS::ls_dim_t N = FloatingPoint::touint(s);
    if (not LS::valid(N) and N != 0) {
      std::cerr << error << "N must be a nonnegative integer in [0,"
                << LS::max_dim-1 << "]" << ", but N=" << N << ".\n";
      std::exit(int(RG::Error::domain));
    }
    return N;
  }
  LS::ls_dim_t read_N(const std::string& error) noexcept {
    std::string s;
    std::cin >> s;
    return read_N(s, error);
  }

  LS::ls_dim_t read_k(const std::string& s,
                      const std::string& error) noexcept {
    if (s.empty()) return k_default;
    const LS::ls_dim_t k = FloatingPoint::touint(s);
    if (not LS::valid(k) and k != 0) {
      std::cerr << error << "k must be a nonnegative integer in [0,"
                << LS::max_dim-1 << "]" << ", but k=" << k << ".\n";
      std::exit(int(RG::Error::domain));
    }
    return k;
  }
  LS::ls_dim_t read_k(const std::string& error) noexcept {
    std::string s;
    std::cin >> s;
    return read_k(s, error);
  }

  gecode_intvec_t read_partial_ls(const LS::ls_dim_t N) noexcept {
    assert(N > 0);
    const LS::ls_dim_t size = N*N;
    gecode_intvec_t partial_ls(size);
    std::string s;
    partial_ls_t ls_s;
    do {
      std::cin >> s;
      if (s.empty()) continue;
      if (N > 10) ls_s.push_back(s);
      else for (auto c : s) ls_s.push_back(std::string(1,c));
      assert(ls_s.size() <= size);
    } while (ls_s.size() != size);
    for (LS::ls_dim_t i=0; i < size; ++i) {
      assert(i < partial_ls.size() and i < ls_s.size());
      partial_ls[i] = (ls_s[i] == "*") ? -1 : std::stoi(ls_s[i]);
    }
    return partial_ls;
  }


  LS::ls_dim_t given_cells(const gecode_intvec_t ls_partial) {
    LS::ls_dim_t res = 0;
    for (auto x : ls_partial) res += x==-1 ? 0 : 1;
    return res;
  }

  void print_header() {
    std::cout
      << "N k m1 m2 brt brsrc brsol bregr brpr prp t sat nds inds inds2 inds3 lvs "
      << "ulvs sol 1chld chcs taus sbps chct taut sbpt ptime prog vers\n";
  }

  void print_stat(const LS::ls_dim_t N, const LS::ls_dim_t k,
                  const LS::ls_dim_t m1, const LS::ls_dim_t m2,
                  const double reading_time, const double solving_time,
                  const LA::option_t alg_options,
                  const gecode_option_t gc_options,
                  LA::statistics_t stat,
                  const Environment::ProgramInfo& proginfo) {
    const auto sat = stat->solutions==0 ? 0 : 1;
    const auto lvs = stat->unsat_leaves + stat->solutions;
    const LA::BrTypeO brt = std::get<LA::BrTypeO>(alg_options);
    const LA::BrSourceO brsrc = std::get<LA::BrSourceO>(alg_options);
    const LA::BrSolutionO brsol = std::get<LA::BrSolutionO>(alg_options);
    const LA::BrEagernessO bregr = std::get<LA::BrEagernessO>(alg_options);
    const LA::BrPruneO brpr = std::get<LA::BrPruneO>(alg_options);
    Environment::RegistrationPolicies<LA::BrTypeO> rp_brt;
    Environment::RegistrationPolicies<LA::BrSourceO> rp_brsrc;
    Environment::RegistrationPolicies<LA::BrSolutionO> rp_brsol;
    Environment::RegistrationPolicies<LA::BrEagernessO> rp_bregr;
    Environment::RegistrationPolicies<LA::BrPruneO> rp_brpr;
    const std::string sbrt = rp_brt.string[int(brt)];
    const std::string sbrsrc = rp_brsrc.string[int(brsrc)];
    const std::string sbrsol = rp_brsol.string[int(brsol)];
    const std::string sbregr = rp_bregr.string[int(bregr)];
    const std::string sbrpr = rp_brpr.string[int(brpr)];

    const Euler::PropO prop = std::get<Euler::PropO>(gc_options);
    Environment::RegistrationPolicies<Euler::PropO> rp_prop;
    const std::string sprop = rp_prop.string[int(prop)];

    const unsigned prec_time = 4;
    const auto fi = std::fixed;
    std::cout << std::setprecision(prec_time) << fi << N << " " << k
              << " " << m1 << " " << m2 << " " << sbrt << " " << sbrsrc
              << " " << sbrsol << " " << sbregr << " " << sbrpr
              << " " << sprop << " " << solving_time << " " << sat << " "
              << stat->nodes << " " << stat->inner_nodes << " "
              << stat->inner_nodes_2chld << " " << stat->inner_nodes_3chld
              << " " << lvs << " " << stat->unsat_leaves << " "
              << stat->solutions << " " << stat->single_child_brnch << " "
              << stat->choice_time.N << " " << stat->tau_time.N << " "
              << stat->subproblem_time.N << " " << stat->choice_time.sum
              << " " << stat->tau_time.sum << " "
              << stat->subproblem_time.sum << " " << reading_time << " "
              << proginfo.prg << " " << proginfo.vrs << "\n";
  }

  class TWO_MOLS : public GC::Space {
    const LS::ls_dim_t N;
    const LA::option_t alg_options;
    const gecode_option_t gecode_options;
    const LA::weights_t wghts;
    const LA::statistics_t stat;
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
          ipl = GC::IPL_VAL;
          break;
      case PropO::bnd:
          ipl = GC::IPL_BND;
          break;
      case PropO::dom:
          ipl = GC::IPL_DOM;
          break;
      case PropO::def:
          ipl = GC::IPL_DEF;
          break;
      default:
          ipl = GC::IPL_DOM;
          break;
      }
      return ipl;
    }
  public:
    TWO_MOLS(const LS::ls_dim_t N, const LA::option_t alg_options,
             const gecode_option_t gecode_options,
             const gecode_intvec_t ls1_partial = {},
             const gecode_intvec_t ls2_partial = {},
             const LA::weights_t wghts = nullptr,
             const LA::statistics_t stat = nullptr) :
      N(N), alg_options(alg_options), gecode_options(gecode_options),
      wghts(wghts), stat(stat),
      x(*this, N*N, 0, N - 1),
      y(*this, N*N, 0, N - 1),
      z(*this, N*N, 0, N - 1),
      V(*this, x.size() + y.size() + z.size(), 0, N - 1) {
      assert(valid());
      assert(wghts);
      assert(stat);
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
            if (ls1_partial[i*N + j] >= 0) {
              dom(*this, x[i*N + j], ls1_partial[i*N + j], ls1_partial[i*N + j],
                  prp_lvl);
            }
            assert(i*N + j < ls2_partial.size());
            if (ls2_partial[i*N + j] >= 0) {
              dom(*this, y[i*N + j], ls2_partial[i*N + j], ls2_partial[i*N + j],
                  prp_lvl);
            }
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
        assert(wghts->size() == N-2);
        LA::post_branching<TWO_MOLS>(*this, V, alg_options);
      }

    }

    TWO_MOLS(TWO_MOLS& T) : GC::Space(T), N(T.N), alg_options(T.alg_options),
             gecode_options(T.gecode_options), wghts(T.wghts), stat(T.stat) {
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

    LA::weights_t weights() const noexcept { assert(valid()); return wghts; }

    LA::statistics_t statistics() const noexcept { assert(valid()); return stat; }

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

#endif