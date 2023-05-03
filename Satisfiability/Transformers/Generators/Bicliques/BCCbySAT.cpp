// Oliver Kullmann, 6.3.2022 (Swansea)
/* Copyright 2022, 2023 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*
  Solving biclique-covering problems by SAT-solving, computing bcc(G)
  for an input-graph G

  (BCC2SAT computes one single SAT-translation, while this program
   attempts to run a complete chain of SAT-solving, starting with a given
   upper bound B on the number of bicliqes, and finishing with outputting
   an optimal solution.)


EXAMPLES:

Bicliques> ./GraphGen clique 16 | ./BCCbySAT 5 "" "" "" "" "" "" ""
# "./BCCbySAT" "5" "" "" "" "" "" "" ""
# ** Parameters **
# B                                     downwards 0 5
# sb-options                            basic-sb with-ssb
# pt-option                             cover
# di-option                             downwards
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             100
# num_e-seeds                           0
# statistics-output                     null
# log-output                            null
# ** Results **
# sb-stats                              100 : 1 1 1; 0
# result-type                           exact
# bcc                                   = 4
1 2 8 10 11 13 14 15 | 3 4 5 6 7 9 12 16
1 2 3 4 6 7 8 10 | 5 9 11 12 13 14 15 16
2 4 7 9 10 11 14 16 | 1 3 5 6 8 12 13 15
1 4 6 10 12 13 14 16 | 2 3 5 7 8 9 11 15

Without specifying an upper bound, and moving upwards:
Bicliques> ./GraphGen clique 16 | ./BCCbySAT "" up "" "" "" "" ST ""
# "./BCCbySAT" "" "up" "" "" "" "" "ST" ""
# ** Parameters **
# B                                     upwards +0 18446744073709551615
# sb-options                            basic-sb with-ssb
# pt-option                             cover
# di-option                             upwards
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             100
# num_e-seeds                           0
# statistics-output                     ST
# log-output                            null
# ** Results **
# sb-stats                              100 : 1 1 1; 0
# result-type                           exact
# bcc                                   = 4
1 2 8 10 11 13 14 15 | 3 4 5 6 7 9 12 16
1 2 3 4 6 7 8 10 | 5 9 11 12 13 14 15 16
2 4 7 9 10 11 14 16 | 1 3 5 6 8 12 13 15
1 4 6 10 12 13 14 16 | 2 3 5 7 8 9 11 15

We note that the bounds shown are before adjustment.

Bicliques> cat ST
B sat maxn    c ptime stime elimc rts  cfs  cfsps  dec decpr  decps    r1        r1ps   cfl cflpd mem        t
1   0  152  736     0     0     0   0    0    nan    0   nan    nan   138         inf     0   nan  25        0
2   0  304 1592     0     0  0.01   1   24   6778   58     0  16380   413      116634    64  8.57  25 0.003541
3   0  456 2328     0     0  0.01  15 2460 214081 3590     0 312418 61756 5.37429e+06 16967 18.46  25 0.011491
4   1  608 3064     0     0  0.01   4  427 188771 1062     0 469496 21953 9.70513e+06  8667  6.89  25 0.002262

Using the simple upper bound V-1, and using a different seed:
Bicliques> ./GraphGen clique 16 | ./BCCbySAT +0,15 binsearch "" "" "" 0 ST ""
# "./BCCbySAT" "+0,15" "binsearch" "" "" "" "0" "ST" ""
# ** Parameters **
# B                                     binsearch +0 15
# sb-options                            basic-sb with-ssb
# pt-option                             cover
# di-option                             binsearch
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             100
# num_e-seeds                           1
#  e-seeds                              0
# statistics-output                     ST
# log-output                            null
# ** Results **
# sb-stats                              100 : 1 1 1; 0
# result-type                           exact
# bcc                                   = 4
1 3 5 9 10 12 13 15 | 2 4 6 7 8 11 14 16
1 3 4 7 11 13 15 16 | 2 5 6 8 9 10 12 14
1 2 4 6 7 9 10 15 | 3 5 8 11 12 13 14 16
1 2 4 9 12 13 14 16 | 3 5 6 7 8 10 11 15
Bicliques> cat ST
B sat maxn    c ptime stime elimc rts  cfs  cfsps  dec decpr  decps     r1        r1ps   cfl cflpd mem        t
8   1 1216 6008     0     0  0.01   2  118  22771 1637     0 315901   7748 1.49518e+06  4624  0.47  26 0.005182
4   1  608 3064     0     0  0.01   5  511 184543 1219     0 440231  26071 9.41531e+06 10873  8.17  25 0.002769
2   0  304 1592     0     0  0.01   1   33  17188   81     0  42188    467      243229    95 11.21  25  0.00192
3   0  456 2328     0     0  0.01  31 6458 224384 8788     0 305340 177216  6.1574e+06 50432 19.38  25 0.028781

Upper and lower bound will be automatically adjusted to the bounds given by symmetry-breaking resp.
the simple upper bound, and thus we get the same result by
Bicliques> ./GraphGen clique 16 | ./BCCbySAT 0,1000 binsearch "" "" "" 0 ST ""
# "./BCCbySAT" "0,1000" "binsearch" "" "" "" "0" "ST" ""
# ** Parameters **
# B                                     binsearch 0 1000
Bicliques> cat ST
B sat maxn    c ptime stime elimc rts  cfs  cfsps  dec decpr  decps     r1        r1ps   cfl cflpd mem        t
8   1 1216 6008     0     0  0.01   2  118  22846 1637     0 316941   7748  1.5001e+06  4624  0.47  26 0.005165
4   1  608 3064     0     0  0.01   5  511  98326 1219     0 234558  26071 5.01655e+06 10873  8.17  25 0.005197
2   0  304 1592     0     0  0.01   1   33  16012   81     0  39301    467      226589    95 11.21  25 0.002061
3   0  456 2328     0     0  0.01  31 6458 202147 8788     0 275081 177216 5.54719e+06 50432 19.38  25 0.031947


One can investigate the symmetry-breaking as preprocessing:
Bicliques> time ./GraphGen grid 10 11 | ./BCC2SAT 0 "" -cs 3000000 ""
c sb-stats                              3000000 : 33 41.0195 50; 1.7225
c sb-seed                               2128577
c restricted-edges                      0
c ** Statistics **
c V                                     110
c E                                     199
c B                                     50
c total-lit-occurrences                 1359400
p cnf 20950 645149
real	3m3.991s
user	3m3.879s
sys	0m0.036s

Then one can provide this sb-sequence:
Bicliques> time ./GraphGen grid 10 11 | ./BCCbySAT 55 "" "" "" 1 2128577 ST ""
# "./BCCbySAT" "55" "" "" "" "1" "2128577" "ST" ""
# ** Parameters **
# B                                     downwards 0 55
# sb-option                             basic-sb
# pt-option                             cover
# di-option                             downwards
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             1
# num_e-seeds                           1
#  e-seeds                              2128577
# statistics-output                     ST
# log-output                            null
# ** Results **
# sb-stats                              1 : 50 50 50; 0
# result-type                           exact
# bcc                                   = 55
real	0m0.755s
user	0m0.742s
sys	0m0.029s


As the solution-statistics shows:
Bicliques> cat ST
 B sat  maxn      c ptime stime elimc rts   cfs cfsps    dec decpr  decps          r1        r1ps    cfl cflpd mem        t
54   0 22626 696583  0.06  0.26  0.02  66 19625 31010 113668     0 179608 4.49213e+06 7.09807e+06 545464 34.75  61 0.632866

no solution was produced (since not needed).
In order to produce a solution:
Bicliques> time ./GraphGen grid 10 11 | ./BCCbySAT 56 "" "" "" 1 2128577 ST ""
...
1,9 2,8 2,10 3,9 | 2,9
2,2 | 2,3 3,2
3,2 4,3 | 4,2
9,5 10,4 10,6 | 10,5
real	0m1.182s
user	0m1.173s
sys	0m0.038s


Even with sb=49 the unsat-result takes much longer.


The above were biclique-cover-problems; now a partition-problem:
Bicliques> ./GraphGen clique 6 | ./BCCbySAT 6 partition2 "" "" "" "" ST ""
# "./BCCbySAT" "6" "partition2" "" "" "" "" "ST" ""
# ** Parameters **
# B                                     downwards 0 6
# sb-option                             basic-sb
# pt-option                             partition-quadratic
# di-option                             downwards
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             100
# num_e-seeds                           0
# statistics-output                     ST
# log-output                            null
# ** Results **
# sb-stats                              100 : 1 1 1; 0
# result-type                           exact
# bcp                                   = 5

No output here, since the automatic upper-bound V-1=5 is never tried:
Bicliques> cat ST
B sat maxn   c ptime stime elimc rts   cfs  cfsps   dec decpr  decps     r1        r1ps    cfl cflpd mem        t
4   0  108 489     0     0     0  70 20225 269100 24368     0 324224 424705 5.65083e+06 195719 27.67  25 0.075158

In order to overcome this, an additional isolated vertex can be added to the input-graph (which increases
that bound, but otherwise has not much influence):
Bicliques> echo "new_vertex" > FILE
Bicliques> ./GraphGen clique 6 | cat - FILE | ./BCCbySAT 6 partition2 "" "" "" "" ST ""
# "./BCCbySAT" "6" "partition2" "" "" "" "" "ST" ""
# ** Parameters **
# B                                     downwards 0 6
# sb-options                            basic-sb with-ssb
# pt-option                             partition-quadratic
# di-option                             downwards
# so-option                             default-solver
# comments-option                       with-comments
# bicliques-option                      with-bicliques
# solver-timeout(s)                     2147483647
# sb-rounds                             100
# num_e-seeds                           0
# statistics-output                     ST
# log-output                            null
# ** Results **
# sb-stats                              100 : 1 1 1; 0
# result-type                           exact
# bcp                                   = 5
1 6 | 2 4 5
2 5 | 4
2 4 6 | 3
2 3 | 5
3 6 | 1
Bicliques> cat ST
B sat maxn   c ptime stime elimc rts   cfs  cfsps   dec decpr  decps     r1        r1ps    cfl cflpd mem        t
5   1  145 710     0     0     0   2   107  63615   217     0 129013   2348 1.39596e+06   1526  2.43  25 0.001682
4   0  116 541     0     0     0  62 14462 400909 20201     0 560003 248027  6.8757e+06 114775 23.25  25 0.036073

Remarks on logging:

If a log-file is given (or /dev/stdout used), then the (complete) minisat-
output is only shown with a completed run (which may have been aborted);
in order to see the progress of the current run, in the calling directory
the file SystemCalls_Popen_out_* can be inspected.


See plans/general.txt.

*/


#include <iostream>

#include <ProgramOptions/Environment.hpp>
#include <Transformers/Generators/Random/Numbers.hpp>

#include "Graphs.hpp"
#include "Bicliques2SAT.hpp"

#include "BCC2SAT.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.9.3",
        "3.5.2023",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/Bicliques/BCCbySAT.cpp",
        "GPL v3"};

  using namespace Bicliques2SAT;
  using namespace BCC2SAT;

  const std::string error = "ERROR[" + proginfo.prg + "]: ";
  const std::string comment = "# ";

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg
         << " B algo-options format-options"
            " sb-rounds timeout seeds stats log\n\n"
    " B              : " << "[+]biclique-cover-size, default is \"+0\"\n"
    " algo-options   : " << Environment::WRP<SB>{} << "\n"
    "                : " << Environment::WRP<SS>{} << "\n"
    "                : " << Environment::WRP<PT>{} << "\n"
    "                : " << Environment::WRP<DI>{} << "\n"
    "                : " << Environment::WRP<SO>{} << "\n"
    " format-options : " << Environment::WRP<DC>{} << "\n"
    "                : " << Environment::WRP<BC>{} << "\n"
    " timeout        : " << "in s, default is " << default_sec << "\n"
    " sb-rounds      : " << "default is " << default_sb_rounds << "\n"
    " seeds          : " << "sequence, can contain \"t\" or \"r\"" << "\n"
    " stats          : " << "filename for solving-stats, default is null\n"
    " log            : " << "filename for solving-log, default is null\n\n"
    " reads a graph from standard input, and attempts to compute its"
    " bcc/bcp-number:\n\n"
    "  - Arguments \"\" (the empty string) yield the default-values.\n"
    "  - Using \"+\" for B means the increment added to the lower-bound.\n"
    "  - Default-values for the options are the first possibilities given.\n"
    "  - For stats the special value \"t\" creates an automatic name (with timestamp).\n"
    "  - By using \"/dev/stdout\" for stats/log the output goes to standard output.\n\n"
;
    return true;
  }

}

int main(const int argc, const char* const argv[]) {

  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  if (argc != 9) {
    std::cerr << error <<
      "Exactly eight arguments (B, algo-opt, form-opt, sb-rounds, timeout, seeds, stats, log)"
      " needed, but " << argc-1 << " provided.\n";
    return int(Error::missing_parameters);
  }

  const auto bounds0 = read_vecvalorinc(argv[1]);
  if (bounds0.size() > 2) {
    std::cerr << error <<
      "Bounds-argument has " << bounds0.size() << " > 2 components.\n";
    return int(Error::faulty_parameters);
  }
  const alg2_options_t algopt =
    Environment::translate<alg2_options_t>()(argv[2], sep);
  const DI di = std::get<DI>(algopt);
  const Bounds bounds = extract_bounds(di, bounds0);
  const format2_options_t formopt =
    Environment::translate<format2_options_t>()(argv[3], sep);
  const auto sec = read_uint_t(argv[4], default_sec);
  const var_t sb_rounds = read_var_t(argv[5], default_sb_rounds);
  const RandGen::vec_eseed_t seeds = RandGen::extract_seeds(argv[6]);
  const auto [stats, statsname] = read_stats(argv[7], proginfo.prg, error);
  const std::string logname = argv[8];
  const auto log = read_log(logname, error);

  if (std::get<SB>(algopt) != SB::none and sb_rounds == 0) {
    std::cerr << error <<
      "Symmetry-breaking on, but number of rounds is zero.\n";
    return int(Error::bad_sb);
  }
  if (std::get<SB>(algopt) == SB::extended) {
    std::cerr << error <<
      "Extended symmetry-breaking not implemented yet.\n";
    return int(Error::bad_sb);
  }

  const DC dc = std::get<DC>(formopt);
  const BC bc = std::get<BC>(formopt);
  if (dc == DC::with) {
    commandline_output(std::make_tuple(DC::with), comment, std::cout,
                       argc, argv);
    Environment::DWW::prefix = comment;
    using Environment::DWW; using Environment::DHW;
    std::cout <<
      DHW{"Parameters"} <<
      DWW{"B"} << bounds << "\n" <<
      DWW{"sb-options"} << std::get<SB>(algopt) << " " <<
                           std::get<SS>(algopt) << "\n" <<
      DWW{"pt-option"} << std::get<PT>(algopt) << "\n" <<
      DWW{"di-option"} << di << "\n" <<
      DWW{"so-option"} << std::get<SO>(algopt) << "\n" <<
      DWW{"comments-option"} << dc << "\n" <<
      DWW{"bicliques-option"} << bc << "\n" <<
      DWW{"solver-timeout(s)"} << sec << "\n";
    if (std::get<SB>(algopt) != SB::none) {
      std::cout <<
        DWW{"sb-rounds"} << sb_rounds << "\n" <<
        DWW{"num_e-seeds"} << seeds.size() << "\n";
      if (not seeds.empty())
        std::cout <<
          DWW{" e-seeds"} << RandGen::ESW{seeds} << "\n";
    }
    std::cout <<
      DWW{"statistics-output"};
    print(std::cout, {stats,statsname});
    std::cout <<
      DWW{"log-output"};
    print(std::cout, {log,logname});
    std::cout <<
      DHW{"Results"};
    std::cout.flush();
  }

  const auto G = Graphs::make_AdjVecUInt(std::cin, Graphs::GT::und);
  BC2SAT T(G, bounds);
  const auto res = T.sat_solve(log.pointer(), algopt, sb_rounds, sec, seeds);
  log.close();

  res.output(dc == DC::with ? &std::cout : nullptr, bc, G, stats.pointer());
  stats.close();

}
