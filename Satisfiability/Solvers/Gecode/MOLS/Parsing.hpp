// Oliver Kullmann, 8.3.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  Parsing of the problem specification


0. Design for general k
    - N is constant.
    - We have primary squares A, B, C, ... (any identifier).
        intro A B C
    - Then we have secondary squares given by some unary operation on the
      primary squares:
       - We have t(A), at(A) (transposed and antitransposed) --- for these we
         likely do not introduce new variables.
         We could use "tA" (forbidding that an identifier starts with "t").
       - The question is here where t(A) can be used; likely we want a very
         restricted usage.
         diag(tA) is equivalent to diag(A), and the same for antidiag and
         at.
         So t(A) and at(A) are only used in lists for orthogonality?

         QUESTION: But perhaps it is actually easier to formally treat t(A)
         and at(A) as new squares, and then to actually eliminate them
         when posting the constraints?
       - Other types of "conjugates" likely need the introduction of new
         squares X = conj(A); expressed like
           conj A X
         introducing X.
       - A more specific syntax is needed:
           conj231 A A231
       - For the latter it is relevant that we can express the relation easily
         (likely via the elementship-constraint)>
    - Then we have conditions on those primary and secondary squares:
     - diag(A), antidiag(X), written
         diag A
         antidiag X
     - ortho A, X, t(B) ..., written
         ortho A X tB
       (here for the formulation we use pairwise the witness-squares; only
       here can we use say tB, atB).
     - = A X .

1. Handle special types of Latin squares (unary conditions):
    - Diagonal Latin square: in the main diagonal and antidiagonal all
      elements are distinct.
      We want two special conditions:
        diag A
        antidiag A
    - Constant diagonal Latin square: in the main diagonal all elements are
      equal to each other, similar for elements of the main antidiagonal.
      These ls's are called "unipotent" (and similarly "antiunipotent").
      Conditions:
        uni A
        antiuni A
    - Idempotent Latin square: A(i,i) = i.
        idem A
      Similarly "row-reduced", "column-reduced":
        rred A
        cred A
    - Symmetric Latin square: it is equal to its transpose.
        symm A
        antisymm A

        QUESTION: it seems that for N>=3 one can not have both at the same
        time? But perhaps easiest to allow such combinations.
    - Later (does not seem to be realisable):
      Semisymmetric Latin square: it is equal to (at least) three of its
      conjugates, which must necessarily include the (1, 2, 3), (3, 1, 2) and
      (2, 3, 1)-conjugates.
    - Totally symmetric Latin square: all six conjugates are equal:
      can be expressed by
       conj321 A A321
       ,,.
       symm A
       = A A321
       ...


2. Handle special types of orthogonality.
    - For example the search for 3 MOLS is expressed by
        intro A B C
        ortho A B C
    - Self-orthogonal Latin square: it is orthogonal to its transposition:
        intro A
        ortho A t(A)
    - Doubly self-orthogonal Latin square: it is orthogonal to both its
      transposed and antitransposed:
        intro A
        ortho A t(A)
        ortho A at(A).

*/

#ifndef PARSING_W8d7RNyiKu
#define PARSING_W8d7RNyiKu

#include <set>
#include <vector>
#include <map>
#include <utility>

#include <cassert>

namespace Parsing {

  // Unary conditions for lines (all combinations possible):
  enum class UCL {
    diag = 1,
    antidiag = 2,
    uni = 3,
    antiuni = 4,
    idem = 5,
    rred = 6,
    cred = 7
  };

  struct UConditions {
    typedef std::set<UCL> cond_t;
  private :
    cond_t cond_;
  public :
    UConditions() noexcept = default;
    const cond_t& cond() const noexcept { return cond_; }
    bool insert(const UCL c) {
      return cond_.insert(c).second;
    }
    bool contains(const UCL c) const noexcept {
      return cond_.contains(c);
    }
  };


  // Versions of a square:
  enum class VS {
    id = 0,
    c213 = 1,
    c312 = 2,
    c321 = 3,
    c132 = 4,
    c231 = 5,
    at = 6
  };

  struct Versions {
    typedef std::set<VS> choices_t;
  private :
    choices_t choices_;
  public :
    Versions() noexcept : choices_({VS::id}) {}
    const choices_t& choices() const noexcept { return choices_; }
    bool insert(const VS v) {
      return choices_.insert(v).second;
    }
    bool contains(const VS v) const noexcept {
      return choices_.contains(v);
    }
  };


  struct Conditions {
    typedef std::vector<Versions> vv_t;
    typedef vv_t::size_type size_t;
    typedef std::pair<size_t, VS> sq_t;
    typedef std::map<sq_t, UConditions> map_t;
    typedef std::pair<sq_t, sq_t> eq_t;
    typedef std::set<eq_t> set_eq_t;
    typedef std::set<sq_t> orth_t;
    typedef std::set<orth_t> set_orth_t;

    const size_t k; // the number of primary ls's
  private :
    vv_t versions_;
    map_t m_; // maps exactly the squares according to versions_
    set_eq_t eq_; // the equalities
    set_orth_t orth_; // the orthogonal sets of squares

  public :
    explicit Conditions(const size_t k) noexcept : k(k) {}

    const vv_t& versions() const noexcept { return versions_; }
    const map_t& map() const noexcept { return m_; }
    const set_eq_t& eq() const noexcept { return eq_; }
    const set_orth_t& orth() const noexcept { return orth_; }

    size_t num_squares() const noexcept {
      size_t sum = 0;
      for (const auto& v : versions_) sum += v.choices().size();
      return sum;
    }

    // Insert version v for primary square i:
    bool insert(const size_t i, const VS v) {
      assert(i < k);
      return versions_[i].insert(v);
    }
    bool insert(const sq_t s) {
      return insert(s.first, s.second);
    }
    bool contains(const size_t i, const VS v) const noexcept {
      assert(i < k);
      return versions_[i].contains(v);
    }
    bool contains(const sq_t s) const noexcept {
      return contains(s.first, s.second);
    }

    // Insert condition c for square s:
    bool insert(const sq_t s, const UCL c) {
      assert(contains(s));
      return m_[s].insert(c);
    }
    bool contains(const sq_t s, const UCL c) const noexcept {
      assert(contains(s.first, s.second));
      const auto f = m_.find(s);
      assert(f != m_.end());
      return f->second.contains(c);
    }

    // Insert equality-condition for squares s1, s2:
    bool insert(const sq_t s1, const sq_t s2) {
      assert(contains(s1) and contains(s2));
      return eq_.insert({s1,s2}).second;
    }
    bool contains(const sq_t s1, const sq_t s2) const noexcept {
      assert(contains(s1) and contains(s2));
      return eq_.contains({s1,s2}) or eq_.contains({s2,s1});
    }

    // Insert orthogonality-condition for a set of squares:
    bool valid(const orth_t& o) const noexcept {
      return std::all_of(o.begin(), o.end(),
                         [this](const sq_t s){return contains(s);});
    }
    bool insert(const orth_t& o) {
      assert(valid(o));
      return orth_.insert(o).second;
    }

  };
}

#endif
