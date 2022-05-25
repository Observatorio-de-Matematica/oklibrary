// Oleg Zaikin, 11.5.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Helper functions for dealing with Gecode variables

  Namespace GcVariables, abbreviated "GV".

TODOS:

*/

#ifndef GCVARIABLES_FQM6cOCEcZ
#define GCVARIABLES_FQM6cOCEcZ

#include <gecode/int.hh>
#include <gecode/search.hh>

namespace GcVariables {

  namespace GC = Gecode;

  void out(std::ostream& o, const GC::IntVarArray& V) {
    for (int v = 0; v < V.size(); ++v) {
      o << v << ":";
      for (GC::IntVarValues j(V[v]); j(); ++j) o << " " << j.val();
      o << "\n";
    }
  }

  bool empty(const GC::IntVarArray& V) noexcept {
    for (int i = 0; i < V.size(); ++i) if (not V[i].assigned()) return false;
    return true;
  }

  size_t assignedvars(const GC::IntVarArray& V) noexcept {
    size_t assigned = 0;
    for (int v = 0; v < V.size(); ++v)
      if (V[v].size() == 1) ++assigned;
    return assigned;
  }

  size_t sumdomsizes(const GC::IntVarArray& V) noexcept {
    size_t sum = 0;
    for (int v = 0; v < V.size(); ++v) sum += V[v].size();
    return sum;
  }

  int assignedval(const GC::IntVarArray& V, const int v) noexcept {
    assert(V.size() > 0 and v < V.size());
    const GC::Int::IntView view = V[v];
    assert(view.assigned());
    const GC::IntVarValues j(view);
    return j.val();
  }
  typedef std::vector<int> values_t;
  values_t values(const GC::IntVarArray& V, const int v) {
    assert(v >= 0 and v < V.size());
    values_t res;
    for (GC::IntVarValues j(V[v]); j(); ++j) res.push_back(j.val());
    assert(res.size() == size_t(V[v].size()));
    return res;
  }
  typedef std::vector<int> solutions_t;
  solutions_t extract(const GC::IntVarArray& V) {
    const size_t N = V.size();
    solutions_t res; res.reserve(N);
    for (size_t v = 0; v < N; ++v) {
      const GC::Int::IntView view = V[v];
      assert(view.assigned());
      const GC::IntVarValues j(view);
      res.push_back(j.val());
    }
    return res;
  }

}

#endif
