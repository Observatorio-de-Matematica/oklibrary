// Oliver Kullmann, 11.2.2021 (Swansea)
/* Copyright 2021 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Programming/InputOutput/Matching.cpp
  \brief Application for checking whether one file matches a pattern given by regular expressions

  USAGE:

  > Matching Patternfile Comparisonfile option

  with option one of
   - lm : line-matching
   - fm : full-matching (or file-matching).

  In case of lm, both files need to have the same number of lines (and there
  needs to be at least one line, and the final line needs to be finished with
  eol), while in case of fm there are no restrictions except of a non-empty
  Comparisonfile needs to be finished with eol.

  The lines of Patternfile resp. the whole Patternfile are interpreted as a
  regular expression in ECMAScript-style, as described in
  https://en.cppreference.com/w/cpp/regex/ecmascript .

  The return-value is 0 iff no error occurred, and the regular expression(s)
  (completely) matched.

*/

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <utility>

#include <cstdlib>
#include <cassert>

#include <ProgramOptions/Environment.hpp>

namespace Matching {
  enum class MatO {lines=0, full=1};
}
namespace Environment {
  template <>
  struct RegistrationPolicies<Matching::MatO> {
    static constexpr int size = int(Matching::MatO::full)+1;
    static constexpr std::array<const char*, size> string
    {"lm", "fm"};
  };
}
namespace Matching {
  std::ostream& operator <<(std::ostream& out, const MatO m) {
    switch (m) {
    case MatO::lines : return out << "matching-lines";
    default : return out << "matching-file";}
  }

}

namespace {

  const Environment::ProgramInfo proginfo{
        "0.3.0",
        "12.2.2021",
        __FILE__,
        "Oliver Kullmann",
        "https://github.com/OKullmann/oklibrary/blob/master/Programming/InputOutput/Matching.cpp",
        "GPL v3"};

  using namespace Matching;

  const std::string error = "ERROR[" + proginfo.prg + "]: ";

  bool show_usage(const int argc, const char* const argv[]) {
    if (not Environment::help_header(std::cout, argc, argv, proginfo))
      return false;
    std::cout <<
    "> " << proginfo.prg << " PatternFile InputFile [options]\n\n"
    " options : " << Environment::WRP<MatO>{} << "\n\n"
    " compares the patterns to the input, with output only in case of\n"
    " error or non-matching.\n\n"
;
    return true;
  }

  enum class Error {
    pnumber = 1,
    file_open = 2,
    file_read = 3,
    eof = 4,
    empty_pattern_file = 5,
    regular_expression = 6,
    number_lines = 7,
    mismatch = 8,
    option = 9,
  };

  Environment::tokens_t split(const std::string& name) {
    std::ifstream in(name);
    if (not in) {
      std::cerr << error << "File \"" << name << "\" not readable.\n";
      std::exit(int(Error::file_open));
    }
    char c;
    auto res = Environment::split(in, '\n', c);
    if (in.bad()) {
      std::cerr << error << "Reading error with file \"" << name << "\".\n";
      std::exit(int(Error::file_read));
    }
    if (c != '\n') {
      std::cerr << error << "File \"" << name << "\" does not finish with"
        "\n end-of-line symbol, but with character-code " << int(c) << ".\n";
      std::exit(int(Error::eof));
    }
    return res;
  }

  typedef std::size_t size_t;

  std::vector<std::regex> extract(const Environment::tokens_t& T, const std::string& Pfile) {
    std::vector<std::regex> regv;
    for (size_t i = 0; i < T.size(); ++i) {
      try { regv.emplace_back(T[i]); }
      catch (const std::regex_error& e) {
        std::cerr << error << "Regular expression error in file \"" <<
          Pfile << "\", line " << i+1 << ":\n"
          "expression: \"" << T[i] << "\"\n"
          "what: " << e.what() << "\n"
          "code: " << e.code() << "\n";
        std::exit(int(Error::regular_expression));
      }
    }
    return regv;
  }

  std::string transfer(const std::string& file, const bool final_eol = true) {
    std::ifstream in(file);
    if (not in) {
      std::cerr << error << "File \"" << file << "\" not readable.\n";
      std::exit(int(Error::file_open));
    }
    std::stringstream s;
    s << in.rdbuf();
    if (in.bad() or s.bad()) {
      std::cerr << error << "Reading error with file \"" << file << "\".\n";
      std::exit(int(Error::file_read));
    }
    if (not final_eol) return s.str();
    if (s.str().empty()) return "";
    if (s.str().back() != '\n') {
      std::cerr << error << "File \"" << file << "\" does not finish with"
        "\n end-of-line symbol, but with character-code " << int(s.str().back()) << ".\n";
      std::exit(int(Error::eof));
    }
    return s.str();
  }

  std::pair<std::regex, std::string> extract(const std::string& Pfile) {
    const std::string Ps = transfer(Pfile, false);
    std::regex res;
    try { res.assign(Ps); }
    catch (const std::regex_error& e) {
      std::cerr << error << "Regular expression error in file \"" <<
        Pfile << "\":\n"
        "expression: \"" << Ps << "\"\n"
        "what: " << e.what() << "\n"
        "code: " << e.code() << "\n";
      std::exit(int(Error::regular_expression));
    }
    return {res, Ps};
  }

}

int main(const int argc, const char* const argv[]) {
  if (Environment::version_output(std::cout, proginfo, argc, argv)) return 0;
  if (show_usage(argc, argv)) return 0;

  if (argc < 3 or argc > 4) {
    std::cerr << error << "Two or three input parameters are required:\n"
      " - the file-name for the pattern,\n"
      " - the file-name of the file to check,\n"
      " - optionally the matching-opting.\n";
    return int(Error::pnumber);
  }

  const std::string Pfile = argv[1];
  const std::string Cfile = argv[2];
  const auto options = argc == 3 ? MatO::lines :
    Environment::read<MatO>(argv[3]);
  if (not options) {
    std::cerr << error << "Invalid option-parameter: \"" << argv[3] << "\".\n";
    return int(Error::option);
  }
  const MatO mo = options.value();

  if (mo == MatO::lines) {

    const auto P_lines = split(Pfile);
    const size_t N = P_lines.size();
    const std::vector<std::regex> regv = extract(P_lines, Pfile);
    assert(regv.size() == N);

    const auto C_lines = split(Cfile);
    if (C_lines.size() != N) {
      std::cerr << error << "File \"" << Cfile << "\" has " << C_lines.size() <<
        " lines, but the pattern-file has " << N << " lines.\n";
      return int(Error::number_lines);
    }
    for (size_t i = 0; i < N; ++i) {
      if (not std::regex_match(C_lines[i], regv[i])) {
        std::cerr << error << "Mismatch in line " << i+1 << ":\n"
          "Pattern: \"" << P_lines[i] << "\"\n"
          "Given  : \"" << C_lines[i] << "\"\n";
        return int(Error::mismatch);
      }
    }
  }
  else {

    const auto [regex, pattern] = extract(Pfile);
    const auto C = transfer(Cfile);
    if (not std::regex_match(C, regex)) {
      std::cerr << error << "Mismatch:\n"
        "Pattern: \"" << pattern << "\"\n"
        "Given  : \"" << C << "\"\n";
      return int(Error::mismatch);
    }
  }
}
