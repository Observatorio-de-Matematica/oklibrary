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

Bicliques> ./GraphGen clique 16 | ./BCCbySAT 5 "" "" "" ""
Symmetry-breaking: 100 : 1 1 1; 0
Minisat-call for B=5: returned SAT
  Literal-Reduction by trimming: 1
  Size obtained: 5
Minisat-call for B=4: returned SAT
  Literal-Reduction by trimming: 0
  Size obtained: 4
Minisat-call for B=3: returned UNSAT

bcc=4
exact 4 5
1 2 5 6 13 14 15 16 | 3 4 7 8 9 10 11 12
1 2 3 4 5 10 12 14 | 6 7 8 9 11 13 15 16
2 3 7 8 12 13 14 15 | 1 4 5 6 9 10 11 16
1 3 4 7 9 14 15 16 | 2 5 6 8 10 11 12 13



TODOS:

1. Output seeds
    - It seems parameter-output is needed.
    - This should include symmetry-breaking.

2. DONE Read seeds

3. Supply format-options

4. More systematic output:
    - Explain the special case where no unsat-test
      is needed due to the results of symmetry-breaking.
    - Explain the output-line "exact 4 4".
    - Control logging.
    - Output symmetry-breaking statistics.

5. Provide a mode which starts with B as given by symmetry-breaking
   (which is a sound lower bound), and then increases B as long as
   unsat was found.
    - This needs an extension of operator-() in BC2SAT.
    - As a further option it should be possible, for activated symmetry-
      breaking, to run internal UCP (without intermediate file-output).

*/


#include <iostream>

#include <ProgramOptions/Environment.hpp>
#include <Transformers/Generators/Random/Numbers.hpp>

#include "Graphs.hpp"
#include "Bicliques2SAT.hpp"

#include "BCC2SAT.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.3.1",
        "7.3.2023",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Transformers/Generators/Bicliques/BCCbySAT.cpp",
        "GPL v3"};

  using namespace Bicliques2SAT;
  using namespace BCC2SAT;

  const std::string error = "ERROR[" + proginfo.prg + "]: ";

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg
         << " B algo-options sb-rounds timeout seeds\n\n"
    " B              : " << "biclique-cover-size, default is "
         << default_B << "\n"
    " algo-options   : " << Environment::WRP<SB>{} << "\n"
    " sb-rounds      : " << "default is " << default_sb_rounds << "\n"
    " timeout        : " << "in s, default is " << default_sec << "\n"
    " seeds          : " << "sequence, can contain \"t\" or \"r\"" << "\n\n"
    " reads a graph from standard input, and attempts to compute its bcc-number:\n\n"
    "  - Arguments \"\" (the empty string) yield the default-values.\n"
    "  - Default-values for the options are the first possibilities given.\n\n"
;
    return true;
  }

}

int main(const int argc, const char* const argv[]) {

  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  if (argc != 6) {
    std::cerr << error <<
      "Exactly five arguments (B, algo-opt, sb-rounds, timeout, seeds)"
      " needed, but only " << argc-1 << " provided.\n";
    return int(Error::missing_parameters);
  }

  const var_t B = read_var_t(argv[1], default_B);
  const alg_options_t algopt =
    Environment::translate<alg_options_t>()(argv[2], sep);
  const var_t sb_rounds = argc >= 4 ?
    read_var_t(argv[3], default_sb_rounds) : default_sb_rounds;
  const auto sec = argc >= 5 ?
    read_uint_t(argv[4], default_sec) : default_sec;
  const RandGen::vec_eseed_t seeds = RandGen::extract_seeds(argv[5]);

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

  const auto G = Graphs::make_AdjVecUInt(std::cin, Graphs::GT::und);
  BC2SAT trans(G, B);
  const auto res = trans(&std::cout, algopt, sb_rounds, sec, seeds);
  std::cout << "\n"; // separation from log-output
  res.output(std::cout, G);

}
