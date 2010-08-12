// Matthew Gwynne, 12.8.2010 (Swansea)
/* Copyright 2010 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Structures/Sets/SetAlgorithms/SubsumptionHypergraph.hpp
  \brief Tests for the components from SubsumptionHypergraph.hpp

*/

#ifndef TESTSUBSUMPTIONHYPERGRAPH_aaJNraVt3
#define TESTSUBSUMPTIONHYPERGRAPH_aaJNraVt3

#include <sstream>
#include <set>

#include <boost/assign/list_of.hpp>

#include <OKlib/Programming/Messages/Utilities/TrivialMessage.hpp>
#include <OKlib/Programming/Messages/messages/Levels.hpp>

#include <OKlib/TestSystem/TestBaseClass_DesignStudy.hpp>
#include <OKlib/TestSystem/TestExceptions_DesignStudy.hpp>
#include <OKlib/Structures/Sets/SetAlgorithms/SubsumptionHypergraph.hpp>

namespace OKlib {
  namespace SetAlgorithms {
    namespace tests {

# define OKLIB_FILE_ID new ::OKlib::Messages::Utilities::FileIdentification \
      (__FILE__, __DATE__, __TIME__, "$Date: 2010/08/12 12:08:08 $", "$Revision: 0.8 $")

      /*!
        \class Subsumption_hypergraph
        \brief Testing the subsumption hypergraph generator
      */

      OKLIB_TEST_CLASS(Tests_Subsumption_hypergraph) {
        OKLIB_TEST_CLASS_C(Tests_Subsumption_hypergraph) {}
      private :

        template
          <template <class ContainerSetsF,
              class ContainerSetsG,
              class OutputContainerSets,
              class UniquenessTag = SubsumptionsTags::hyperedges_may_not_be_unique,
              class OrderTag = SubsumptionsTags::hyperedges_may_not_be_sorted_by_size,
              class SizeTag = typename boost::mpl::if_<
                boost::mpl::and_<
                  typename OKlib::traits::has_size_function<typename ContainerSetsF::value_type>::type,
                  typename OKlib::traits::has_size_function<typename ContainerSetsG::value_type>::type 
                  >, 
                SubsumptionsTags::use_size_of_hyperedges, 
                SubsumptionsTags::do_not_use_size_of_hyperedges>::type>
          class Subsumption_hypergraph>
        OKLIB_TEST_CLASS(Test_Subsumption_hypergraph) {
          OKLIB_TEST_CLASS_C(Test_Subsumption_hypergraph) {}
        private :
          void test(OKlib::TestSystem::Basic) {

            {
              typedef int value_type;
              typedef std::set<std::set<value_type> > hypergraph_F_type;
              typedef std::set<std::set<value_type> > hypergraph_G_type;
              typedef std::set<hypergraph_F_type > hypergraph_output_type;
              typedef hypergraph_output_type::iterator output_iterator;
              typedef Subsumption_hypergraph<hypergraph_F_type, hypergraph_G_type, hypergraph_output_type > hypergraph_type;
              hypergraph_type sub_hyp;
              
              { // empty sequence
                hypergraph_F_type empty_F;
                hypergraph_output_type empty_output;
                empty_output = sub_hyp.subsumption_hypergraph(empty_F.begin(), empty_F.end(), empty_F.begin(), empty_F.end());
                OKLIB_TEST_EQUAL_RANGES(empty_output, hypergraph_output_type());
                empty_output = sub_hyp(empty_F.begin(), empty_F.end(), empty_F.begin(), empty_F.end());
                OKLIB_TEST_EQUAL_RANGES(empty_output, hypergraph_output_type());
              }
              { // example sequence
                hypergraph_G_type hg_F = boost::assign::list_of
                  (boost::assign::list_of(1)(2)(3))
                  (boost::assign::list_of(1)(2)(4))
                  (boost::assign::list_of(1)(3)(5));
                hypergraph_F_type hg_G = boost::assign::list_of(boost::assign::list_of(1)(2)(3)(4));
                hypergraph_output_type intended_output = boost::assign::list_of(boost::assign::list_of
                                                                                (boost::assign::list_of(1)(2)(3))
                                                                                (boost::assign::list_of(1)(2)(4)));
                hypergraph_output_type example_output;
                example_output = sub_hyp.subsumption_hypergraph(hg_F.begin(),hg_F.end(), hg_G.begin(), hg_G.end());
                OKLIB_TEST_EQUAL_W3(example_output, intended_output);
              }
            }

          }

        };
        
        void test(::OKlib::TestSystem::Basic) {
          typedef ::OKlib::TestSystem::RunTest::container_type container_type;
          ::OKlib::TestSystem::TestLevel& test_level(::OKlib::TestSystem::test_level(::OKlib::TestSystem::Basic()));
          container_type test_objects;

          using OKlib::Messages::Utilities::trivial_message;
          OKLIB_FULL_LOG(trivial_message("\nFIRST TEST\n"));
          {
            std::stringstream test_err, test_messages, test_log;
            OKlib::Messages::MessagesBase::set(test_log, OKlib::Messages::MessagesBase::level(log_stream()));
            OKlib::Messages::MessagesBase::set(test_log, OKlib::Messages::MessagesBase::language(log_stream()));
            OKlib::Messages::MessagesBase::set(test_err, OKlib::Messages::MessagesBase::level(log_stream()));
            OKlib::Messages::MessagesBase::set(test_err, OKlib::Messages::MessagesBase::language(log_stream()));
            OKlib::Messages::MessagesBase::set(test_messages, OKlib::Messages::MessagesBase::level(log_stream()));
            OKlib::Messages::MessagesBase::set(test_messages, OKlib::Messages::MessagesBase::language(log_stream()));
            test_objects.push_back(new Test_Subsumption_hypergraph<OKlib::SetAlgorithms::Subsumption_hypergraph>());
            ::OKlib::TestSystem::RunTest::run_tests(test_err, test_messages, test_log, test_level, test_objects);
            OKLIB_FULL_LOG(trivial_message("Output of the three streams after RunTest::run_tests:\n"));
            log(trivial_message("content of test_err:\n" + test_err.str()));
            log(trivial_message("content of test_messages:\n" + test_messages.str()));
            log(trivial_message("test_log level = "));
            log(OKlib::Messages::messages::LevelNames(OKlib::Messages::MessagesBase::level(test_log)));
            log(trivial_message("\ncontent of test_log:\n" + test_log.str()));
            if (not test_err.str().empty())
              OKLIB_THROW("Error thrown!");
            if (test_messages.str().empty())
              OKLIB_THROW("No messages!");
            if (test_log.str().empty())
              OKLIB_THROW("No log!");
          }
        }
      };

    }
  }
}

# undef OKLIB_FILE_ID
#endif
