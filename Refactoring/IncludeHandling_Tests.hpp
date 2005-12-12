// Oliver Kullmann, 28.8.2005 (Swansea)

/*!
  \file IncludeHandling_Tests.hpp
  \brief Tests regarding include statement handling.
  \todo Each loop in each test should have an inline comment.
  \todo Every time a data member is initialised we should test it.
*/

#ifndef INCLUDEHANDLINGTESTS_77665r

#define INCLUDEHANDLINGTESTS_77665r

#include <string>
#include <ostream>
#include <utility>
#include <vector>
#include <sstream>
#include <cassert>

#include <boost/spirit.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/assign/list_of.hpp>

#include "TestBaseClass.hpp"
#include "TestExceptions.hpp"

#include "ParserBase_Tests.hpp"


namespace OKlib {

  namespace Refactoring {

    // ##############################################################


    /*!
      \class TestData
      \brief To provide data used by the testing of include handling.      
     
      This class has a data member test_vector which is a vector of
      tuples. Each tuple represents a different program.  The first 
      element of each tuple is a string representation of the prefix
      of the program, the second element is a vector of pairs (the 
      first element of each pair is a tuple representing an include 
      directive, the second element is the string representation of 
      the same include directive) and the third element is the string
      representation of the whole program.

      \todo New vector for negative tests.
      \todo Improve design so that only the necessary typedefs are 
      public and the data is made available through public member 
      functions.
      \todo Improve naming.
    */

    class TestData {

    public:

      typedef std::string string_type;
      typedef int size_type;

      typedef boost::tuple<size_type,string_type,size_type,Include_forms,string_type> include_directive_data_type;
      typedef std::pair<include_directive_data_type,string_type> id_w_context_type;
      typedef std::vector<id_w_context_type> id_w_context_vec_type;
      typedef boost::tuple<string_type,id_w_context_vec_type,string_type> pr_type;
      typedef std::vector<pr_type> test_vector_type;    

      typedef id_w_context_vec_type::const_iterator id_w_c_vec_const_iterator_type;

      
      typedef include_directive_data_type id_t;
      typedef pr_type el_t;

      static test_vector_type test_vector;
      typedef test_vector_type::const_iterator const_iterator;

      typedef std::string prefix_t;
      typedef std::vector<prefix_t> vec_prefix_t;
      static vec_prefix_t ref_prefix_vector;

      typedef std::pair<std::string,std::string> program_pair_type;
      typedef std::vector<program_pair_type> vec_program_t;
      static vec_program_t working_vector;

      typedef vec_program_t::const_iterator const_program_iterator;

      typedef prefix_t p_t;
      typedef program_pair_type pp_t;

      TestData() {

        using namespace boost::assign;

        ref_prefix_vector += 
          p_t("OKsystem/AutarkySearch/AnalyseTotalAssignment.hpp");

        working_vector +=
          pp_t("AnalyseTotalAssignment.hpp","OKsystem/AutarkySearch/AnalyseTotalAssignment.hpp");
                         
        test_vector +=

          el_t("",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",0,system_header,"#include<iostream>"), ""),
               "#include<iostream>"
              ),
          el_t("",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(1,"iostream",0,system_header,"# include<iostream>"), ""),
               "# include<iostream>"
               ),
          el_t("",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(1,"iostream",1,system_header,"# include <iostream>"), ""),
               "# include <iostream>"
               ),
          el_t("",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"header.hpp",0,source_code_header,"#include\"header.hpp\""), ""),
               "#include\"header.hpp\""
               ),
          el_t("",
               list_of<std::pair<id_t,string_type> > 
                (id_t(1,"header.hpp",0,source_code_header,"# include\"header.hpp\""), ""),
               "# include\"header.hpp\""
               ),
          el_t("",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(1,"header.hpp",1,source_code_header,"# include \"header.hpp\""), ""),
               "# include \"header.hpp\""
               ),
          el_t("\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",0,system_header,"#include<iostream>"), ""),
               "\n#include<iostream>"
               ),
          el_t("\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",0,system_header,"#include<iostream>"), "\n"),
               "\n#include<iostream>\n"
               ),
          el_t("\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",1,system_header,"#include <iostream>"), "\n")
               (id_t(0,"header.hpp",1,source_code_header,"#include \"header.hpp\""), "\n"),
               "\n#include <iostream>\n#include \"header.hpp\"\n"
               ),
          el_t("prefix\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",1,system_header,"#include <iostream>"), "\n")
                 (id_t(0,"header.hpp",1,source_code_header,"#include \"header.hpp\""), "\n"),
               "prefix\n#include <iostream>\n#include \"header.hpp\"\n"
               ),
          el_t("prefix\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",1,system_header,"#include <iostream>"), "\n")
                 (id_t(0,"string",1,system_header,"#include <string>"), "\n")
                 (id_t(0,"header.hpp",1,source_code_header,"#include \"header.hpp\""), "\n"),
               "prefix\n#include <iostream>\n#include <string>\n#include \"header.hpp\"\n"
               ),
          el_t("prefix\n",
               list_of<std::pair<id_t,string_type> > 
                 (id_t(0,"iostream",0,system_header,"#include<iostream>"), "context0\n")
                 (id_t(1,"string",0,system_header,"# include<string>"), "context1\n")
                 (id_t(1,"header.hpp",1,source_code_header,"# include \"header.hpp\""), "context2"),
               "prefix\n#include<iostream>context0\n# include<string>context1\n# include \"header.hpp\"context2"
              )
          ;
      }

      const id_w_context_vec_type& include_directive_context_vector(const_iterator iterator) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        return id_w_c_vec;
      }

      const size_type& number_spaces_after_hash(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const id_t& id(pair.first);
        const size_type& number_spaces(id.get<0>());
        return number_spaces;
      }

      const string_type& header(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const id_t& id(pair.first);
        const string_type& header_(id.get<1>());
        return header_;
      }

      const size_type& number_spaces_after_include(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const id_t& id(pair.first);
        const size_type& number_spaces(id.get<2>());
        return number_spaces;
      }

      Include_forms include_form(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const id_t& id(pair.first);
        Include_forms include_form_(id.get<3>());
        return include_form_;
      }
      
      const string_type& context_string(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const string_type& context_string_(pair.second);
        return context_string_;
      }
      
      const string_type& include_directive_string(const_iterator iterator, id_w_c_vec_const_iterator_type iterator2) {
        const el_t& el(*iterator);
        const id_w_context_vec_type& id_w_c_vec(el.get<1>());
        const std::pair<id_t,string_type>& pair(*iterator2);
        const id_t& id(pair.first);
        const string_type& id_string(id.get<4>());
        return id_string;
      }

      const string_type& program_string(const_iterator iterator) {
        const el_t& el(*iterator);
        const string_type& program_string_(el.get<2>());
        return program_string_;
      }

      const string_type& prefix_string(const_iterator iterator) {
        const el_t& el(*iterator);
        const string_type& prefix_string_(el.get<0>());
        return prefix_string_;
      }

    };
 
    TestData::test_vector_type TestData::test_vector;
    TestData::vec_prefix_t TestData::ref_prefix_vector;
    TestData::vec_program_t TestData::working_vector;

    // ##############################################################

    /*!

      \class Test_IncludeDirective
      \brief Testing classes representing one include directive.
      \todo Add an extended description.
      \todo Testing of equality and inequality operators.
    */

    template <template <class String> class Include_Directive>
    class Test_IncludeDirective : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_IncludeDirective test_type;
      Test_IncludeDirective() {
        insert(this);
      }
    private :
      void perform_test_trivial() {
        {         
          typedef Include_Directive<TestData::string_type> id_type;         
          typedef TestData::id_w_context_vec_type id_w_c_vec_type;
          typedef TestData::const_iterator iterator_t;

          TestData test_data;
          id_w_c_vec_type test_vector;

          iterator_t end(test_data.test_vector.end());
          for (iterator_t i=test_data.test_vector.begin(); i!=end; ++i) {

            typedef typename id_w_c_vec_type::const_iterator iterator;
            const iterator& end(test_vector.end());
            
            for (iterator j = test_vector.begin(); j != end; ++j) {

              const TestData::size_type& spaces_after_hash(test_data.number_spaces_after_hash(i,j));
              const TestData::string_type& header(test_data.header(i,j));              
              const TestData::size_type& spaces_after_include(test_data.number_spaces_after_include(i,j));
              Include_forms include_form(test_data.include_form(i,j));
              assert(include_form != undefined_include_form);
              const TestData::string_type& expected_output(test_data.include_directive_string(i,j));
              id_type id(header,spaces_after_hash,spaces_after_include,include_form);	  
              std::ostringstream output;
              output << id;
              std::string output_string = output.str();
              
              OKLIB_TEST_EQUAL(header,id.header_file());
              OKLIB_TEST_EQUAL(spaces_after_hash,id.number_spaces_after_hash());
              OKLIB_TEST_EQUAL(spaces_after_include,id.number_spaces_after_include());
              OKLIB_TEST_EQUAL(include_form,id.include_form());
              OKLIB_TEST_EQUAL(expected_output,output_string);
              
              switch(include_form) {
              case system_header :
                OKLIB_TEST_EQUAL(id.opening(),"<");
                OKLIB_TEST_EQUAL(id.closing(),">");
                break;
              case source_code_header :
                OKLIB_TEST_EQUAL(id.opening(),"\"");
                OKLIB_TEST_EQUAL(id.closing(),"\"");
                break;
              }
            }
          }
        }
      }
    };

    // ##############################################################

    /*!
      \class Test_ProgramRepresentationIncludes
      \brief Testing classes for representing the include-directives within a program.
      \todo Add an extended description.
      \todo Testing of equality and inequality operators.
    */

    template <template <class charT = char, class traits = std::char_traits<charT>, class Allocator = std::allocator<charT> > class Program_Representation_Includes>
    class Test_ProgramRepresentationIncludes : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_ProgramRepresentationIncludes test_type;
      Test_ProgramRepresentationIncludes() {
        insert(this);
      }
    private :
      void perform_test_trivial() {
 
        {
          typedef TestData::string_type string_type;
          typedef TestData::size_type size_type;
          typedef TestData::id_w_context_vec_type id_w_c_vec_type;
          typedef TestData::const_iterator iterator_t;
          typedef IncludeDirective<string_type> id_type;

          TestData test_data;

          iterator_t end(test_data.test_vector.end());
          
          for (iterator_t i=test_data.test_vector.begin(); i!=end; ++i) {
            
            const string_type& prefix_string(test_data.prefix_string(i));
            const string_type& program_string(test_data.program_string(i));
            
            typedef typename id_w_c_vec_type::const_iterator iterator;

            Program_Representation_Includes<> program_representation;
            program_representation(prefix_string);

            id_w_c_vec_type test_vector(test_data.include_directive_context_vector(i));
            const iterator& end(test_vector.end());
             
            for (iterator j = test_vector.begin(); j != end; ++j) {    
              
              const string_type& context_string(test_data.context_string(i,j));
              const size_type& spaces_after_hash(test_data.number_spaces_after_hash(i,j));
              const string_type& header(test_data.header(i,j));              
              const size_type& spaces_after_include(test_data.number_spaces_after_include(i,j));
              Include_forms include_form(test_data.include_form(i,j));
              assert(include_form != undefined_include_form);
              id_type include_directive(header,spaces_after_hash,spaces_after_include,include_form);	  
              std::pair<id_type,string_type> id_w_context(std::make_pair(include_directive,context_string));
              program_representation(id_w_context);

            }

            std::ostringstream output;
            output << program_representation;
            string_type output_string = output.str();
            OKLIB_TEST_EQUAL(output_string,program_string);
          }
        }    
      }
    };
    
    // ##############################################################

    /*!
      \class Test_IncludeParsingGrammar
      \brief Testing grammars for parsing source code units in order to extract include directives.
      \todo Reinstate testing of program strings which are not matched by the grammar.
      \todo Add an extended description.
    */
    
    template <class Include_Parsing_Grammar>
    class Test_IncludeParsingGrammar : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_IncludeParsingGrammar test_type;
      Test_IncludeParsingGrammar() {
        insert(this);
      }
    private :
      void perform_test_trivial() {
        {         
          typedef TestData::string_type string_type;
          typedef TestData::size_type size_type;

          typedef TestData::id_w_context_vec_type id_w_c_vec_type;
          typedef TestData::const_iterator iterator_t;
          typedef IncludeDirective<string_type> id_type;
          typedef typename Include_Parsing_Grammar::program_representation_type pr_type;
          
          TestData test_data;
          id_w_c_vec_type test_vector;
          iterator_t end(test_data.test_vector.end());

          for (iterator_t i=test_data.test_vector.begin();i!=end;++i) {

            const string_type& prefix_string(test_data.prefix_string(i));
            const string_type& program_string(test_data.program_string(i));
            
            typedef typename id_w_c_vec_type::const_iterator iterator;

            pr_type program_representation;
            program_representation(prefix_string);

            id_w_c_vec_type test_vector(test_data.include_directive_context_vector(i));
                    
            typedef typename id_w_c_vec_type::const_iterator iterator;
            const iterator& end(test_vector.end());
            
            pr_type pr_expected;
            pr_expected(prefix_string);
            
            for (iterator j = test_vector.begin(); j != end; ++j) {

              const string_type& context_string(test_data.context_string(i,j));
              const size_type& spaces_after_hash(test_data.number_spaces_after_hash(i,j));
              const string_type& header(test_data.header(i,j));              
              const size_type& spaces_after_include(test_data.number_spaces_after_include(i,j));
              Include_forms include_form(test_data.include_form(i,j));
              assert(include_form != undefined_include_form);
              id_type include_directive(header,spaces_after_hash,spaces_after_include,include_form);	  
              std::pair<id_type,string_type> id_w_context(std::make_pair(include_directive,context_string));
              pr_expected(id_w_context);
            }

            pr_type pr_result;
            Include_Parsing_Grammar g(pr_result);
            
            if (not parse(program_string.c_str(), g).full)
              OKLIB_THROW("String \"" + program_string + "\" was not accepted");
            
            OKLIB_TEST_EQUAL(pr_result, pr_expected);

          }
        }
      }
    };

    // ##############################################################

    /*!
      \class Test_Parsing
      \brief Testing parsing of include directives from an istream.
      \todo Remove obsolete second test.
      \todo Here, as in Test_IncludeParsingGrammar, part of the test involves
      filling a ProgramRepresentationIncludes object with the include directive
      /context data from the TestData vector. Could this be extracted to a 
      further test and the result made available to both tests?
    */

    template <template <class charT, class traits, class Allocator> class Program_Representation_Includes, class StreamExtractor>
    class Test_Parsing : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_Parsing test_type;
      Test_Parsing() {
        insert(this);
      }
    private :
      void perform_test_trivial() {
        
        {
          typedef TestData::string_type string_type;
          typedef TestData::size_type size_type;
          typedef TestData::id_w_context_vec_type id_w_c_vec_type;
          typedef TestData::const_iterator iterator_t;

          typedef IncludeDirective<string_type> id_type;

          typedef char char_type;
          typedef std::char_traits<char_type> traits_type;
          typedef std::allocator<char_type> allocator_type;
          typedef Program_Representation_Includes<char_type, traits_type, allocator_type> pr_type;
          typedef StreamExtractor stream_extractor_type;
          
          TestData test_data;
          iterator_t end(test_data.test_vector.end());

          for (iterator_t i=test_data.test_vector.begin();i!=end;++i) {

            typedef typename id_w_c_vec_type::const_iterator iterator;

            const string_type& prefix_string(test_data.prefix_string(i));
            const string_type& program_string(test_data.program_string(i));          

            pr_type program_representation;
            program_representation(prefix_string);

            id_w_c_vec_type test_vector(test_data.include_directive_context_vector(i));
                    
            const iterator& end(test_vector.end());

            pr_type pr_expected;
            pr_expected(prefix_string);
            
            for (iterator j = test_vector.begin(); j != end; ++j) {

              const string_type& context_string(test_data.context_string(i,j));
              const size_type& spaces_after_hash(test_data.number_spaces_after_hash(i,j));
              const string_type& header(test_data.header(i,j));              
              const size_type& spaces_after_include(test_data.number_spaces_after_include(i,j));
              Include_forms include_form(test_data.include_form(i,j));
              assert(include_form != undefined_include_form);
              id_type include_directive(header,spaces_after_hash,spaces_after_include,include_form);	  
              std::pair<id_type,string_type> id_w_context(std::make_pair(include_directive,context_string));
              pr_expected(id_w_context);

            }

            std::istringstream program_stream(program_string);
            pr_type program_rep;
            (stream_extractor_type(program_stream))(program_rep);
            OKLIB_TEST_EQUAL(program_rep.prefix,prefix_string);
            OKLIB_TEST_EQUAL_RANGES(program_rep.include_directives_with_context,pr_expected.include_directives_with_context);

          }
        }

        {
          typedef char char_type;
          typedef std::char_traits<char_type> traits_type;
          typedef std::allocator<char_type> allocator_type;

          typedef Program_Representation_Includes<char_type, traits_type, allocator_type> pr_type;
          typedef StreamExtractor stream_extractor_type;

          typedef typename pr_type::string_type string_type;
          typedef IncludeDirective<string_type> id_type;
          typedef std::pair<id_type, string_type> pair_type;
          typedef std::vector<pair_type> container_type;

          using namespace boost::assign;

          typedef std::string prefix_type;
          typedef std::string program_type;

          program_type program_0("This is prefix 0\n#include<iostream>context\n# include <string> more context");
          program_type program_1("This is prefix 1\n#include<boost/filesystem>context\n# include \"Refactoring.hpp\" more context");
          
          prefix_type prefix_0("This is prefix 0\n");
          prefix_type prefix_1("This is prefix 1\n");

          container_type id_w_context_0;

          id_w_context_0 += 
            std::make_pair(id_type("iostream",0,0,system_header),string_type("context\n")),
            std::make_pair(id_type("string",1,1,system_header),string_type(" more context"));

          container_type id_w_context_1;

          id_w_context_1 += 
            std::make_pair(id_type("boost/filesystem",0,0,system_header),string_type("context\n")),
            std::make_pair(id_type("Refactoring.hpp",1,1,source_code_header),string_type(" more context"));

          typedef boost::tuple<program_type, prefix_type, container_type> el_t;
          typedef std::vector<el_t> test_vector_type;
          test_vector_type test_vector;

          test_vector += 
            el_t(program_0,prefix_0,id_w_context_0),
            el_t(program_1,prefix_1,id_w_context_1);

          typedef typename test_vector_type::const_iterator iterator;
          const iterator& end(test_vector.end());
          for (iterator i = test_vector.begin(); i != end; ++i) {
             const el_t& el(*i);
             const std::string& program_string(el.template get<0>());
             const prefix_type& prefix_string(el.template get<1>());
             const container_type& id_w_context(el.template get<2>());
             std::istringstream program_stream(program_string);
             pr_type program_rep;
             (stream_extractor_type(program_stream))(program_rep);
             OKLIB_TEST_EQUAL(program_rep.prefix,prefix_string);
             OKLIB_TEST_EQUAL_RANGES(program_rep.include_directives_with_context,id_w_context);
          }
        }
      }
    };

    // ##############################################################

    /*!
      \class Test_ExtendIncludeDirectives
      \brief Test class for functor which handles the extending of include
      directives in a single file.
    */

    template <template <class UniquenessPolicy> class ExtendIncludeDirectives>
    class Test_ExtendIncludeDirectives : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_ExtendIncludeDirectives test_type;
      Test_ExtendIncludeDirectives() {
        insert(this);
      }
    private :
      void perform_test_trivial() {
        typedef OKlib::SearchDataStructures::AssociativePrefixContainer<std::string> APC_type;
        APC_type prefix_container(TestData::ref_prefix_vector);
        ExtendIncludeDirectives<APC_type> extend_include_directives(prefix_container); 
        typedef TestData::vec_program_t::const_iterator const_program_iterator;
        const const_program_iterator& end(TestData::working_vector.end());
        for (const_program_iterator begin(TestData::working_vector.begin()); begin!= end; ++begin) {
          std::istringstream program(begin -> first);
          std::string expected_extended_program(begin -> second);
          extend_include_directives(program);          
        }
      }
    };

    // ##############################################################

    /*!
      \class Test_ExtendIncludeDirectivesTwoDirectories
      \brief Test class for extending all include directives in files below some working directories with paths relative to a working directory.
    */

    template <template <class Path, class UniquenessPolicy> class ExtendIncludeDirectivesTwoDirectories>
    class Test_ExtendIncludeDirectivesTwoDirectories : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_ExtendIncludeDirectivesTwoDirectories test_type;
      Test_ExtendIncludeDirectivesTwoDirectories() {
        insert(this);
      }
    private :
      void perform_test_trivial() {

      }
    };

    // ##############################################################

    /*!
      \class Test_ExtendIncludeDirectivesTwoRanges
    */

    template <template <class Range1, class Range2, class UniquenessPolicy> class ExtendIncludeDirectivesTwoRanges>
    class Test_ExtendIncludeDirectivesTwoRanges : public ::OKlib::TestSystem::TestBase {
    public :
      typedef Test_ExtendIncludeDirectivesTwoRanges test_type;
      Test_ExtendIncludeDirectivesTwoRanges() {
        insert(this);
      }
    private :
      void perform_test_trivial() {

        typedef std::vector<std::string> range_t;
        range_t work_range;
        range_t ref_range;

        using namespace boost::assign;
        typedef std::string s_t;

        ref_range +=
          s_t("header001.hpp/dir001/"),
          s_t("header002.hpp/dir001/dir002/"),
          s_t("header003.hpp/dir001/dir002/dir003/");

        work_range +=
          s_t("#include \"header001.hpp\""),
          s_t("#include \"header001.hpp\"\n\"header002.hpp\"");

        ExtendIncludeDirectivesTwoRanges<range_t,range_t,ThrowIfNonUnique>(ref_range,work_range);

      }
    };

  }

}

#endif
