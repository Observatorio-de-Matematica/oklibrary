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

Bicliques> ./GraphGen clique 16 | ./BCCbySAT 5 "" "" "" "" "" ""
# "./BCCbySAT" "5" "" "" "" "" "" ""
# "" 0
bcc=4
exact 4 5
100 : 1 1 1; 0
1 2 8 10 11 13 14 15 | 3 4 5 6 7 9 12 16
1 2 3 4 6 7 8 10 | 5 9 11 12 13 14 15 16
2 4 7 9 10 11 14 16 | 1 3 5 6 8 12 13 15
1 4 6 10 12 13 14 16 | 2 3 5 7 8 9 11 15

One can investigate the symmetry-breaking as preprocessing:
Bicliques> ./GraphGen grid 10 11 | ./BCC2SAT 0 "" -cs 3000000 ""
c sb-stats                              3000000 : 33 41.0195 50; 1.7225
c sb-seed                               2128577
Then one can provide this sb-sequence (here with showing the log directly
on standard output):
Bicliques> time ./GraphGen grid 10 11 | ./BCCbySAT 55 "" 1 "" 2128577 Stats /dev/stdout
bcc=55
exact 55 55
real	0m1.198s
user	0m1.200s
sys	0m0.029s

As the first line of Log shows:
Symmetry-breaking: 1 : 50 50 50; 0
the good symmetry-breaking (found with the more extensive search) was used.
The solution-statistics:
Bicliques> cat Stats
55   1 23045 709479  0.07  0.27  0.03   3   351  1021   4126     0  12003       98340      286082  16894  4.84  62 0.343748
54   0 22626 696583  0.07  0.26  0.02  66 19625 31752 113668     0 183909 4.49213e+06 7.26803e+06 545464 34.75  59 0.618067

Even with sb=49 the unsat-result takes much longer.


The above were biclique-cover-problems; a partition-problem:
Bicliques> ./GraphGen clique 6 | ./BCCbySAT 6 partition2 "" "" "" "" ""
# "./BCCbySAT" "6" "partition2" "" "" "" "" ""
# "" 0
bcp=5
exact 5 6
100 : 1 1 1; 0
1 5 6 | 2 4
6 | 1 5
3 | 1 2 5 6
4 | 2 3
5 | 1


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
        "0.7.2",
        "9.4.2023",
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
         << " B algo-options sb-rounds timeout seeds stats log\n\n"
    " B              : " << "[+]biclique-cover-size, default is \"+"
         << default_B << "\"\n"
    " algo-options   : " << Environment::WRP<SB>{} << "\n"
    "                : " << Environment::WRP<PT>{} << "\n"
    "                : " << Environment::WRP<DI>{} << "\n"
    "                : " << Environment::WRP<SO>{} << "\n"
    " sb-rounds      : " << "default is " << default_sb_rounds << "\n"
    " timeout        : " << "in s, default is " << default_sec << "\n"
    " seeds          : " << "sequence, can contain \"t\" or \"r\"" << "\n"
    " stats          : " << "filename for solving-stats, default is null\n\n"
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

  if (argc != 8) {
    std::cerr << error <<
      "Exactly seven arguments (B, algo-opt, sb-rounds, timeout, seeds, stats, log)"
      " needed, but " << argc-1 << " provided.\n";
    return int(Error::missing_parameters);
  }

  const auto bounds0 = read_bounds(argv[1]);
  if (not bounds0) {
    std::cerr << error <<
      "Bounds-argument faulty.\n";
    return int(Error::faulty_parameters);
  }
  const alg2_options_t algopt =
    Environment::translate<alg2_options_t>()(argv[2], sep);
  const var_t sb_rounds = read_var_t(argv[3], default_sb_rounds);
  const auto sec = read_uint_t(argv[4], default_sec);
  const RandGen::vec_eseed_t seeds = RandGen::extract_seeds(argv[5]);
  const auto [stats, statsname] = read_stats(argv[6], proginfo.prg, error);
  const auto log = read_log(argv[7], error);

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

  commandline_output(std::make_tuple(DC::with), comment, std::cout, argc, argv);
  const auto G = Graphs::make_AdjVecUInt(std::cin, Graphs::GT::und);
  BC2SAT T(G, bounds0.value());
  const auto res = T.sat_solve(log.pointer(), algopt, sb_rounds, sec, seeds);
  log.close();

  std::cout << comment << "\"" << statsname << "\" " << stats.is_cout << "\n";
  res.output(std::cout, G, stats.pointer());
  stats.close();

}
