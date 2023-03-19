// Oliver Kullmann, 6.3.2022 (Swansea)
/* Copyright 2022, 2023 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  General definitions regarding the SAT-translation of the bcc-problem

   - Types imported from Bicliques2SAT:
    - var_t

   - Constants:
    - default_B
    - default_sec

   - Functions:
    - read_var_t(string, var_t) -> var_t
    - read_uint_t(string, uint_t) -> uint_t

   - Scoped enum Error

*/

#ifndef BCC2SAT_LsdBtFBZpI
#define BCC2SAT_LsdBtFBZpI

#include <utility>
#include <optional>

#include <Numerics/NumTypes.hpp>
#include <Numerics/NumInOut.hpp>
#include <ProgramOptions/Strings.hpp>

#include "Bicliques2SAT.hpp"

namespace BCC2SAT {

  typedef Bicliques2SAT::var_t var_t;
  constexpr var_t default_B = 0;
  constexpr FloatingPoint::int_t default_sec = FloatingPoint::P231m1;


  var_t read_var_t(const std::string& s, const var_t def) {
    if (s.empty()) return def;
    return FloatingPoint::toUInt(s);
  }

  FloatingPoint::uint_t read_uint_t(const std::string& s, const FloatingPoint::uint_t def) {
    if (s.empty()) return def;
    return FloatingPoint::touint(s);
  }


  std::pair<FloatingPoint::uint_t, bool>
  read_uint_with_plus(const std::string& s) {
    if (s.starts_with('+'))
      return {FloatingPoint::touint(s.substr(1)), true};
    else return {FloatingPoint::touint(s), false};
  }
  std::optional<Bicliques2SAT::Bounds> read_bounds(const std::string& s) {
    // assuming for now DI::downwards XXX
    using namespace Bicliques2SAT;
    if (s.empty()) return Bounds{DI::downwards, true, 0, 0, 0};
    const auto split = Environment::split(s, ',');
    const auto size = split.size();
    assert(size != 0);
    if (size >= 3) return {};
    else if (size == 1) {
      const auto [B, with_plus] = read_uint_with_plus(s);
      if (with_plus) return Bounds{DI::downwards, true, B, 0, 0};
      else return Bounds{DI::downwards, false, 0, 0, B};
    }
    else {
      const auto l = read_uint_t(split[0], 0);
      const auto [u,with_plus] = read_uint_with_plus(split[1]);
      if (with_plus) return Bounds{DI::downwards, true, u, l, 0};
      else return Bounds{DI::downwards, false, 0, l, u};
    }
  }

  enum class Error {
    missing_parameters = 1,
    faulty_parameters = 2,
    bad_sb = 3,
    bad_log = 4,
    found_unsat = 20,
  };

}

#endif
