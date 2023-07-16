// Oliver Kullmann, 1.7.2023 (Swansea)
/* Copyright 2023 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Tools for analysing qdimacs

TODOSL

1. Function snn2UInt_t (strict natnum to UInt_t)
    - Assume is_strict_natnum.
    - So the only problems can be that the number is too big; just return a
      pair of UInt_t and bool.

*/

#ifndef QDIMACSSYNTAX_s4ZN5nxQ7J
#define QDIMACSSYNTAX_s4ZN5nxQ7J

#include <utility>
#include <algorithm>
#include <exception>
#include <vector>

#include <cstdlib>

#include <ProgramOptions/Strings.hpp>
#include <Numerics/NumTypes.hpp>
#include <Numerics/NumInOut.hpp>

#include "DimacsTools.hpp"
#include "Algorithms.hpp"

namespace QDimacsSyntax {

  typedef Environment::tokens_t tokens_t;
  typedef FloatingPoint::uint_t level_t;
  typedef FloatingPoint::UInt_t count_t;
  typedef FloatingPoint::float80 float_t;

  enum class Error {
    missing_parameters = 1,
    input_file_error = 2,
  };

  const std::string is_incorrect = "ERROR";
  constexpr std::streamsize width_code = 3;
  void syntax_error(const int code) noexcept {
    std::cout << is_incorrect << std::setfill('0') <<
      std::setw(width_code) << code << std::endl;
    std::exit(0);
  }

  std::pair<std::pair<Environment::tokens_t, bool>, count_t>
  get_lines(std::ifstream& input) {
    assert(input.good());
    std::pair<std::pair<Environment::tokens_t, bool>, count_t>
      res{Environment::get_lines_check(input), 0};
    if (not res.first.second) return res;
    for (auto it = res.first.first.rbegin();
         it != res.first.first.rend(); ++it)
      if (it->empty()) ++res.second;
      else break;
    res.first.first.resize(res.first.first.size() - res.second);
    return res;
  }

  // Line with first non-c-line, returns true iff c-lines have syntax-error:
  std::pair<count_t, bool> analyse_comments(const tokens_t& F,
                                            const level_t tolerance) noexcept {
    count_t first_nonc = 0;
    for (; first_nonc < F.size(); ++first_nonc) {
      const auto& L = F[first_nonc];
      if (not L.starts_with("c")) break;
      if (tolerance == 0 and (L.size() == 1 or L[1] != ' '))
        return {first_nonc, true};
    }
    return {first_nonc, false};
  }

  // Allows s=="0":
  bool is_strict_natnum(const std::string_view s) noexcept {
    if (s.empty()) return false;
    if (s[0] == '0') return s == "0";
    return std::ranges::all_of(s, [](const char c)noexcept{
                                  return '0' <= c and c <= '9';});
  }

  // True iff an error occurred:
  std::pair<DimacsTools::dimacs_pars, bool>
  analyse_parline(const std::string& L, const level_t tolerance) noexcept {
    const auto S = Environment::split(L, ' ');
    if (tolerance == 0) {
      if (S.size() != 4 or S[0] != "p" or S[1] != "cnf" or
          not is_strict_natnum(S[2]) or not is_strict_natnum(S[3])
          or L.back() == ' ') return {{}, true};
      count_t n, c;
      try {
        n = FloatingPoint::to_UInt(S[2]);
        c = FloatingPoint::to_UInt(S[3]);
      }
      catch(const std::exception&) { return {{}, true}; }
      if (n == RandGen::max_var) return {{}, true};
      return {{n,c}, false};
    }
    else {
      if (S.size() < 4 or S[0] != "p" or S[1] != "cnf" or
          not is_strict_natnum(S[2])) return {{}, true};
      count_t cindex = 3;
      while (cindex < S.size() and S[cindex].empty()) ++cindex;
      if (cindex == S.size() or not is_strict_natnum(S[cindex]))
        return {{}, true};
      count_t n, c;
      try {
        n = FloatingPoint::to_UInt(S[2]);
        c = FloatingPoint::to_UInt(S[cindex]);
      }
      catch(const std::exception&) { return {{}, true}; }
      if (n == RandGen::max_var) return {{}, true};
      ++cindex; while (cindex < S.size() and S[cindex].empty()) ++cindex;
      if (cindex != S.size()) return {{}, true};
      return {{n,c}, false};
    }
  }

  bool begins_ae(const std::string& s) noexcept {
    return s.starts_with("a ") or s.starts_with("e ");
  }
  count_t first_nonae(const tokens_t& F, count_t i) noexcept {
    while (i < F.size() and begins_ae(F[i])) ++i;
    return i;
  }
  // Returns empty set iff error was found, otherwise the variables in
  // ae-line s:
  std::set<count_t> analyse_numbers_ae(const std::string& s,
                                       const count_t n,
                                       const level_t verbosity,
                                       count_t& spaces) noexcept {
    assert(s.starts_with("a ") or s.starts_with("e "));
    assert(s.ends_with(" 0"));
    const auto size = s.size();
    if (size <= 4) {
      if (verbosity >= 1)
        std::cout << "\nempty a/e-line\n";
      return {};
    }
    if (s[size-3] == ' ') ++spaces;
    const auto split =
      Environment::split(std::string_view(s).substr(2, size-4), ' ');
    std::set<count_t> res;
    for (count_t i = 0; i < split.size(); ++i) {
      const std::string& entry = split[i];
      if (entry.empty()) {++spaces; continue;}
      if (not is_strict_natnum(entry)) {
        if (verbosity >= 1)
          std::cout << "\nwrong entry \"" << entry << "\"\n";
        return {};
      }
      const count_t x = FloatingPoint::to_UInt(entry);
      if (x == 0) {
        if (verbosity >= 1)
          std::cout << "\nwrong entry 0\n";
        return {};
      }
      if (x > n) {
        if (verbosity >= 1)
          std::cout << "\nwrong entry " << x << " > max-n = " << n << "\n";
        return {};
      }
      if (res.contains(x)) {
        if (verbosity >= 1)
          std::cout << "\nrepeated entry " << x << "\n";
        return {};
      }
      res.insert(x);
    }
    if (res.empty()) {
      if (verbosity >= 1) {
        std::cout << "\na/e-line only contains spaces\n";
      }
      return {};
    }
    return res;
  }

  // The variables of the a-e-lines (second coordinate != end iff error found):
  std::pair<std::vector<std::set<count_t>>, count_t>
  readae(const tokens_t& F,
         const count_t n,
         const count_t begin, const count_t end,
         const level_t verbosity) {
    assert(begin < end and end < F.size());
    std::vector<std::set<count_t>> res(end - begin);
    count_t i = begin;
    count_t additional_spaces = 0;
    for (; i < end; ++i) {
      const count_t i0 = i - begin;
      res[i0] = analyse_numbers_ae(F[i], n, verbosity, additional_spaces);
      if (res[i0].empty()) break;
      for (count_t j0 = 0; j0 < i0; ++j0) {
        if (not Algorithms::empty_intersection(res[i0], res[j0])) {
          if (verbosity >= 1) {
            std::cout << "\na/e-line " << j0 << " intersects with line "
                      << i0 << "\n";
          }
          return {res, i};
        }
      }
    }
    if (verbosity >= 2)
      std::cout << "add-spaces-ae " << additional_spaces << "\n";
    return {res, i};
  }

  count_t max_ae_index(const std::vector<std::set<count_t>>& vars) noexcept {
    count_t res = 0;
    for (const auto& S : vars) {
      assert(not S.empty());
      res = std::max(res, *S.crbegin());
    }
    return res;
  }
  // Whether block 0 <= i < vars.size() is universal:
  std::vector<bool>
  is_universal_block(const std::vector<std::set<count_t>>& vars,
                     const bool first_a) {
    const auto size = vars.size();
    assert(size != 0);
    std::vector<bool> res(size);
    res[0] = first_a;
    for (count_t i = 1; i < size; ++i) res[i] = not res[i-1];
    return res;
  }
  // Whether variable v is in the ae-blocks:
  std::vector<bool>
  is_ae_var(const std::vector<std::set<count_t>>& vars,
                   const count_t n) {
    std::vector<bool> res(n+1);
    for (const auto& S : vars)
      for (const auto v : S) res[v] = true;
    return res;
  }
  // Whether variable v is universal:
  std::vector<bool>
  is_universal_var(const std::vector<std::set<count_t>>& vars,
                   const std::vector<bool>& unib,
                   const count_t n) {
    assert(vars.size() == unib.size());
    std::vector<bool> res(n+1);
    for (count_t i = 0; i < vars.size(); ++i) {
      const bool is_uni = unib[i];
      for (const auto v : vars[i]) res[v] = is_uni;
    }
    return res;
  }

  // The string_view-data is nullptr iff no " 0" was found:
  std::pair<std::string_view, std::string::size_type>
  literal_part(const std::string& s) noexcept {
    const auto end = s.rfind(" 0");
    if (end == std::string::npos) return {};
    if (s.find_first_not_of(' ', end+2) != std::string::npos) return {};
    return {{s.begin(), s.begin() + end}, s.size() - end - 2};
  }
  // Returns size of clause, and 0 iff error:
  typedef std::vector<count_t> degvec_t;
  count_t analyse_clause(const std::string& s0, degvec_t& pos, degvec_t& neg,
                         const count_t n, const level_t verbosity,
                         const std::vector<bool>& aev,
                         const std::vector<bool>& univ,
                         count_t& spaces,
                         const level_t tolerance,
                         count_t& repetitions) {
    const auto [s, trailing_spaces] = literal_part(s0);
    if (s.data() == nullptr) {
      if (verbosity >= 1)
        std::cout << "\nclause not containing final \" 0\"\n";
      return 0;
    }
    spaces += trailing_spaces;
    if (tolerance == 0 and trailing_spaces != 0) {
      if (verbosity >= 1)
        std::cout << "\nclause containing trailing spaces\n";
      return 0;
    }

    const auto size = s.size();
    if (size == 0) {
      if (verbosity >= 1)
        std::cout << "\nempty clause\n";
      return 0;
    }
    if (s[size-1] == ' ') ++spaces;
    const auto split = Environment::split(s, ' ');
    using Lit = DimacsTools::Lit;
    using Var = DimacsTools::Var;
    std::set<Lit> C;
    for (count_t i = 0; i < split.size(); ++i) {
      const std::string_view entry(split[i]);
      if (entry.empty()) {++spaces; continue;}
      const count_t start = entry[0] == '-';
      const std::string_view num = entry.substr(start);
      if (not is_strict_natnum(num)) {
        if (verbosity >= 1)
          std::cout << "\nwrong variable \"" << num << "\"\n";
        return 0;
      }
      const Var v(FloatingPoint::to_UInt(std::string(num)));
      if (v.v == 0) {
        if (verbosity >= 1)
          std::cout << "\nwrong variable 0\n";
        return 0;
      }
      if (v.v > n) {
        if (verbosity >= 1)
          std::cout << "\nwrong variable " << v << " > max-n = " << n << "\n";
        return 0;
      }
      if (not aev[v.v]) {
        if (verbosity >= 1)
          std::cout << "\nnon-ae-variable " << v << "\n";
        return 0;
      }
      const Lit x(start==0, v);
      if (C.contains(x)) {
        if (tolerance == 0) {
          if (verbosity >= 1)
            std::cout << "\nrepeated literal " << x << "\n";
          return 0;
        }
        else ++repetitions;
      }
      else if (C.contains(-x)) {
        if (verbosity >= 1)
          std::cout << "\ncomplementary literal " << x << "\n";
        return 0;
      }
      C.insert(x);
      if (x.s) ++pos[v.v]; else ++neg[v.v];
    }

    if (C.empty()) {
      if (verbosity >= 1) {
        std::cout << "\nclause only contains spaces\n";
      }
      return 0;
    }
    if (std::ranges::all_of(C, [&univ](const Lit x)noexcept{
                              return univ[x.v.v];})) {
      if (verbosity >= 1) {
        std::cout << "\nclause only contains universal variables\n";
      }
      return 0;
    }
    return C.size();
  }

  // Formal and pure (non-formal) global variables:
  std::pair<count_t, count_t>
  num_pure_global_vars(const bool first_a,
                       const std::set<count_t>& G,
                       const degvec_t& pos,
                       const degvec_t& neg) noexcept {
    std::pair<count_t, count_t> res{};
    if (not first_a) return res;
    for (const count_t v : G) {
      const bool zpos = pos[v] == 0, zneg = neg[v] == 0;
      if (zpos) { res.first += zneg; res.second += not zneg; }
      else res.second += zneg;
    }
    return res;
  }

}

#endif
