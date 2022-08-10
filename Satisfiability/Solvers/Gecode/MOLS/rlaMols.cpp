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

1. Counting all (reduced) 36 triples of MOLS for N=5
    - for the propagation-level "domain"
    - enumerative branching-type
    - max-deg/dom-var variable-selection,
    - the default branching-order,
    - default lookahead-reduction,
    - commit-distance 1,
    - 2 threads,
    - and no stopping:

MOLS> ./rlaMols 5 data/SpecsCollection/3MOLS/basis "" count dom enu maxdegdom "" "" 1 2 ""
# command-line: "./rlaMols" "5" "data/SpecsCollection/3MOLS/basis" "" "count" "dom" "enu" "maxdegdom" "" "" "1" "2" ""
# N: 5
# k=6 total_num_sq=6: "data/SpecsCollection/3MOLS/basis"
#   num_uc=9 num_eq=0 num_peq=3
# no_ps
# num_runs=1
# threads=2
# rt=count-solutions(count)
# no_stopping
#   propagation-level: domain-prop(dom)
#   branching-type: enumerative-branching(enu)
#   variable-heuristic: max-deg/dom-var(maxdegdom)
#   order-heuristic: ascending-order(asc)
#   la-reduction-type: relaxed-pruning(relpr)
#   commit-distance: 1
  N       rt  pl  bt        bv   bo    lar gcd     satc           t        ppc st      nds      lvs
  5    count dom enu maxdegdom  asc  relpr   1       36       0.091        691  0       11        8
    mu0  qfppc  pprunes  pmprune  pprobes  rounds   solc     tr  pelvals     dp
310.667  0.333   19.423  135.707  166.465   1.667  0.000  0.019    0.773  1.000
302.000  0.000    5.825  133.775  120.645   1.000  0.000  0.009    0.000  0.000
320.000  1.000   38.808  137.097  250.000   3.000  0.000  0.032    2.318  2.000
  9.018  0.577   17.236    1.726   72.457   1.155  0.000  0.012    1.338  1.000
    mu0  qfppc  pprunes  pmprune  pprobes  rounds   solc     tr  pelvals     dp
289.250  1.556   78.423   44.998   53.249   1.250  4.500  0.008    6.090  2.000
251.000  1.154    3.226   21.262    9.541   1.000  0.000  0.001    1.993  1.000
309.000  2.333  208.000   71.845  129.126   2.000  8.000  0.022   11.650  3.000
 25.104  0.366   98.537   22.420   55.055   0.463  3.338  0.009    3.441  0.926

*/

/* BUGS:

*/

/* TODOS:

Compare also with todos in gcMols.

-1. Catching SIGUSR1, output the current results:
     - The class rlaStats needs another atomic_bool, "report", initially
       false, which is set to true by the signal-handler "activate_report".
     - When the add-function finds "report" true, output of stats is done,
       and report=false.
     - Possibly output to std::cerr ? Or another log-member, to which in
       this case output is directed?

1. Better time-information
   - The reduction-time is sum of user-times for all running threads.
   - One needs to divide this number by the number of threads, for an
     approximation of the real user-time; idle threads mean that this
     time is too small, but at least it's a simple handling.
   - Also the total reduction-time ("tr") should be reported (the sum-
     member of the statistics), corrected in the same way.

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
#include "LookaheadReduction.hpp"

namespace {

  const Environment::ProgramInfo proginfo{
        "0.99.0",
        "10.8.2022",
        __FILE__,
        "Oliver Kullmann and Oleg Zaikin",
        "https://github.com/OKullmann/oklibrary/blob/master/Satisfiability/Solvers/Gecode/MOLS/rlaMols.cpp",
        "GPL v3"};

  const std::string error = "ERROR[" + proginfo.prg + "]: ";
  constexpr int commandline_args = 13;

  using namespace Conditions;
  using namespace Encoding;
  using namespace PartialSquares;
  using namespace Solvers;
  using namespace Options;
  using namespace CommandLine;
  using namespace LookaheadReduction;

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg <<
      " N file_cond file_ps run-type prop-level branch-type"
      " branch-var branch-order la-type gcd threads"
      " (stop-type,stop-value)* tree-logging\n\n"
      " - N            : \";\"-separated list of \"a[,b][,c]\"-sequences\n"
      " - file_cond    : filename/string for conditions-specification\n"
      " - file_ps      : filename/string for partial-squares-specification\n"
      " - run-type     : " << Environment::WRPO<RT>{} << "\n" <<
      " - prop-level   : " << Environment::WRPO<PropO>{} << "\n" <<
      " - branch-type  : " << Environment::WRPO<BRT>{} << "\n" <<
      " - branch-var   : " << Environment::WRPO<BHV>{} << "\n" <<
      " - branch-order : " << Environment::WRPO<GBO>{} << "\n" <<
      " - la-reduction : " << Environment::WRPO<LAR>{} << "\n" <<
      " - gcd          : Gecode commit-distance; list as for N\n"
      " - threads      : floating-point for number of threads\n"
      " - stop-type    : " << Environment::WRPO<LRST>{} << "\n" <<
      " - tree-logging : " << Environment::WRPO<TREE>{} << "\n\n" <<
      "Here\n"
      "  - file_ps can be the empty string (no partial instantiation)\n"
      "  - to use a string instead of a filename, a leading \"@\" is needed\n"
      "  - the six algorithmic options can be lists (all combinations)\n"
      "  - these lists can have a leading + (inclusion) or - (exclusion)\n"
      "  - stop-values are unsigned int; times in seconds\n"
      "  - pairs of stop-types/values are separated by \"|\"\n"
      "  - for sat-solving and enumeration, output goes to file \"" <<
      "SOLUTIONS_" << proginfo.prg << "_N_timestamp\".\n\n"
;
    return true;
  }

  void rh(std::ostream& out) {
    out.width(wN); out << "N" << " ";
    Environment::header_policies<RT, PropO, BRT, BHV, GBO, LAR>(out);
    out.width(wgcd); out << "gcd" << " ";
    out << std::string(sep_spaces, ' ');
    rh_genstats(out);
    out << "\n";
  }

  void rs(std::ostream& out, const rlaSR& res) {
    const auto state = FloatingPoint::fixed_width(out, precision);
    out << std::string(sep_spaces, ' ');
    rs_genstats(out, res);
    out << "\n";
    res.outS(out, true);
    out.flush();
    FloatingPoint::undo(out, state);
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

  const auto list_N = read_N(argc, argv);
  const auto [ac, name_ac] = read_ac(argc, argv);
  const auto [ps0, name_ps] = read_ps(argc, argv, list_N);
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
  const list_unsigned_t gcdv = read_comdist(argc, argv, 10);
  const size_t num_runs =
    list_N.size()*pov.size()*brtv.size()*bvarv.size()*gbov.size()
    *larv.size()*gcdv.size();

  const double threads = read_threads(argc, argv, 11);

  const auto stod = read_rlast(argc, argv, 12);

  const auto to = Environment::read<TREE>(argv[13]);
  if (not to) {
    std::cerr << error << "Wrong item \"" << argv[13] << "\" for "
      "tree-logging.\n";
    return 1;
  }
  const bool with_tree_logging = to.value() == TREE::on;


  const std::string outfile = output_filename(proginfo.prg, list_N);
  const bool with_file_output = Options::with_file_output(rt);
  if (with_file_output and num_runs != 1) {
    std::cerr << error << "For solution-output the number of runs must be 1,"
      " but is " << num_runs << ".\n";
    return 1;
  }
  std::ostream* const out = with_file_output ?
    new std::ofstream(outfile) : nullptr;
  const delete_on_exit delete_out(out);
  if (with_file_output and (not out or not *out)) {
    std::cerr << error << "Can not open file \"" << outfile << "\" for "
      "writing.\n";
    return 1;
  }
  const bool with_log = Options::with_log(rt);
  std::ostream* const log = with_log ? &std::cout : nullptr;
  const std::string treeloggingfile = with_tree_logging ?
    treelogging_filename(proginfo.prg, list_N) : std::string();
  std::ostream* const tree_log = with_tree_logging ?
    new std::ofstream(treeloggingfile) : nullptr;
  const delete_on_exit delete_tree_log(tree_log);


  commandline_output(std::cout, argc, argv);
  info_output(std::cout,
              list_N, ac, name_ac, ps0, name_ps, rt,
              num_runs, threads, outfile, with_file_output);
  st_output(std::cout, stod);
  treelogging_output(std::cout, to.value(), treeloggingfile);
  algo_output(std::cout, std::make_tuple(pov, brtv, bvarv, gbov, larv));
  cd_output(std::cout, gcdv);
  std::cout.flush();


  for (const size_t N : list_N)
    for (const PropO po : pov) {
      const EncCond enc(ac, ps0 ? ps0.value() : PSquares(N,psquares_t{}),
                        prop_level(po));
      for (const BRT brt : brtv)
        for (const BHV bvar : bvarv)
          for (const GBO gbo : gbov)
            for (const LAR lar : larv)
              for (unsigned gcd : gcdv) {
                const rlaSR res =
                  rlasolver(enc, rt, brt, bvar, gbo, lar,
                            gcd, threads, stod, log, tree_log);
                if (with_log and
                    rt != RT::enumerate_with_log and
                    rt != RT::unique_s_with_log)
                  std::cout << "\n";
                rh(std::cout);
                std::cout.width(wN); std::cout << N << " ";
                Environment::data_policies(std::cout,
                  std::make_tuple(rt, po, brt, bvar, gbo, lar));
                std::cout.width(wgcd); std::cout << gcd << " ";
                rs(std::cout, res);
                if (with_file_output)
                  Environment::out_line(*out, res.b.list_sol, "\n");
              }
    }
}
