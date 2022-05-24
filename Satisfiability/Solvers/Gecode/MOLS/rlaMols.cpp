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

MOLS> time ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" enum dom enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo 	rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	enum 6247 11190.147 145531043 702831 1994566 23
real	68m13.473s
user	187m29.102s
sys	0m48.871s
MOLS> time ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" enum dom enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	enum 6247 10890.773 145535235 702831 1994566 22
   vals    props   elvals   prunes   mprune   probes    rounds        solc      leaf          t    qelvals   qprunes
431.263  8.42716  13.1531  89.3961  229.698  272.053    1.5094  0.00483613  0.544098  0.0232329  0.0311831  0.261767
    155        0        0        0        0        2         1           0         0          0          0         0
    676       74       89      871      647     2390         8          15         1   0.274853   0.173489       2.2
53.6119  7.33887  11.1931   98.527  163.833  215.269  0.731606    0.114594  0.498052  0.0212473  0.0263532  0.290642
real	69m0.530s
user	182m35.353s
sys	0m45.149s


MOLS> time ./rlaMols 7 data/SpecsCollection/Eulerinvsymmbalt "" enum dom enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	enum 6247 8546.903 145519085 702831 1994566 23
real	56m50.373s
user	142m27.460s
sys	0m28.481s
MOLS> time ./rlaMols 7 data/SpecsCollection/Eulerinvsymmbalt "" enum val enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindom asc relpr 	enum 6256 8684.915 200646489 3811464 9517856 25
real	52m10.313s
user	144m45.477s
sys	5m54.339s
With checking for all-different on the lists by lareduction:
MOLS> time ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" enum val enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindom asc relpr 	enum 6256 16261.191 200642669 3811464 9517856 25
real	103m52.608s
user	272m5.740s
sys	4m47.637s
Strange that the runtime has gone up so much (that bit of testing should
take no time?).

MOLS> time ./rlaMols 7 data/SpecsCollection/Eulerinvsymmbalt "" enum val enumbr mindom asc relnpr 6
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindom asc relnpr 	enum 6256 15458.920 200612436 3811464 9517856 25
real	50m59.412s
user	257m39.538s
sys	39m10.022s
MOLS> time ./rlaMols 7 data/SpecsCollection/Eulerinvsymmbalt "" enum val enumbr mindom asc relnpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindom asc relnpr 	enum 6256 10971.967 200646137 3811464 9517856 25
real	66m7.620s
user	182m52.600s
sys	7m13.229s


EXPERIMENT:
Adding a final call of status() to commit, so that the final three lines of
RlaBranching::commit then are

      [[maybe_unused]]const auto status = s.status();
      assert(status == GC::SS_BRANCH);

doesn't change anything:
MOLS> time ./rlaMols_debug 7 data/SpecsCollection/Eulerinvsymmbalt "" enum dom enumbr mindom asc relpr 3
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
dom enumbr mindom asc relpr 	enum 6247 10949.095 0 702831 1994566 22
   vals    props   elvals   prunes   mprune   probes    rounds        solc      leaf          t    qelvals   qprunes
431.263  8.42718  13.1531  89.3961  229.698  272.053    1.5094  0.00483613  0.544098  0.0242563  0.0311831  0.261767
    155        0        0        0        0        2         1           0         0          0          0         0
    676       74       89      871      647     2390         8          15         1   0.272879   0.173489       2.2
53.6119  7.33889  11.1931   98.527  163.833  215.269  0.731606    0.114594  0.498052   0.021974  0.0263531  0.290642
real	66m52.594s
user	183m29.551s
sys	0m50.246s

Thus the problem seems unrelated to the commit-action.
Since runtime seems unaffected, calling status twice seems harmless.


2. An easier example for the solutions-duplication:

MOLS> ./rlaMols 5 data/SpecsCollection/LSred "" enum val enumbr mindegdom asc relpr 1
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindegdom asc relpr 	enum 62 0.001 122 19 46 4
  vals    props   elvals   prunes   mprune   probes    rounds     solc      leaf            t    qelvals   qprunes
    43  2.40741  3.59259  7.59259  13.8148  16.2593   1.22222   2.2963  0.703704  3.28889e-05  0.0833553  0.268646
    29        0        0        0        0        2         1        0         0        3e-06          0         0
    61        6        8       38       52       62         2        5         1     0.000118   0.170213   1.58333
9.2487  2.06173  2.69271  12.0392  17.2917  18.7772  0.423659  1.65981  0.465322   3.4729e-05  0.0541185  0.415753

All variable-heuristics yield an overcount, the minimum is 57 (while 56) is correct:
MOLS> ./rlaMols 5 data/SpecsCollection/LSred "" enum val enumbr mindom asc relpr 1
ERROR[Solvers::rlasolver]: there are equal elements in the solution-list
pl bt bh bo lar 		rt sat t prop flvs nds h
val enumbr mindom asc relpr 	enum 57 0.000 47 9 25 4
   vals    props   elvals  prunes   mprune   probes    rounds     solc      leaf  t    qelvals   qprunes
47.8125    2.625   3.8125    7.75   27.375  25.9375    1.3125   3.5625    0.5625  0  0.0855403  0.292849
     40        0        0       0       10        7         1        0         0  0          0         0
     61        7       10      22       52       52         2       10         1  0        0.2  0.615385
6.14512  2.44609  3.50654  6.6883  14.8363  14.6855  0.478714  3.36588  0.512348  0  0.0759935  0.196481


*/

/* TODOS:

Compare also with todos in gcMols.

0. Additional to the gecode-statistics, our statistics on the reduction
   need to be provided:
   - It seems best, given that there are quite a lot of numbers, to have every
     block of statistics (one "run") always having the R-header also for the
     general statistics.
   - And this perhaps with a fixed standardised width per column (so that
     a kind of visual anchor is created).
   - To this general statistics the number of la-reduction-calls needs to be
     added; call it "larc".
   - General statistics:
     - "pl": better "ppl" ("propagation-level"), w=3
     - "bt": perhaps "binbr" -> "bin", "enumbr" -> "enum", and then w=4
     - "bh": petter "bv" ("branching variable"), w=9
     - "bo": w=4
     - "lar": w=6
     - perhaps say 6 spaces separation to the next group
     - "rt": w=8
     - "sat": perhaps "satc", w = 9
     - "t": w=10 (rounded to 3 decimal places)
     - "prop": perhaps "ppc" ("propagation-calls"), w=10
     - "flvs": w=9
     - "nds": better "gnds" ("gecode-nodes), w=9
     - "h": better "gd" ("gecode-depth"), w=5
     - perhaps say 3 spaces separation to next group
     - "larc": w=9.

1. A customised brancher with Gecode's default branching strategies is needed:
   - DONE Handled by LookaheadBranching::RlaBranching.
   - Check via unit tests that the solver and gcMols give equal results. OZ

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
        "0.5.1",
        "24.5.2022",
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

  const size_t sep_spaces = 6;
  void rh(std::ostream& out) {
    Environment::header_policies<RT, PropO, BRT, BHV, GBO, LAR>(out);
    out << std::string(sep_spaces, ' ');
    out << "satc t ppc flvs gnds gd larc"; // XXX
    out << std::endl;
  }

  void rs(std::ostream& out, const rlaSR& res) {
    out << std::string(sep_spaces, ' ');
    res.rs(out); // XXX
    out << "\n";
    res.S.out(out, {"vals", "props", "elvals", "prunes",
                  "mprune", "probes", "rounds", "solc", "leaf",
                  "t", "qelvals", "qprunes"});
    out.flush();
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
            rh(std::cout);
            Environment::data_policies(std::cout, std::make_tuple(rt, po, brt, bvar, gbo, lar));
            rs(std::cout, res);
            if (with_file_output)
              Environment::out_line(*out, res.b.list_sol, "\n");
          }
  }
  if (out) delete out;
}
