// Oliver Kullmann, 12.6.2016 (Swansea)
/* Copyright 2016 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Satisfiability/Transformers/Generators/Pythagorean.cpp
  \brief CNF generator for the Pythagorean tuples problem

  For the boolean problems for triples, use
  > Pythagorean n 3 0 2
  or
  > Pythagorean n 3 1 2
  (doesn't matter here).

  For the boolean problem for quadruples, use
  > Pythagorean n 4 0 2
  while the injective form (all components different) is obtained by
  > Pythagorean n 4 1 2

  The second parameter is K >= 3, the length of the Pythagorean tuple.

  For the third parameter d >= 0, also larger values can be used, which enforce
  a respective minimum distance between the (sorted) components of the tuples

    x_1^2 + ... + x_{K-1}^2 = x_K^2,

  that is, x_i + d <= x_{i+1} for 1 <= i <= K-1.

  The fourth parameter m >= 0 is the number of colours, with
   - m = 0: only output the max-occuring vertex and the number of hyperedges
   - m = 1: output the hypergraph
   - m = 2: output the boolean problem (the default)
   - m >= 3: currently uses the strong direct translation.

  An optional fifth parameter can be "-", in which case output is put to
  standard output, or "filename", in which case a file is created.
  Default output is to file "Pyth_n_K_d_m.cnf".

  Requires C++11. Compile with

  > g++ -Wall --std=c++11 -Ofast -o Pythagorean Pythagorean.cpp

  Pythagorean numbers established:
   - Ptn(3,3) = 7825
   - Ptn(3,3,3) > 800000 (g2wsat)
   - Ptn(4,4) = 105
   - Ptn_i(4,4) = 163
   - Ptn(4,4,4) > 1600 (vw1) (2000 hard to satisfy)
   - Ptn(5,5) = 37
   - Ptn_i(5,5) = 75
   - Pth(5,5,5) = 191 (vw1 for 190, found easily; SplittingViaOKsolver with
     D=20 and minisat-2.2.0 for 191: total run-time around 46 min).

*/

#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>
#include <fstream>

namespace {

  typedef unsigned long int uint_t;
  typedef long int int_t;
  typedef unsigned long long int cnum_t;

  enum {
    errcode_parameter = 1,
    errcode_too_large = 2,
    errcode_too_small = 3,
    errcode_not_yet = 4,
    errcode_file = 5
  };

  const std::string program = "Pythagorean";
  const std::string err = "ERROR[" + program + "]: ";

  const std::string version = "0.2";

  const std::string filename = "Pyth_";

  typedef std::vector<uint_t> tuple_t;
  typedef std::vector<tuple_t> vector_t;

  void oklib_output(std::ostream* const out) {
    *out << "c OKlibrary, program " << program << ".cpp in version " << version << ".\n";
  }

  inline cnum_t var_number(const uint_t i, const uint_t m, const uint_t col) {
    assert(i >= 1);
    assert(col < m);
    return cnum_t(i-1) * m + col + 1;
  }

}

int main(const int argc, const char* const argv[]) {
  if (argc <= 4 or argc >= 7) {
    std::cerr << err << "Four or five arguments are needed:\n"
     " - The number n >= 0 of elements.\n"
     " - The size K >= 3 of the tuple.\n"
     " - The enforced distance d >= 0 between components.\n"
     " - The number m >= 0 of colours.\n"
     " - The filename or \"-\" for standard output.";
    return errcode_parameter;
  }

  const uint_t n = std::stoul(argv[1]);

  const uint_t K = std::stoul(argv[2]);
  if (K <= 2) {
    std::cerr << err << "Second input " << K << " must be at least 2.\n";
    return errcode_too_small;
  }
  if ( K > 5) {
    std::cerr << err << "Second input " << K << " currently must be at most 5.\n";
    return errcode_not_yet;
  }
  const uint_t abs_max = uint_t(std::sqrt(std::numeric_limits<uint_t>::max())) / K;
  if (n > abs_max) {
    std::cerr << err << "First input " << n << " larger than maximal allowed value: " << abs_max << ".\n";
    return errcode_too_large;
  }

  const uint_t dist = std::stoul(argv[3]);
  if (dist > abs_max) {
    std::cerr << err << "Third input " << dist << " larger than maximal allowed value: " << abs_max << ".\n";
    return errcode_too_large;
  }

  const uint_t m = std::stoul(argv[4]);

  const std::string file = (argc == 5) ?
    filename + std::to_string(n) + "-" + std::to_string(K) + "-" + std::to_string(dist) + "-" + std::to_string(m) + ".cnf"
    : argv[5];
  const bool del = (file != "-");
  std::ostream* const out = (del) ? new std::ofstream(file) : &std::cout;
  class Delete_wrapper {
    const std::ostream* const p;
  public :
    Delete_wrapper(std::ostream* const out) : p(out) {}
    ~Delete_wrapper() {delete p;}
  } const D((del) ? out : nullptr);
  if (not *out) {
    std::cerr << err << "Couldn't open file " << file << " for writing.\n";
    return errcode_file;
  }

  // Computing the list of Pythagorean tuples:
  vector_t res;
  cnum_t hn = 0;
  
  const uint_t n2 = n*n;
  uint_t max = 0;

  if (K == 3) {
    for (uint_t a = 1; a < n; ++a) {
      const uint_t a2 = a*a;
      for (uint_t b = a+dist; b < n; ++b) {
        const uint_t b2 = b*b;
        const uint_t c2 = a2 + b2;
        if (c2 > n2) break;
        const uint_t c = std::sqrt(c2);
        if (c*c != c2) continue;
        if (c < b+dist) continue;
        if (c > max) max = c;
        ++hn; if (m >= 1) res.push_back({{a,b,c}});
      }
    }
  }
  else if (K == 4) {
    for (uint_t a = 1; a < n; ++a) {
      const uint_t a2 = a*a;
      for (uint_t b = a+dist; b < n; ++b) {
        const uint_t b2 = b*b;
        for (uint_t c = b+dist; c < n; ++c) {
          const uint_t c2 = c*c;
          const uint_t d2 = a2 + b2 + c2;
          if (d2 > n2) break;
          const uint_t d = std::sqrt(d2);
          if (d*d != d2) continue;
          if (d < c+dist) continue;
          if (d > max) max = d;
          ++hn; if (m >= 1) res.push_back({{a,b,c,d}});
        }
      }
    }
  }
  else {
    for (uint_t a = 1; a < n; ++a) {
      const uint_t a2 = a*a;
      for (uint_t b = a+dist; b < n; ++b) {
        const uint_t b2 = b*b;
        for (uint_t c = b+dist; c < n; ++c) {
          const uint_t c2 = c*c;
          for (uint d = c+dist; d < n; ++d) {
            const uint_t d2 = d*d;
            const uint_t e2 = a2 + b2 + c2 + d2;
            if (e2 > n2) break;
            const uint_t e = std::sqrt(e2);
            if (e*e != e2) continue;
            if (e < d+dist) continue;
            if (e > max) max = e;
            ++hn; if (m >= 1) res.push_back({{a,b,c,d,e}});
          }
        }
      }
    }
  }

  if (m == 0) {
    *out << max << " " << hn << "\n";
    return 0;
  }

  // removing duplicates:
  for (auto& x : res) x.erase(std::unique(x.begin(), x.end()), x.end());

  // anti-lexicographical sorting:
  std::sort(res.begin(), res.end(),
    [](const tuple_t& x, const tuple_t& y) {
      return std::lexicographical_compare(x.rbegin(), x.rend(), y.rbegin(), y.rend());
    }
  );

  std::vector<cnum_t> degree(max, 0);
  for (const auto& x : res) for (const auto i : x) ++degree[i-1];
  cnum_t occ_n = 0, min_d = -1, max_d = 0, sum_d = 0;
  uint_t min_v = 0, max_v = 0;
  typedef std::vector<cnum_t>::size_type  vs_t;
  for (vs_t i = 0; i < degree.size(); ++i) {
    const auto deg = degree[i];
    if (deg != 0) {
      ++occ_n; sum_d += deg;
      if (deg < min_d) {min_d = deg; min_v = i+1;}
      if (deg > max_d) {max_d = deg; max_v = i+1;}
    }
  }

  if (m == 1) {
    *out << "c Hypergraph of Pythagorean " << K << "-tuples, up to n=" << n << ",\n"
    "c  with minimum-distance between (sorted) components = " << dist << ".\n";
    oklib_output(out);
    *out << "c Number of occurring vertices = " << occ_n << ".\n";
    if (occ_n > 0) {
      *out << "c Minimum degree = " << min_d << ", attained for vertex " << min_v << ".\n";
      *out << "c Maximum degree = " << max_d << ", attained for vertex " << max_v << ".\n";
      *out << "c Average degree = " << double(sum_d) / occ_n << ".\n";
    }
    *out << "p hyp " << max << " " << hn << "\n";
    for (const auto& x : res) {
      for (const auto i : x) *out << i << " ";
      *out << "0\n";
    }
  }
  else if (m == 2) {// DIMACS output:
    *out << "c Boolean Pythagorean " << K << "-tuples problem, up to n=" << n << ",\n"
    "c  with minimum-distance between (sorted) components = " << dist << ",\n"
    "c  yielding " << hn << " tuples.\n";
    oklib_output(out);
    *out << "c Number of occurring variables = " << occ_n << ".\n";
    if (occ_n > 0) {
      *out << "c Minimum degree = " << 2*min_d << ", attained for variable " << min_v << ".\n";
      *out << "c Maximum degree = " << 2*max_d << ", attained for variable " << max_v << ".\n";
      *out << "c Average degree = " << 2*double(sum_d) / occ_n << ".\n";
    }
    const cnum_t cn = 2 * hn;
    *out << "p cnf " << max << " " << cn << "\n";
    for (const auto& x : res) {
      for (const auto i : x) *out << i << " "; *out << "0 ";
      for (const int_t i : x) *out << -i << " "; *out << "0\n";
    }
  } else {
    assert(m >= 3);
    *out << "c " << m << "-Colour Pythagorean " << K << "-tuples problem, up to n=" << n << ",\n"
    "c  with minimum-distance between (sorted) components = " << dist << ",\n"
    "c  yielding " << hn << " tuples.\n";
    oklib_output(out);
    *out << "c Using the strong direct translation.\n";
    *out << "c Number of occurring variables = " << m*occ_n << ".\n";
    if (occ_n > 0) {
      *out << "c Degrees, ignoring the ALOAMO-clauses:\n";
      *out << "c  Minimum = " << m*min_d << ", attained for variable " << min_v << ".\n";
      *out << "c  Maximum = " << m*max_d << ", attained for variable " << max_v << ".\n";
      *out << "c  Average degree = " << m*double(sum_d) / occ_n << ".\n";
    }
    const cnum_t cn = m * hn + occ_n * (1 + (m * (m - 1)) / 2);
    const cnum_t vn = m * cnum_t(max);
    *out << "p cnf " << vn << " " << cn << "\n";
    for (const auto& x : res) {
      for (uint_t col = 0; col < m; ++col) {
        for (const auto i : x) *out << var_number(i,m,col) << " ";
        *out << "0"; if (col != m-1) *out << " ";
      }
      *out << "\n";
    }
    for (uint_t i = 0; i < degree.size(); ++i)
      if (degree[i] != 0) {
        const uint_t v = i+1;
        for (uint_t col = 0; col < m; ++col)
          *out << -int_t(var_number(v,m,col)) << " ";
        *out << "0";
        for (uint_t col1 = 0; col1 < m; ++col1)
          for (uint_t col2 = col1+1; col2 < m; ++col2)
            *out << " " << var_number(v,m,col1) << " " << var_number(v,m,col2) << " 0";
        *out << "\n";
      }
  }
}
