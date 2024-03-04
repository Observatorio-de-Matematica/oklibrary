// Oliver Kullmann, 3.3.2024 (Swansea)
/* Copyright 2024 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  SAT translation of cyclic pandiagonal squares (refining
  Pandiagonal.cpp)

  "Cyclic" here means simple that for every every cell with value x
  the cyclically next cell ("wrapping around") has value x+1 mod N.
  This is called "horizontally semicyclic" in https://oeis.org/A071607 .

TODOS:

1. Due to the symmetry, only one diagonal and one antidiagonal constraint
   is needed here.


EXAMPLES:

Viewing the four (cyclic) pandiagonal Sudokus for N=11:

LatinSquares> n=11; ./CPandiagonal +$N "" | clasp 0 | ./CP_clasp_first_columns.awk -v N=$N | ./CP_clasp_expand.awk -v N=$N
  0  1  2  3  4  5  6  7  8  9 10
  4  5  6  7  8  9 10  0  1  2  3
  8  9 10  0  1  2  3  4  5  6  7
  1  2  3  4  5  6  7  8  9 10  0
  5  6  7  8  9 10  0  1  2  3  4
  9 10  0  1  2  3  4  5  6  7  8
  2  3  4  5  6  7  8  9 10  0  1
  6  7  8  9 10  0  1  2  3  4  5
 10  0  1  2  3  4  5  6  7  8  9
  3  4  5  6  7  8  9 10  0  1  2
  7  8  9 10  0  1  2  3  4  5  6

  0  1  2  3  4  5  6  7  8  9 10
  7  8  9 10  0  1  2  3  4  5  6
  3  4  5  6  7  8  9 10  0  1  2
 10  0  1  2  3  4  5  6  7  8  9
  6  7  8  9 10  0  1  2  3  4  5
  2  3  4  5  6  7  8  9 10  0  1
  9 10  0  1  2  3  4  5  6  7  8
  5  6  7  8  9 10  0  1  2  3  4
  1  2  3  4  5  6  7  8  9 10  0
  8  9 10  0  1  2  3  4  5  6  7
  4  5  6  7  8  9 10  0  1  2  3

  0  1  2  3  4  5  6  7  8  9 10
  8  9 10  0  1  2  3  4  5  6  7
  5  6  7  8  9 10  0  1  2  3  4
  2  3  4  5  6  7  8  9 10  0  1
 10  0  1  2  3  4  5  6  7  8  9
  7  8  9 10  0  1  2  3  4  5  6
  4  5  6  7  8  9 10  0  1  2  3
  1  2  3  4  5  6  7  8  9 10  0
  9 10  0  1  2  3  4  5  6  7  8
  6  7  8  9 10  0  1  2  3  4  5
  3  4  5  6  7  8  9 10  0  1  2

  0  1  2  3  4  5  6  7  8  9 10
  3  4  5  6  7  8  9 10  0  1  2
  6  7  8  9 10  0  1  2  3  4  5
  9 10  0  1  2  3  4  5  6  7  8
  1  2  3  4  5  6  7  8  9 10  0
  4  5  6  7  8  9 10  0  1  2  3
  7  8  9 10  0  1  2  3  4  5  6
 10  0  1  2  3  4  5  6  7  8  9
  2  3  4  5  6  7  8  9 10  0  1
  5  6  7  8  9 10  0  1  2  3  4
  8  9 10  0  1  2  3  4  5  6  7


One pandiagonal Sudoku for N=25:
LatinSquares> N=25; ./CPandiagonal +$N "" | clasp -t 1 | ./CP_clasp_first_columns.awk -v N=$N | ./CP_clasp_expand.awk -v N=$N
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9
  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4
 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13
  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8
 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3
 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21
 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1
  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6
 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11
  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7
 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12
 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2
 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22
 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0
 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5  6  7  8  9 10
  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24  0  1  2  3  4  5

Remark: the same chain of scripts works also for cadical (then only one
solution is extracted; also for OKsolver), e.g.
LatinSquares> N=29; ./CPandiagonal +$N "" | cadical | ./CP_clasp_first_columns.awk -v N=$N | ./CP_clasp_expand.awk -v N=$N
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9
 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4
  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7
 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12
 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0
 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13
  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2
  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8
 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3
 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22
 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11
 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5
 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10
 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21
 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1
  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28  0  1  2  3  4  5  6
 25 26 27 28  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24

Indeed seems OKsolver more effective than Cadical; for example in rather short time:
LatinSquares> N=37; ./CPandiagonal $N "" > TEMP$N; OKsolver2002 -O TEMP$N | ./CP_clasp_first_columns.awk -v N=$N | ./CP_clasp_expand.awk -v N=$N; rm TEMP$N
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36
  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1
  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0
 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6
 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11
 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9
 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33
  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8
 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22
 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12
 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7
 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10
 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29
  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4
 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13
 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3
 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21
 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5
 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
 28 29 30 31 32 33 34 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36  0  1  2
 35 36  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34



Only counting:

The following sequence is
  https://oeis.org/A071607
  Number of strong complete mappings of the cyclic group Z_{2n+1}
for N=1,3,5,7,9,11, ... :
  1, 0, 2, 4, 0, 8, 348, 0, 8276, 43184, 0, 5602176, 78309000, 0, 20893691564, 432417667152, 0
LatinSquares> for (( i=11; i <=20; ++i)); do echo $i; ./CPandiagonal $i "" | clasp 0 -q -t 6 | awk '/^c Models/;/^c CPU/'; done
11
c Models         : 8
c CPU Time       : 0.007s
12
c Models         : 0
c CPU Time       : 0.020s
13
c Models         : 348
c CPU Time       : 0.029s
14
c Models         : 0
c CPU Time       : 0.148s
15
c Models         : 0
c CPU Time       : 0.821s
16
c Models         : 0
c CPU Time       : 0.964s
17
c Models         : 8276
c CPU Time       : 6.982s
18
c Models         : 0
c CPU Time       : 17.298s
19
c Models         : 43184
c CPU Time       : 204.995s
20
c Models         : 0
c CPU Time       : 259.121s


LatinSquares> for (( i=11; i <=20; ++i)); do echo $i; ./CPandiagonal +$i "" | clasp 0 -q | awk '/^c Models/;/^c CPU/'; done
11
c Models         : 4
c CPU Time       : 0.004s
12
c Models         : 0
c CPU Time       : 0.010s
13
c Models         : 112
c CPU Time       : 0.014s
14
c Models         : 0
c CPU Time       : 0.035s
15
c Models         : 0
c CPU Time       : 0.107s
16
c Models         : 0
c CPU Time       : 0.032s
17
c Models         : 28
c CPU Time       : 0.085s
18
c Models         : 0
c CPU Time       : 0.380s
19
c Models         : 30
c CPU Time       : 1.046s
20
c Models         : 0
c CPU Time       : 4.189s

*/

#include <iostream>

#include <ProgramOptions/Environment.hpp>

#include "PQOptions.hpp"
#include "PQEncoding.hpp"
#include "Commandline.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.0.9",
        "3.3.2024",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/LatinSquares/CPandiagonal.cpp",
        "GPL v3"};

  const std::string error = "ERROR[" + proginfo.prg + "]: ";
  constexpr int commandline_args = 2;

  using namespace Commandline;
  using namespace PQOptions;
  using namespace PQEncoding;

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg <<
      " [+]N constraint-type\n\n"
      " - N               : unsigned integer\n"
      " - constraint-type : " << Environment::WRPO<CT>{} << "\n\n" <<
      "Here\n"
      "  - \"+\" adds the (generalised) Sudoku-conditions\n"
      "  - for options the first possibility is the default, "
        "triggered by the empty string.\n\n"
;
    return true;
  }

}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  if (argc != commandline_args + 1) {
    std::cerr << error << "Exactly " << commandline_args << " command-line"
      " arguments needed, but the real number is " << argc-1 << ".\n";
    return 1;
  }

  const auto [N, sudoku] = read_dim(argv[1], error);
  const auto ct0 = Environment::read<CT>(argv[2]);
  if (not ct0) {
    std::cerr << error << "The constraint-type could not be read from"
        " string \"" << argv[2] << "\".\n";
    return 1;
  }
  const CT ct = ct0.value();

  const CEncoding enc(N, ct, sudoku);

  std::cout << Environment::Wrap(proginfo, Environment::OP::dimacs);
  using Environment::DHW;
  using Environment::DWW;
  std::cout << DHW{"Parameters"}
            << DWW{"command-line"};
  Environment::args_output(std::cout, argc, argv);
  std::cout << "\n"
            << DWW{"N"} << N << "\n"
            << DWW{"Constraint_type"} << ct << "\n"
            << DWW{"box-constraint"} << sudoku << "\n";
  if (sudoku) {
    std::cout << DWW{"  b,q,r"} << enc.b << " " << enc.q << " "
              << enc.r << "\n"
              << DWW{"  main,sides,corner"};
    PEncoding::output(std::cout, enc.boxes);
    std::cout << std::endl;
  }

  cpandiagonal(std::cout, enc, sudoku);
}
