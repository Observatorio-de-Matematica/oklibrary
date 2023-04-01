// Oliver Kullmann, 28.2.2022 (Swansea)
/* Copyright 2022, 2023 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  Certain tools for handling Dimacs-files


   - Imported from Random/ClauseSets.hpp (first three from VarLit.hpp):

      - var_t : typedef for unsigned 64-bit
      - Var : concrete type, wrapper around var_t
      - Lit : conrete type, wrapper around v : Var, s : bool (true is positive)

      - Clause : typedef for vector of Lit
      - ClauseList : typedef for vector of Clause
      - dimacs_pars : concrete type, wrapper around n, c : var_t
      - DimacsClauseList : typedef for pair of dimacs_pars, ClauseList.

   - General tools:

      - VarSet : typedef for set of Var
      - var(Clause) -> VarSet
      - var(ClauseList) -> VarSet
      - var(DimacsClauseList) -> VarSet

      - struct FormalClauseList: wrapper for
       - DimacsClauseList F
       - VarSet V
       - operations <=> and <<

      - varlist_t : typedef for vector of Var
      - var(varlist_t) -> VarSet

      - is_sorted(Clause) -> bool
      - sorted_elements(ClauseList) -> bool
      - sorted_elements(DimacsClauseList) -> bool
      - is_fully_sorted(ClauseList) -> bool
      - is_fully_sorted(DimacsClauseList) -> bool

      - is_tautological(Clause) -> bool

      Modifiers ("m" for in-place-modification, "e" for erase):

       - msort(Clause&)
       - emsort(Clause&) (erase duplicated literals)

       - emtautological(ClauseList&) (remove tautological clauses)
       - emtautological(DimacsClauseList&)

       - msort_elements(ClauseList&)
       - msort_elements(DimacsClauseList&)
       - emsort_elements(ClauseList&)
       - emsort_elements(DimacsClauseList&)

       - sort_elements(ClauseList) -> ClauseList
       - esort_elements(ClauseList) -> ClauseList
       - sort_elements(DimacsClauseList) -> DimacsClauseList
       - esort_elements(DimacsClauseList) -> DimacsClauseList

       "full" means elements and list itself:
       - full_emsort(ClauseList&)
       - full_emsort(DimacsClauseList&)
       - standardise_clauses(ClauseList&) (full_emsort plus removal of
         tautological clauses)
       - standardise_clauses(DimacsClauseList&).

      Statistics:

       - num_litocc(ClauseList) -> var_t
       - num_litocc(DimacsClauseList) -> var_t
       - length_statistics(ClauseList) -> GenStats::FreqStats
         (result type has typedef length_statistics_t)
       - length_statistics(DimacsClauseList) -> GenStats::FreqStats


   - Reading strict Dimacs from istream:

    "Strict" means
     - the n-bound and the c-bound is sharp
     - all comments before the p-line (and they start with "c")
     - the p-line starts exactly with "p cnf "
     - every clause on its own line
     - reading of literals using reading of unsigned integers.
    Pure variables (occurring in exactly one sign) and trivial variables
    (not occurring at all) are allowed.

    - read_strict_dimacs_pars -> dimacs_pars :
        skipping lines starting with "c", then expecting a line exactly
        starting with "p cnf ", and then reading two var_t's.
    - read_strict_literal -> Lit :
        reading a string (in the normal way), extracting a possible leading
        "-", and then reading one ull.
    - read_strict_clause -> Clause :
        via read_string_literal, literals are read, and pushed into the
        vector, until 0 is read; end-of-line then asserted.
      Remark: all clause-reading-functions read including the end-of-line
      character.
    - read_strict_Dimacs -> DimacsClauseList :
        first read_strict_dimacs_pars, establishing c, then reading exactly
        c clauses via read_strict_clause.


   Reading strict QDimacs from istream:

   "Strict" additionally means here that a- and e-line are on their
   lines, and are not broken over several lines.

    - read_strict_variable -> Var :
        reading a string, asserting it not starting with "-", and then
        reading ull, converting to Var.

    - read_strict_aline -> varlist_t :
        read string, assert it to be "a", then similar to read_strict_clause
        use read_strict_variable until 0 found (eol also asserted).
    - skip_strict_eline -> void :
        read string, assert it to be "e", read and discard the rest of line.
    - read_strict_gline -> varlist_t :
        expects lines starting with a or e (as above with aline/eline),
        if the first line is an a-line, this is taken as the list of
        global variables, all other a/e-lines are discarded (stoping with the
        first none-a/e-line)
    - read_strict_gline_withother -> pair<VarSet, other_ealines_t>
        other_ealines_t = vector<string>
      now also returning the other e/a-lines as is (and returning a set)

    - varmap_t : typedef for map from Var to Var
    - list2map(varlist_t V) -> varmap_t :
        maps the variables in V to 1, ..., V.size() (asserting that all
        variables were different)
    - list_as_map(varlist_t V) -> varmap_t :
        the inverse map (i -> vi)
    - rename(Lit, varmap_t) -> Lit :
        rename the literal according to the var-map, return {0,+1} iff
        the literal is Lit(0), and return {0,-1} if the map does not
        contain the variable
    - rename(Clause, varmap_t) -> Clause :
        rename the literals in the clause according to the var-map,
        ignoring literals with variables not in the map.

    - read_strict_clause_filterrename (also for Dimacs) -> Clause :
        using a var-map, like read_strict_clause, also reading until
        Lit(0) found, but extracting only literals in the var-map.

    - read_strict_clause_split (also for Dimacs) -> std::array<Clause,2} :
        using a var-set, like read_strict_clause, also reading until
        Lit(0) found, and splitting into two clauses according to var-set.

    - extract_apart_strict2qcnf -> void, sice-effect -> ostream :
        reads a whole strict 2QCNF, and puts the extracted global slice
        to the out-stream (see application 2QCNF2aCNF)
    - extract_gpart_strictqcnf -> void, sice-effect -> ostream :
        more general, read strict QCNF (see application QCNF2gCNF).


   - Sliced clause-sets (-lists)

    - valid_slicedcnf(F) : defines the concepts of a "sliced CNF"
        true iff all members are valid DimacsClauseList's
        and have the same number of clauses
    - sorted_elements_slicedcnf(F)
    - struct GslicedCNF :
     - typedef scnf_t (sliced CNF as array of size 2)
     - members
      - SF : scnf_t
      - reference-access O(), G() to index 0 resp 1
      - VarSet V
      - other : other_ealines_t
      - operator ==
   - valid(GslicedCNF) : SF is valid and has sorted elements and V == var(G)
   - operator << for GslicedCNF
   - read_strict_GslicedCNF(istream) -> GslicedCNF


   - Using external SAT solvers:

    - typedef Lit_filter (function from Lit to bool)
    - scoped enum SolverR
    - function extract_ret(SystemCalls::ReturnValue) -> SolverR
    - class Minisat_stats
     - sr : SolverR
    - class Minisat_return
     - rv : EReturnValue
      - ReturnValue rv, string out, err
     - stats : Minisat_stats
     - pa : Clause (partial assignment)

    - const strings input_filename, output_filename, minisat_string

    - minisat_call(string CNFfile, Lit_filter, string options)
          -> Minisat_return
      the Lit_filter is for the interpretation of a satisfying assignment,
      while the options-string is passed as is
    - minisat_call(DimacsClauseList F, Lit_filter, string options)
          -> Minisat_return
      writes F to a CNFfile, and then calls the above version.
    - minisat_call(SystemCalls::put_cin_t, Lit_filter, string options)
          -> Minisat_return
      does not create a file, but uses a pipe to stdin of minisat.
      Via DimacsClauseListref_put(F) one can wrapp a DimacsClauseList.


See plans/general.txt.

*/

#ifndef DIMACSTOOLS_E5ASWHPBqt
#define DIMACSTOOLS_E5ASWHPBqt

#include <istream>
#include <string>
#include <functional>
#include <filesystem>
#include <exception>
#include <utility>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <type_traits>
#include <array>
#include <set>
#include <algorithm>

#include <cassert>
#include <cstdio> // for std::FILE, std::fputs
#include <cinttypes> // for PRIu64

#include <ProgramOptions/Strings.hpp>
#include <Transformers/Generators/Random/ClauseSets.hpp>
#include <SystemSpecifics/SystemCalls.hpp>
#include <Numerics/Statistics.hpp>

#include "Algorithms.hpp"

namespace DimacsTools {

  /* Reading strict Dimacs
      - for the parameters n, c: n is an upper bound for the occurring
        var-indices, c is the exact number of clauses.
  */
  typedef RandGen::var_t var_t;
  typedef RandGen::Var Var;
  typedef RandGen::Lit Lit;
  typedef RandGen::Clause Clause;
  typedef RandGen::ClauseList ClauseList;
  typedef RandGen::dimacs_pars dimacs_pars;
  typedef RandGen::DimacsClauseList DimacsClauseList;


  typedef std::set<Var> VarSet;
  VarSet var(const Clause& C) {
    VarSet res;
    for (const Lit x : C) res.insert(var(x));
    return res;
  }
  VarSet var(const ClauseList& F) {
    VarSet res;
    for (const Clause& C : F) {
      const VarSet V = var(C);
      res.insert(V.begin(), V.end());
    }
    return res;
  }
  VarSet var(const DimacsClauseList& F) {
    return var(F.second);
  }

  struct FormalClauseList {
    DimacsClauseList F;
    VarSet V;
    friend auto operator <=>(const FormalClauseList&,
                             const FormalClauseList&) noexcept = default;
    friend std::ostream& operator <<(std::ostream& out,
                                     const FormalClauseList& F) {
      out << F.F;
      Environment::out_line(out, F.V);
      return out << "\n";
    }
  };


  void msort(Clause& C) noexcept {
    std::ranges::sort(C);
  }
  bool is_sorted(const Clause& C) noexcept {
    return std::ranges::is_sorted(C);
  }
  void emsort(Clause& C) noexcept {
    msort(C);
    C.erase(std::unique(C.begin(), C.end()), C.end());
  }

  bool is_tautological(const Clause& C) noexcept {
    assert(is_sorted(C));
    const auto adjfind = std::ranges::adjacent_find(C,
      [](const Lit x, const Lit y) {return x == -y;});
    return adjfind != C.end();
  }
  void emtautological(ClauseList& F) noexcept {
    std::erase_if(F, is_tautological);
  }
  void emtautological(DimacsClauseList& F) noexcept {
    assert(F.first.c == F.second.size());
    emtautological(F.second);
    F.first.c = F.second.size();
  }

  void msort_elements(ClauseList& F) noexcept {
    std::ranges::for_each(F, msort);
  }
  void msort_elements(DimacsClauseList& F) noexcept {
    msort_elements(F.second);
  }
  void emsort_elements(ClauseList& F) noexcept {
    std::ranges::for_each(F, emsort);
  }
  void emsort_elements(DimacsClauseList& F) noexcept {
    emsort_elements(F.second);
  }

  ClauseList sort_elements(ClauseList F) noexcept {
    msort_elements(F); return F;
  }
  ClauseList esort_elements(ClauseList F) noexcept {
    emsort_elements(F); return F;
  }
  DimacsClauseList sort_elements(DimacsClauseList F) noexcept {
    msort_elements(F.second); return F;
  }
  DimacsClauseList esort_elements(DimacsClauseList F) noexcept {
    emsort_elements(F.second); return F;
  }

  bool sorted_elements(const ClauseList& F) noexcept {
    return std::ranges::all_of(F, is_sorted);
  }
  bool sorted_elements(const DimacsClauseList& F) noexcept {
    return sorted_elements(F.second);
  }

  bool is_fully_sorted(const ClauseList& F) noexcept {
    return sorted_elements(F) and std::ranges::is_sorted(F);
  }
  bool is_fully_sorted(const DimacsClauseList& F) noexcept {
    return is_fully_sorted(F.second);
  }

  // The result of full_emsort corresponds precisely to first creating
  // clauses as sets from the clauses of F, and then put this into a set:
  void full_emsort(ClauseList& F) noexcept {
    emsort_elements(F);
    std::ranges::sort(F);
    F.erase(std::unique(F.begin(), F.end()), F.end());
  }
  void full_emsort(DimacsClauseList& F) noexcept {
    full_emsort(F.second);
    F.first.c = F.second.size();
  }
  // Sorted non-tautological elements without repetitions, and
  // the whole list also sorted without repetitions (full_emsort plus
  // elimination of tautological clauses):
  void standardise_clauses(ClauseList& F) noexcept {
    emsort_elements(F);
    emtautological(F);
    std::ranges::sort(F);
    F.erase(std::unique(F.begin(), F.end()), F.end());
  }
  void standardise_clauses(DimacsClauseList& F) noexcept {
    standardise_clauses(F.second);
    F.first.c = F.second.size();
  }


  var_t num_litocc(const ClauseList& F) noexcept {
    var_t sum = 0;
    for (const Clause& C : F) sum += C.size();
    return sum;
  }
  var_t num_litocc(const DimacsClauseList& F) noexcept {
    return num_litocc(F.second);
  }
  typedef GenStats::FreqStats<var_t, FloatingPoint::float80>
          length_statistics_t;
  length_statistics_t length_statistics(const ClauseList& F) {
    return {F, [](auto){return true;},
        [](const Clause& C){return C.size();}};
  }
  length_statistics_t length_statistics(const DimacsClauseList& F) {
    return length_statistics(F.second);
  }


  // Skipping lines which start with "c", after which must come a line
  // starting with "p cnf ":
  dimacs_pars read_strict_dimacs_pars(std::istream& in) noexcept {
    std::string line;
    do {
      std::getline(in, line); assert(in and not line.empty());
    } while (line.front() == 'c');
    assert(in and not line.empty() and line.starts_with("p cnf "));
    std::istringstream s(line.substr(6));
    var_t n; s >> n; var_t c; s >> c; assert(s);
    return {n,c};
  }

  Lit read_strict_literal(std::istream& in) noexcept {
    std::string s;
    in >> s;
    assert(in); assert(not s.empty());
    if (s.starts_with('-')) return {std::stoull(s.substr(1)), -1};
    else return {std::stoull(s), +1};
  }
  Clause read_strict_clause(std::istream& in) noexcept {
    assert(in);
    Clause res;
    for (Lit x; (x = read_strict_literal(in)).v != Var{0}; res.push_back(x));
    [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
    return res;
  }
  DimacsClauseList read_strict_Dimacs(std::istream& in) noexcept {
    assert(in);
    DimacsClauseList res;
    res.first = read_strict_dimacs_pars(in); assert(in);
    var_t c = res.first.c;
    res.second.reserve(c);
    for (; c != 0; --c) res.second.push_back(read_strict_clause(in));
    return res;
  }

  Var read_strict_variable(std::istream& in) noexcept {
    std::string s;
    in >> s;
    assert(in); assert(not s.empty()); assert(not s.starts_with('-'));
    return Var(std::stoull(s));
  }

  typedef std::vector<Var> varlist_t;
  VarSet var(const varlist_t& v) { return VarSet(v.begin(), v.end()); }

  varlist_t read_strict_aline(std::istream& in) noexcept {
    assert(in);
    {std::string s; in >> s; assert(s == "a");}
    varlist_t res;
    for (Var v; (v = read_strict_variable(in)) != Var(0); res.push_back(v));
    [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
    assert(in);
    return res;
  }
  void skip_strict_eline(std::istream& in) noexcept {
    assert(in);
    {std::string s; in >> s; assert(s == "e");}
    std::string dummy; std::getline(in, dummy, '\n');
  }
  // Reading all a-,e-lines, extracting "global variables", which are the
  // outermost universal variables if existing:
  varlist_t read_strict_gline(std::istream& in) {
    assert(in);
    varlist_t res;
    {std::string s; in >> s; assert(s == "a" or s == "e");
     if (s == "a") {
       for (Var v; (v = read_strict_variable(in)) != Var(0); res.push_back(v));
       [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
     }
     else { std::string dummy; std::getline(in, dummy, '\n'); }
    }
    assert(in);
    char c = in.peek();
    while (not in.eof() and (c == 'a' or c == 'e')) {
      std::string dummy; std::getline(in, dummy, '\n');
      c = in.peek();
    }
    assert(in);
    return res;
  }
  typedef std::vector<std::string> other_ealines_t;
  // Now also return the other e/a-lines, and returning a VarSet:
  std::pair<VarSet, other_ealines_t>
  read_strict_gline_withother(std::istream& in) {
    assert(in);
    std::pair<VarSet, other_ealines_t> res;
    {std::string s; in >> s; assert(s == "a" or s == "e");
     if (s == "a") {
       for (Var v; (v = read_strict_variable(in)) != Var(0);
            res.first.insert(v));
       [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
     }
     else {
       std::string line; std::getline(in, line, '\n');
       res.second.push_back("e" + std::move(line) + "\n");
     }
    }
    assert(in);
    char c = in.peek();
    while (not in.eof() and (c == 'a' or c == 'e')) {
      std::string line; std::getline(in, line, '\n');
      res.second.push_back(std::move(line) + "\n");
      c = in.peek();
    }
    assert(in);
    assert(res.second.empty() or res.second[0].starts_with("e "));
    return res;
  }

  typedef std::map<Var, Var> varmap_t;

  // For V=[v1,v2,...,vn] create the mapping vi -> i
  // (i.e., for the map m holds m(V[i]) = i):
  varmap_t list2map(const varlist_t& V) {
    var_t v = 1;
    varmap_t m;
    for (const Var w : V) m.insert({w,Var(v++)});
    assert(m.size() == V.size());
    return m;
  }
  // For V=[v1,v2,...,vn] create the mapping i -> iv
  // (i.e., for the map m holds m(i) = V[i]):
  varmap_t list_as_map(const varlist_t& V) {
    varmap_t m;
    for (var_t i = 0; i < V.size(); ++i) m.insert({Var(i+1),V[i]});
    assert(m.size() == V.size());
    return m;
  }

  Lit rename(const Lit x, const varmap_t& m) noexcept {
    if (x == Lit(0)) return {0,+1};
    const auto f = m.find(x.v);
    if (f == m.end()) return {0,-1};
    else return {x.s, f->second};
  }
  Clause rename(const Clause& C, const varmap_t& m) noexcept {
    Clause res;
    for (const Lit x : C) {
      const Lit y = rename(x, m);
      if (y != Lit{0,-1}) res.push_back(y);
    }
    return res;
  }

  // Reading a clause, filtering out (and otherwise) renaming by m:
  Clause read_strict_clause_filterrename(std::istream& in,
                                         const varmap_t m) {
    assert(in);
    Clause res;
    for (Lit x; (x = rename(read_strict_literal(in), m)) != Lit(0);)
      if (x != Lit(0,-1)) res.push_back(x);
    [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
    return res;
  }

  // Reading a clause, splitting into two according to vs (in/out):
  std::array<Clause,2> read_strict_clause_split(std::istream& in,
                                                const VarSet& vs) {
    assert(in);
    std::array<Clause,2> res;
    for (Lit x; (x = read_strict_literal(in)) != Lit(0);)
      if (vs.contains(x.v)) res[0].push_back(x);
      else res[1].push_back(x);
    [[maybe_unused]]const char eol = in.get(); assert(eol == '\n');
    return res;
  }

  void extract_apart_strict2qcnf(std::istream& in,
                                 std::ostream& out) {
    const dimacs_pars dp = read_strict_dimacs_pars(in);
    const varmap_t m = list2map(read_strict_aline(in));
    skip_strict_eline(in);
    out << dimacs_pars(m.size(), dp.c);
    for (var_t c = dp.c; c != 0; --c)
      out << read_strict_clause_filterrename(in, m);
  }
  // More generally, extract the "global part" of a QCNF, but with the
  // difference that now the empty clause-set is returned in case there were
  // no global variables (in which case also "in" is not read further):
  void extract_gpart_strictqcnf(std::istream& in, std::ostream& out) {
    const dimacs_pars dp = read_strict_dimacs_pars(in);
    const varmap_t m = list2map(read_strict_gline(in));
    if (m.empty()) { out << dimacs_pars(0,0); return; }
    out << dimacs_pars(m.size(), dp.c);
    for (var_t c = dp.c; c != 0; --c)
      out << read_strict_clause_filterrename(in, m);
  }


  template <class CNF>
  bool valid_slicedcnf(const CNF& F) noexcept {
    {typedef typename CNF::value_type cnf_t;
     if (not std::is_same_v<cnf_t, DimacsClauseList>) return false;}
    typedef typename CNF::size_type size_t;
    const size_t size = F.size();
    if (size == 0) return true;
    const DimacsClauseList& F0 = F[0];
    if (not valid(F0)) return false;
    const auto c = F0.first.c;
    for (size_t i = 1; i < size; ++i) {
      const DimacsClauseList& G = F[i];
      if (G.first.c != c) return false;
      if (not valid(G.second)) return false;
    }
    return true;
  }
  template <class CNF>
  bool sorted_elements_slicedcnf(const CNF& FF) noexcept {
    return std::all_of(FF.begin(), FF.end(),
      [](const DimacsClauseList& F){return sorted_elements(F);});
  }

  struct GslicedCNF {
    typedef std::array<DimacsClauseList, 2> scnf_t;
    scnf_t SF; // "other" and "global"
    VarSet V; // occurring global variables and 
    VarSet T; // trivial global variables
    other_ealines_t other;

    GslicedCNF() noexcept {}
    GslicedCNF(const DimacsClauseList& O, const FormalClauseList& R,
               const other_ealines_t& other) :
      SF{O, R.F}, V{R.V}, other(other) {}

    const DimacsClauseList& O() const noexcept { return SF[0]; }
    DimacsClauseList& O() noexcept { return SF[0]; }
    const DimacsClauseList& G() const noexcept { return SF[1]; }
    DimacsClauseList& G() noexcept { return SF[1]; }

    bool operator ==(const GslicedCNF& rhs) const noexcept = default;
  };
  bool valid (const GslicedCNF& F) noexcept {
    return valid_slicedcnf(F.SF) and sorted_elements_slicedcnf(F.SF)
      and var(F.G()) == F.V and
      Algorithms::empty_intersection(F.V, F.T);
  }

  // Ignoring the trivial variables, standardising the global a-line,
  // outputting the clauses, sorting the clauses per slice, with
  // global-slice output second:
  std::ostream& operator <<(std::ostream& out, const GslicedCNF& F) {
    assert(valid(F));
    const var_t n = std::max(F.SF[0].first.n, F.SF[1].first.n),
      c = F.SF[0].first.c;
    out << dimacs_pars{n, c} << "a";
    for (const Var v : F.V) out << " " << v;
    out << " 0\n";
    for (const std::string& line : F.other) out << line;
    for (var_t i = 0; i < c; ++i) {
      const Clause& O = F.SF[0].second[i];
      if (not O.empty())
        for (const Lit x : O) out << x << " ";
      out << F.SF[1].second[i];
    }
    return out;
  }


  // The trivial global variables are not taken into accout for G.n:
  GslicedCNF read_strict_GslicedCNF(std::istream& in) {
    const dimacs_pars dp = read_strict_dimacs_pars(in);
    GslicedCNF res;
    res.O().first.c = dp.c;
    res.G().first.c = dp.c;
    {auto gline = read_strict_gline_withother(in);
     res.T = std::move(gline.first);
     res.other = std::move(gline.second);
    }
    for (var_t c = dp.c; c != 0; --c) {
      auto [C1,C0] = read_strict_clause_split(in, res.T);
      std::ranges::sort(C0); std::ranges::sort(C1);
      if (not C0.empty())
        res.O().first.n = std::max(res.O().first.n, C0.back().v.v);
      res.O().second.push_back(std::move(C0));
      res.G().second.push_back(std::move(C1));
    }
    for (const Clause& C : res.G().second)
      for (const Lit& x : C) {
        const Var v = var(x);
        res.V.insert(v);
        res.T.erase(v);
      }
    if (not res.V.empty()) res.G().first.n = (--res.V.end())->v;
    {[[maybe_unused]] const var_t nt = res.T.empty() ? 0 :
        (--res.T.end())->v;
     assert(dp.n == std::max({res.O().first.n, res.G().first.n, nt}));}
    assert(valid(res));
    return res;
  }


  typedef std::function<bool(Lit)> Lit_filter;
  const Lit_filter triv_filter = [](Lit){return true;};

  enum class SolverR {
    unknown=0,
    sat=1,
    unsat=2,
    aborted=3
  };
  std::ostream& operator <<(std::ostream& out, const SolverR sr) {
    switch (sr) {
    case SolverR::unknown : return out << "returned undetermined";
    case SolverR::sat : return out << "returned SAT";
    case SolverR::unsat : return out << "returned UNSAT";
    case SolverR::aborted : return out << "call aborted";
    default : return out << "SolverR::UNKNOWN";}
  }
  SolverR extract_ret(const SystemCalls::ReturnValue& rv) {
    if (rv.s != SystemCalls::ExitStatus::normal) return SolverR::aborted;
    else if (rv.val == 0) return SolverR::unknown;
    else if (rv.val == 10) return SolverR::sat;
    else if (rv.val == 20) return SolverR::unsat;
    else {
      std::ostringstream m;
      m << "DimacsTools::extract_ret: solver-call yields unknown return-value "
        << rv.val << ":\n"
        "  Allowed return-values are 0, 10, 20.\n"
        "  The exit-type of the solver-call was: " << rv.s << ".\n";
      throw std::runtime_error(m.str());
    }
  }

  struct Minisat_stats {
    SolverR sr;
    typedef SystemCalls::EReturnValue ret_t;
    Minisat_stats(const ret_t& rv) : sr(extract_ret(rv.rv)) {}
    friend std::ostream& operator <<(std::ostream& out, const Minisat_stats& s) {
      return out << s.sr << "\n";
    }
  };

  struct Minisat_return {
    typedef SystemCalls::EReturnValue ret_t;
    ret_t rv;
    Minisat_stats stats;
    Clause pa; // satisfyfing assignment, otherwise empty

    Minisat_return(const ret_t rv, const Lit_filter& f,
                   const std::filesystem::path& out)
      : rv(rv), stats(rv), pa(extract_pa(stats.sr, f, out)) {}

    static Clause extract_pa(const SolverR sr, const Lit_filter& f,
                             const std::filesystem::path& out) {
      if (sr != SolverR::sat) return {};
      auto lines = Environment::get_lines(out);
      if (lines.size() != 2)
        throw std::runtime_error("DimacsTools::Minisat_return::extract_pa: "
          "output-file has " + std::to_string(lines.size()) + " lines");
      if (lines[0] != "SAT")
        throw std::runtime_error("DimacsTools::Minisat_return::extract_pa: "
          "first output-line is \"" + lines[0] + "\"");
      std::istringstream s(std::move(lines[1]));
      Clause pa;
      for (Lit x; (x=read_strict_literal(s)).v.v != 0;)
        if (f(x)) pa.push_back(x);
      return pa;
    }
  };

  const std::string input_filename = "DimacsTools_minisatcall_in_";
  const std::string output_filename = "DimacsTools_minisatcall_out_";
  const std::string minisat_string = "minisat";

  // Reading from file:
  Minisat_return minisat_call(const std::string& input,
                              const Lit_filter& f = triv_filter,
                              const std::string& options = "") {
    assert(not input.empty());
    const std::string timestamp =
      std::to_string(Environment::CurrentTime::timestamp());
    const std::string out =
      SystemCalls::system_filename(output_filename + timestamp);
    const std::string command = minisat_string + " " + options + " "
      + input + " " + out;
    const std::filesystem::path pout(out);
    try {
      const Minisat_return res(SystemCalls::esystem(command, ""), f, pout);
      if (not std::filesystem::remove(pout))
        throw std::runtime_error(
          "DimacsTools::minisat_call(file): error when removing file " + out);
      return res;
    }
    catch (const std::runtime_error& e) {
      std::ostringstream o;
      o << "DimacsTools::minisat_call: Error when calling SAT-solver by\n"
        "  \"" << command << "\"\n  Specific error message:\n" << e.what()
        << "\n";
      throw std::runtime_error(o.str());
    }
  }

  Minisat_return minisat_call(const DimacsClauseList& F,
                              const Lit_filter& f = triv_filter,
                              const std::string& options = "") {
    const std::string timestamp =
      std::to_string(Environment::CurrentTime::timestamp());
    const std::string in =
      SystemCalls::system_filename(input_filename + timestamp);
    {std::ofstream fin(in);
     if (not fin)
       throw std::runtime_error(
         "DimacsTools::minisat_call(F): error when creating input-file " + in);
     fin << F;
    }
    const Minisat_return res = minisat_call(in, f, options);
    const std::filesystem::path pin(in);
    if (not std::filesystem::remove(pin))
      throw std::runtime_error(
        "DimacsTools::minisat_call(F): error when removing file " + in);
    return res;
  }


  // To make the following overloads of << available, use
  // "using DimacsTools:: operator <<;" :
  std::FILE* operator <<(std::FILE* const fp, const dimacs_pars& dp) {
    std::fprintf(fp, "p cnf %" PRIu64 " %" PRIu64 "\n", dp.n, dp.c);
    return fp;
  }
  std::FILE* operator <<(std::FILE* const fp, const Clause& C) {
    for (const Lit x : C)
      if (not x.s) fprintf(fp, "-%" PRIu64 " ", x.v.v);
      else fprintf(fp, "%" PRIu64 " ", x.v.v);
    fprintf(fp, "0\n");
    return fp;
  }
  std::FILE* operator <<(std::FILE* const fp, const ClauseList& F) {
    for (const Clause& C : F) fp << C;
    return fp;
  }
  std::FILE* operator <<(std::FILE* const fp, const DimacsClauseList& F) {
    return fp << F.first << F.second;
  }

  // Simple wrapper for FILE*-output, using strings per line:
  struct DimacsClauseListref_put {
    const DimacsClauseList& F;
    DimacsClauseListref_put(const DimacsClauseList& F) noexcept : F(F) {}
    void operator ()(std::FILE* const fp) const { fp << F; }
  };

  // Using a pipe for stdin of minisat:
  Minisat_return minisat_call(const SystemCalls::put_cin_t& PF,
                              const Lit_filter& f = triv_filter,
                              const std::string& options = "") {
    const std::string timestamp =
      std::to_string(Environment::CurrentTime::timestamp());
    const std::string out =
      SystemCalls::system_filename(output_filename + timestamp);
    const std::string command = minisat_string + " " + options + " "
      + "/dev/stdin" + " " + out;
    const std::filesystem::path pout(out);
    SystemCalls::Popen po(command);
    const Minisat_return res(po.etransfer(PF), f, pout);
    if (not std::filesystem::remove(pout))
      throw std::runtime_error(
        "DimacsTools::minisat_call(pipe): error when removing file " + out);
    return res;
  }

}

#endif
