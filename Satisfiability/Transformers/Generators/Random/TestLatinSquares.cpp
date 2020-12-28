// Oliver Kullmann, 18.12.2020 (Swansea)
/* Copyright 2020 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

#include <iostream>
#include <sstream>

#include <cassert>

#include <ProgramOptions/Environment.hpp>

#include "LatinSquares.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.2.6",
        "28.12.2020",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/Random/TestLatinSquares.cpp",
        "GPL v3"};

  using namespace LatinSquares;

}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv))
  return 0;

  {assert(not valid_basic(ls_row_t{},0));
   assert(not valid_basic(ls_row_t{},1));
   assert(not valid_basic(ls_row_t{0},0));
   assert(valid_basic(ls_row_t{0},1));
   assert(not valid_basic(ls_row_t{0},2));
   assert((valid_basic(ls_row_t{0,1},2)));
   assert((not valid_basic(ls_row_t{0,2},2)));
   assert((valid_basic(ls_row_t{0,0},2)));
  }

  {assert(not valid_basic_partial(ls_row_t{},0));
   assert(not valid_basic_partial(ls_row_t{},1));
   assert(not valid_basic(ls_row_t{0},0));
   assert(valid_basic_partial(ls_row_t{0},1));
   assert(not valid_basic_partial(ls_row_t{0},2));
   assert((valid_basic_partial(ls_row_t{0,1},2)));
   assert((valid_basic_partial(ls_row_t{0,2},2)));
   assert((valid_basic_partial(ls_row_t{0,0},2)));
  }

  {assert(all_different(ls_row_t{0}));
   assert((all_different(ls_row_t{0,1})));
   assert((not all_different(ls_row_t{0,0})));
   assert((not all_different(ls_row_t{1,1})));
   assert((all_different(ls_row_t{2,1,0})));
   assert((not all_different(ls_row_t{2,1,2})));
  }

  {assert(all_different_partial(ls_row_t{0}));
   assert((all_different_partial(ls_row_t{0,1})));
   assert((not all_different_partial(ls_row_t{0,0})));
   assert((not all_different_partial(ls_row_t{1,1})));
   assert((all_different_partial(ls_row_t{2,1,0})));
   assert((not all_different_partial(ls_row_t{2,1,2})));
   assert((all_different_partial(ls_row_t{4,1,4,2})));
   assert((not all_different_partial(ls_row_t{4,1,4,1})));
  }

  {assert((valid(ls_t{{0}})));
   assert((valid(ls_t{{0,1},{1,0}})));
   assert((valid(ls_t{{0,1,2},{2,0,1},{1,2,0}})));
   assert((not valid(ls_t{{0,1,2},{2,0,1},{0,2,1}})));
  }

  {assert((valid_partial(ls_t{{0}})));
   assert((valid_partial(ls_t{{0,1},{1,0}})));
   assert((valid_partial(ls_t{{0,1,2},{2,0,1},{1,2,0}})));
   assert((not valid_partial(ls_t{{0,1,2},{2,0,1},{3,2,1}})));
   assert((valid_partial(ls_t{{0,1,2},{2,0,1},{3,2,3}})));
  }


  {assert(valid_basic(first_basic(1)));
   assert(valid_basic(first_basic(2)));
   ls_t L = first_basic(3);
   assert((L == ls_t{{0,0,0},{0,0,0},{0,0,0}}));
   assert(next_basic(L));
   assert((L == ls_t{{1,0,0},{0,0,0},{0,0,0}}));
   assert(next_basic(L));
   assert((L == ls_t{{2,0,0},{0,0,0},{0,0,0}}));
   assert(next_basic(L));
   assert((L == ls_t{{0,1,0},{0,0,0},{0,0,0}}));
  }

  {for (ls_dim_t N = 1; N <= 3; ++N)
     assert(trivial_count_all_ls(N) == all_ls[N]);
  }

  {assert(standard(0) == ls_row_t{});
   assert(standard(1) == ls_row_t{0});
   assert((standard(2) == ls_row_t{0,1}));
  }

  {assert(cyclic_ls(1) == ls_t{{0}});
   assert((cyclic_ls(2) == ls_t{{0,1},{1,0}}));
   assert((cyclic_ls(3) == ls_t{{0,1,2},{1,2,0},{2,0,1}}));
  }

  {assert((find_first_duplication({0}) == index_pair_t{1,1}));
   assert((find_first_duplication({0,1}) == index_pair_t{2,2}));
   assert((find_first_duplication({0,0}) == index_pair_t{0,1}));
   assert((find_first_duplication({0,1,0}) == index_pair_t{0,2}));
   assert((find_first_duplication({0,1,1,0}) == index_pair_t{1,2}));
  }

  {assert(transpose({}) == ls_t{});
   assert(transpose({{1}}) == ls_t{{1}});
   assert((transpose({{1,2},{3,4}}) == ls_t{{1,3},{2,4}}));
   assert((transpose({{1,2,3},{4,5,6},{7,8,9}}) == ls_t{{1,4,7},{2,5,8},{3,6,9}}));
   for (ls_dim_t N = 0; N <= 10; ++N) {
     ls_t L = empty_ls(N);
     assert(transpose(L) == L);
     if (N == 0) continue;
     L = cyclic_ls(N);
     assert(transpose(L) == L);
   }
  }

  {assert(has_standardised_first_column({}));
   assert(has_standardised_first_column({{0}}));
   assert(not has_standardised_first_column({{1}}));
   assert(has_standardised_first_column({{0,0},{1},{2}}));
   assert(not has_standardised_first_column({{0,0},{1},{2},{2}}));
   assert(has_standardised_first_column(empty_ls(0)));
   assert(not has_standardised_first_column(empty_ls(1)));
   for (ls_dim_t N = 1; N <= 10; ++N)
     assert(has_standardised_first_column(cyclic_ls(N)));
  }

  {assert(standardise_first_column({}) == ls_t{});
   assert(standardise_first_column({{0}}) == ls_t{{0}});
   assert((standardise_first_column({{0,0},{2},{1}}) == ls_t{{0,0},{1},{2}}));
   assert((standardise_first_column({{0,0,0},{0},{0}}) == ls_t{{0},{},{}}));
   assert((standardise_first_column({{0,2,1},{0,0,0},{0,0,0}}) == ls_t({{0,0,0},{},{}})));
   for (ls_dim_t N = 1; N <= 10; ++N) {
     const ls_t L = cyclic_ls(N);
     assert(standardise_first_column(L) == L);
   }
  }

  {assert(has_standardised_first_row({}));
   assert(has_standardised_first_row({{0}}));
   assert(not has_standardised_first_row({{1}}));
   assert(has_standardised_first_row({{0,1,2,3,4},{}}));
   assert(not has_standardised_first_row({{0,1,2,7,4},{}}));
   for (ls_dim_t N = 1; N <= 10; ++N)
     assert(has_standardised_first_row(cyclic_ls(N)));
  }

  {assert(standardise_first_row({}) == ls_t{});
   assert(standardise_first_row({{0}}) == ls_t{{0}});
   assert((standardise_first_row({{2,1,0},{7,77,777},{8,88,888}}) == ls_t({{0,1,2},{777,77,7},{888,88,8}})));
   for (ls_dim_t N = 1; N <= 10; ++N) {
     const ls_t L = cyclic_ls(N);
     assert(standardise_first_row(L) == L);
   }
  }


  {assert(valid(Set{},0));
   assert(valid(Set{{0}},1));
   assert(valid(Set{{0,1}},2));
   assert(not valid(Set{{1,0}},2));
   assert(valid(Set{{2,4,6}},10));
   assert(not valid(Set{{2,1,6}},10));
   assert(not valid(Set{{11}},10));
   assert(not valid(Set{{1,1}},10));
  }

  {Set s{{1,3,7,9,20}};
   assert(valid(s,21));
   assert(not valid(s,20));
   assert(s.size() == 5);
   assert(s.contains(7));
   s.remove(7);
   assert(s.size() == 4);
   assert(not s.contains(7));
   assert((s.s == set_t{1,3,9,20}));
  }

  {assert(valid(SetSystem{}));
   assert(valid(SetSystem{{{{0}}}}));
   assert(not valid(SetSystem{{{{1}}}}));
   assert(valid(SetSystem{{{{0,1}},{{1}}}}));
   assert(valid(SetSystem{{{{3}},{{}},{{}},{{}}}}));
  }

  {assert(is_sdr({},{}));
   assert((is_sdr({0},{{{{0}}}})));
   assert((is_sdr({2,1,0},{{{{0,2}},{{1}},{{0,2}}}})));
   assert((not is_sdr({2,1,0},{{{{0,2}},{{1}},{{1,2}}}})));
  }

  {assert(is_psdr({},{}));
   assert((is_psdr({0},{{{{0}}}})));
   assert((is_psdr({2,1,0},{{{{0,2}},{{1}},{{0,2}}}})));
   assert((not is_psdr({2,1,0},{{{{0,2}},{{1}},{{1,2}}}})));
   assert((is_psdr({2,1,3},{{{{0,2}},{{1}},{{1,2}}}})));
  }

  {PBij b(3); std::stringstream s;
    assert(b.total_size()==3 and b.size()==0 and b.empty() and not b.total()
           and b(0)==3 and b[0]==3);
   s << b; assert(s.str() == "* * *;* * *"); s.str("");
   assert(b.set(0,2));
   assert(b.size()==1 and not b.empty() and not b.total() and
          b(0)==2 and b[2]==0 and b(1)==3 and b[0]==3);
   s << b; assert(s.str() == "2 * *;* * 0"); s.str("");
   assert(not b.set(0,2));
   assert(not b.set(1,2));
   assert(b.size()==1 and not b.empty() and not b.total() and
          b(0)==2 and b[2]==0 and b(1)==3 and b[0]==3);
   assert(b.set(1,1));
   assert(b.size()==2 and not b.empty() and not b.total() and
          b(0)==2 and b(1)==1 and b(2)==3 and
          b[2]==0 and b[1]==1 and b[0]==3);
   s << b; assert(s.str() == "2 1 *;* 1 0"); s.str("");
   assert(not b.set(2,2));
   assert(not b.set(2,1));
   assert(b.set(2,0));
   assert(b.size()==3 and not b.empty() and b.total() and
          b(0)==2 and b(1)==1 and b(2)==0 and
          b[2]==0 and b[1]==1 and b[0]==2);
   s << b; assert(s.str() == "2 1 0;2 1 0");
  }

  {RG::randgen_t g;
   assert(random_psdr({{{{0}}}}, g).r() == ls_row_t{0});
   assert((random_psdr({{{{0}},{{2}},{{1}}}}, g).r() == ls_row_t{0,2,1}));
   assert((random_psdr({{{{}},{{0}},{{}}, {{2}},{{1}}}}, g).r() == ls_row_t{5,0,5,2,1}));
   assert((random_psdr({{{{0,1,2,3,4}},{{0,1,2,3,4}},{{0}}, {{0,1,2,3,4}},{{1}}}}, g).r() == ls_row_t{4,2,0,3,1}));

   SetSystem S{{{{0,1,2,3,4}},{{0,1,2,3,4}},{{0}},{{0,1,2,3,4}},{{1}}}};
   assert(valid(S));
   const PBij p = random_psdr(S,g);
   assert((p.r() == ls_row_t{2,1,0,3,5}));
   assert(p.total_size() == 5);
   assert(p.size() == 4);
   remove_psdr(p, S);
   assert(valid(S));
   assert((S.S == setsystem_t{{{0,1,3,4}},{{0,2,3,4}},{{}},{{0,1,2,4}},{{1}}}));
  }

  {RG::randgen_t g;
   const ls_dim_t N = 10;
   SetSystem S(N);
   assert(valid(S));
   for (unsigned i = 0; i < 200; ++i)
     assert(random_psdr(S,g).size() == N);
  }

  {RG::randgen_t g;
   const ls_dim_t max_N = 10;
   const unsigned T = 100;
   for (ls_dim_t N = 1; N <= max_N; ++N)
     for (unsigned i = 0; i < T; ++i) {
       const auto P = random_pls(N, g);
       assert(valid_partial(P.L));
       assert(P.rows_completed >= 1 and P.rows_completed <= N);
       assert(valid(P.A));
       assert(P.rows_completed == N or is_psdr(P.next.r(), P.A));
     }
  }

  {// XXX test some concrete values

  }

}
