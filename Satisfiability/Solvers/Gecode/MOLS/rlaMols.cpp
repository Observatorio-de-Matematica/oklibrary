// Oleg Zaikin, 13.4.2022 (Swansea)
/* Copyright 2022 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  A Gecode-based solver for general MOLS-LS-related problems:
   - using (only) branching strategies as provided by Gecode itself,
   - but with the lookahead-reduction.

Examples:

*/

/* BUGS:

1. Wrong count:

MOLS> ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" +count dom enumbr mindom asc relpr 12
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	+count 6247 21549.988 145501695 702831 1994566 23

while the count in Eulerinvsymmbalt is 6236.
All four forms of la-reduction show the same count:
MOLS> ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" count dom enumbr mindom asc - 12
# N=7
# k=3 total_num_sq=3: "data/SpecsCollection/Eulerinvsymmbalt"
#   num_uc=7 num_eq=0 num_peq=1
# num_ps=0: ""
# num_runs=4
# threads=12
# rt=count-solutions(count)
#   propagation: domain-prop(dom)
#   branching-type: enumerative-branching(enumbr)
#   variable-heuristic: min-dom-var(mindom)
#   order-heuristic: ascending-order(asc)
#   la-reduction-type: relaxed-pruning(relpr) relaxed-nonpruning(relnpr) eager-pruning(eagpr) eager-nonpruning(eagnpr)
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	count 6247 21076.627 145487113 702831 1994566 24
dom enumbr mindom asc relnpr 	count 6247 23443.596 145494209 702831 1994566 24
dom enumbr mindom asc eagpr 	count 6247 27755.376 145481836 702831 1994566 23
dom enumbr mindom asc eagnpr 	count 6247 34281.727 145498008 702831 1994566 23

MOLS> ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" +enum dom enumbr mindom asc relpr 1
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	+enum 6247 10076.145 145535627 702831 1994566 22
MOLS> time ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" enum dom enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	enum 6247 11190.147 145531043 702831 1994566 23
real	68m13.473s
user	187m29.102s
sys	0m48.871s


2. DONE (when rlaStats is constructed, then now atomic_bool abort is initialised)
   There seems to be an initialisation-problem with satd:

MOLS> ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" satd dom enumbr mindom asc - 1
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	satd 1 0.610 7433 36 114 17
dom enumbr mindom asc relnpr 	satd 0 0.000 202 0 0 0
dom enumbr mindom asc eagpr 	satd 0 0.000 202 0 0 0
dom enumbr mindom asc eagnpr 	satd 0 0.000 202 0 0 0
MOLS> ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" satd dom enumbr mindom asc -relpr 1
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relnpr 	satd 1 0.643 7433 36 114 17
dom enumbr mindom asc eagpr 	satd 0 0.000 202 0 0 0
dom enumbr mindom asc eagnpr 	satd 0 0.000 202 0 0 0

Always the first run is correct, the others are wrong.

*/

/* TODOS:

0. Additional to the gecode-statistics, our statistics on the reduction
   need to be provided.

1. A customised brancher with Gecode's default branching strategies is needed:
   - Handled by LookaheadBranching::RlaBranching.
   - Check via unit tests that the solver and gcMols give equal results.

*/

/*

BUGS:

*/

#include <iostream>
#include <string>
#include <ostream>
#include <fstream>

#include <cassert>

#include <ProgramOptions/Environment.hpp>
#include <Numerics/NumInOut.hpp>

#include "Conditions.hpp"
#include "Encoding.hpp"
#include "PartialSquares.hpp"
#include "Solvers.hpp"
#include "Options.hpp"
#include "CommandLine.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.4.1",
        "22.5.2022",
        __FILE__,
        "Oliver Kullmann and Oleg Zaikin",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Solvers/Gecode/MOLS/rlaMols.cpp",
        "GPL v3"};

  const std::string error = "ERROR[" + proginfo.prg + "]: ";
  constexpr int commandline_args = 10;

  using namespace Conditions;
  using namespace Encoding;
  using namespace PartialSquares;
  using namespace Solvers;
  using namespace Options;
  using namespace CommandLine;

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg <<
      " N file_cond file_ps run-type prop-level branch-type branch-var"
      " branch-order la-type threads\n\n"
      " - file_cond    : filename for conditions-specification\n"
      " - file_ps      : filename for partial-squares-specification\n"
      " - run-type     : " << Environment::WRPO<RT>{} << "\n" <<
      " - prop-level   : " << Environment::WRPO<PropO>{} << "\n" <<
      " - branch-type  : " << Environment::WRPO<BRT>{} << "\n" <<
      " - branch-var   : " << Environment::WRPO<BHV>{} << "\n" <<
      " - branch-order : " << Environment::WRPO<GBO>{} << "\n" <<
      " - la-reduction : " << Environment::WRPO<LAR>{} << "\n" <<
      " - threads      : floating-point for number of threads\n\n"
      "Here\n"
      "  - file_ps can be the empty string (no partial instantiation)\n"
      "  - the algorithmic options can be lists (all combinations)\n"
      "  - these lists can have a leading + (inclusion) or - (exclusion)\n"
      "  - for sat-solving and enumeration, output goes to file \"" <<
      "SOLUTIONS_" << proginfo.prg << "_N_timestamp\".\n\n"
;
    return true;
  }

  void rh(std::ostream& out) {
    out << "pl bt bh bo lar \t\t";
    GBasicSR::rh(out); out << std::endl;
  }

}


int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  if (argc !=  commandline_args + 1) {
    std::cerr << error << "Exactly " << commandline_args << " command-line"
      " arguments needed, but the real number is " << argc-1 << ".\n";
    return 1;
  }

  const size_t N = read_N(argc, argv);
  const auto [ac, name_ac] = read_ac(argc, argv);
  const auto [ps, name_ps] = read_ps(argc, argv, N);
  const RT rt = read_rt(argc, argv);

  const list_propo_t pov = read_opt<PropO>(argc, argv, 5, "po",
                                           "propagation");
  const list_brt_t brtv = read_opt<BRT>(argc, argv, 6, "brt",
                                        "branching-type");
  const list_bhv_t bvarv = read_opt<BHV>(argc, argv, 7, "bvar",
                                        "gc-variable-heuristics");
  const list_gbo_t gbov = read_opt<GBO>(argc, argv, 8, "gbo",
                                        "gc-order-heuristics");
  const list_lar_t larv = read_opt<LAR>(argc, argv, 9, "lar",
                                        "lookahead-reduction");
  const size_t num_runs =
    pov.size()*brtv.size()*bvarv.size()*gbov.size()*larv.size();

  const double threads = read_threads(argc, argv, 10);

  const std::string outfile = output_filename(proginfo.prg, N);

  const bool with_file_output = Options::with_file_output(rt);
  if (with_file_output and num_runs != 1) {
    std::cerr << error << "For solution-output the number of runs must be 1,"
      " but is " << num_runs << ".\n";
    return 1;
  }
  std::ostream* const out = with_file_output ?
    new std::ofstream(outfile) : nullptr;
  if (with_file_output and (not out or not *out)) {
    std::cerr << error << "Can not open file \"" << outfile << "\" for "
      "writing.\n";
    return 1;
  }

  const bool with_log = Options::with_log(rt);
  std::ostream* const log = with_log ? &std::cout : nullptr;

  info_output(std::cout,
              N, ac, name_ac, ps, name_ps, rt,
              num_runs, threads, outfile, with_file_output);
  algo_output(std::cout, std::make_tuple(pov, brtv, bvarv, gbov, larv));
  std::cout.flush();

  if (num_runs != 1) rh(std::cout); // XXX
  for (const PropO po : pov) {
    const EncCond enc(ac, ps, prop_level(po));
    for (const BRT brt : brtv)
      for (const BHV bvar : bvarv)
        for (const GBO gbo : gbov)
          for (const LAR lar : larv) {
            const rlaSR res =
              rlasolver(enc,rt,brt,bvar,gbo,lar,threads,log);
            if (with_log and
              rt != RT::enumerate_with_log and rt != RT::unique_s_with_log)
              std::cout << "\n";
            if (num_runs == 1) rh(std::cout); // XXX
            using Environment::W0;
            std::cout << W0(po) << " "
                      << W0(brt) << " " << W0(bvar) << " " << W0(gbo) << " "
                      << W0(lar) << " \t";
            res.rs(std::cout); // XXX
            std::cout << std::endl;
            if (with_file_output)
              Environment::out_line(*out, res.b.list_sol, "\n");
          }
  }
  if (out) delete out;
}
