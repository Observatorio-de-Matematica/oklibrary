// Oliver Kullmann, 26.6.2023 (Swansea)
/* Copyright 2023 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*

  General definitions for processing the corpus QBF2BCC

*/

#ifndef DIRSTATISTICS_gDRJRMWsir
#define DIRSTATISTICS_gDRJRMWsir

#include <filesystem>
#include <string>

#include <cassert>

#include <Numerics/NumTypes.hpp>

namespace DirStatistics {

  namespace FP = FloatingPoint;

  enum class Error {
    missing_parameters = 1,
  };

  const std::string leaf_ending = ".B";

  bool is_leaf_QBF2BCC(const std::filesystem::path& p) {
    assert(std::filesystem::is_directory(p));
    return(p.string().ends_with(leaf_ending));
  }

  // Applying F to all leaves:
  template <class FUNC>
  void for_each_leaf(const std::filesystem::path& p, FUNC& F) {
    assert(not is_leaf_QBF2BCC(p));
    const std::filesystem::directory_iterator end;
    std::filesystem::directory_iterator di(p);
    if (di == end) return;
    const auto first = *di++;
    if (is_leaf_QBF2BCC(first)) {
      F(first);
      for (; di != end; ++di) F(*di);
    }
    else {
      for_each_leaf(first, F);
      for (; di != end; ++di) for_each_leaf(*di, F);
    }
  }

  struct nlvs {
    typedef FP::UInt_t count_t;
    count_t n = 0;
    void operator ()(const std::filesystem::path&) noexcept { ++n; }
  };

}

#endif
