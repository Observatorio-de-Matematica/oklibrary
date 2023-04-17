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
# sb-option                             basic-sb
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

One can investigate the symmetry-breaking as preprocessing:
Bicliques> ./GraphGen grid 10 11 | ./BCC2SAT 0 "" -cs 3000000 ""
c sb-stats                              3000000 : 33 41.0195 50; 1.7225
c sb-seed                               2128577
Then one can provide this sb-sequence:
Bicliques> time ./GraphGen grid 10 11 | ./BCCbySAT 55 "" "" "" 1 2128577 Stats ""
# "./BCCbySAT" "55" "" "" "" "1" "2128577" "Stats" ""
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
# statistics-output                     Stats
# log-output                            null
# ** Results **
# sb-stats                              1 : 50 50 50; 0
# result-type                           exact
# bcc                                   = 55
real	0m0.755s
user	0m0.742s
sys	0m0.029s


As the solution-statistics shows:
Bicliques> cat Stats
 B sat  maxn      c ptime stime elimc rts   cfs cfsps    dec decpr  decps          r1        r1ps    cfl cflpd mem        t
54   0 22626 696583  0.06  0.26  0.02  66 19625 31010 113668     0 179608 4.49213e+06 7.09807e+06 545464 34.75  61 0.632866

no solution was produced (since not needed).
In order to produce a solution:
Bicliques> time ./GraphGen grid 10 11 | ./BCCbySAT 56 "" "" "" 1 2128577 Stats ""
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
Bicliques> ./GraphGen clique 6 | ./BCCbySAT 6 partition2 "" "" "" "" STATS ""
# "./BCCbySAT" "6" "partition2" "" "" "" "" "STATS" ""
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
# statistics-output                     STATS
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
Bicliques> cat STATS
B sat maxn   c ptime stime elimc rts   cfs  cfsps   dec decpr  decps     r1        r1ps    cfl cflpd mem        t
5   1  135 645     0     0     0   2   107  66708   217     0 135287   2346 1.46259e+06   1526  2.43  25 0.001604
4   0  108 489     0     0     0  70 20225 320864 24368     0 386591 424705 6.73782e+06 195719 27.67  25 0.063033


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
        "0.9.0",
        "17.4.2023",
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
      DWW{"sb-option"} << std::get<SB>(algopt) << "\n" <<
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
