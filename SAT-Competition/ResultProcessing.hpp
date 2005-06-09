// Oliver Kullmann, 18.5.2005 (Swansea)

#ifndef RESULTPROCESSING_ppOgB535

#define RESULTPROCESSING_ppOgB535

#include <vector>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <cassert>

#include "FunctionHandling.hpp"

#include "BasicSetOperations.hpp"

#include "SingleResult.hpp"
#include "ParsingSingleResult.hpp"
#include "ParsingResultSequences.hpp"

namespace OKlib {

  namespace SATCompetition {

    class ResultNode;

    typedef std::set<const ResultNode*> SetResultNodesP;

    typedef std::map<SuperSeries, SetResultNodesP*> MapSuperSeries;
    typedef std::map<Series, SetResultNodesP*> MapSeries;
    typedef std::map<Benchmark, SetResultNodesP*> MapBenchmark;
    typedef std::map<Solver, SetResultNodesP*> MapSolver;
    typedef std::map<SATStatus, SetResultNodesP*> MapSATStatus;
    typedef std::map<TimeOut, SetResultNodesP*> MapTimeOut;

    struct ResultNode {
      const ResultBasis* rb;

      MapSuperSeries::const_iterator m_sup_ser_it;
      MapSeries::const_iterator m_ser_it;
      MapBenchmark::const_iterator m_bench_it;
      MapSolver::const_iterator m_solv_it;
      MapSATStatus::const_iterator m_sat_stat_it;
      MapTimeOut::const_iterator m_tmo_it;

      explicit ResultNode(const ResultBasis* rb) : rb(rb) {}
    };

    inline std::ostream& operator <<(std::ostream& out, const ResultNode& r) {
      return out << *(r.rb);
    }

    typedef std::vector<const ResultNode*> VectorResultNodesP;
    
    inline std::ostream& operator <<(std::ostream& out, const VectorResultNodesP& vec) {
      typedef VectorResultNodesP::const_iterator const_iterator;
      const const_iterator end(vec.end());
      for (const_iterator i = vec.begin(); i != end; ++i)
        out << **i << "\n";
      return out;
    }



    // ---------------------------------------------------------------------------------------------------------------------

    template <typename ResultIterator>
    // ToDo: Conceptualisation
    // ResultIterator::value_type is Result
    // It is assumed that the lifetime of these results is as long as the lifetime of the ResultDatabase object.
    class ResultDatabase {
      MapSuperSeries map_sup_ser;
      MapSeries map_ser;
      MapBenchmark map_bench;
      MapSolver map_solv;
      MapSATStatus map_sat_stat;
      MapTimeOut map_tmo;

      typedef std::vector<ResultNode> VectorResultNodes;
      VectorResultNodes result_collection;
      typedef VectorResultNodes::size_type number_results_type;
      number_results_type number_results_;
      typedef VectorResultNodes::iterator result_collection_iterator;
      typedef VectorResultNodes::const_iterator result_collection_const_iterator;

    public :

      ResultDatabase(ResultIterator begin, const ResultIterator end) {
        for (; begin != end; ++begin)
          result_collection.push_back(ResultNode(&*begin));
        number_results_ = result_collection.size();
        query_result.resize(number_results_);

        const MapSuperSeries::iterator end_map_sup_ser(map_sup_ser.end());
        const MapSeries::iterator end_map_ser(map_ser.end());
        const MapBenchmark::iterator end_map_bench(map_bench.end());
        const MapSolver::iterator end_map_solv(map_solv.end());
        const MapSATStatus::iterator end_map_sat_stat(map_sat_stat.end());
        const MapTimeOut::iterator end_map_tmo(map_tmo.end());

        for (result_collection_iterator i = result_collection.begin(); i != result_collection.end(); ++i) {
          ResultNode& rn(*i);
          const ResultBasis* r(rn.rb);

          update_map<SuperSeries>(rn, r -> super_series(), map_sup_ser, rn.m_sup_ser_it, end_map_sup_ser);
          update_map<Series>(rn, r -> series(), map_ser, rn.m_ser_it, end_map_ser);
          update_map<Benchmark>(rn, r -> benchmark(), map_bench, rn.m_bench_it, end_map_bench);
          update_map<Solver>(rn, r -> solver(), map_solv, rn.m_solv_it, end_map_solv);
          update_map<SATStatus>(rn, r -> sat_status(), map_sat_stat, rn.m_sat_stat_it, end_map_sat_stat);
           update_map<TimeOut>(rn, r -> time_out(), map_tmo, rn.m_tmo_it, end_map_tmo);
        }
      }

      ~ResultDatabase() {
        delete_values(map_sup_ser);
        delete_values(map_ser);
        delete_values(map_bench);
        delete_values(map_solv);
        delete_values(map_sat_stat);
        delete_values(map_tmo);
      }

      number_results_type number_results() const { return number_results_; }

      const MapSuperSeries& super_series() const { return map_sup_ser; }
      const MapSeries& series() const { return map_ser; }
      const MapBenchmark& benchmark() const { return map_bench; }
      const MapSolver& solver() const { return map_solv; }
      const MapSATStatus& sat_status() const { return map_sat_stat; }
      const MapTimeOut& time_out() const { return map_tmo; }

      typedef std::vector<const SetResultNodesP*> VectorOfSetsP;
      VectorOfSetsP vector_of_sets;

    private :
      VectorResultNodesP query_result;
    public :

      const VectorResultNodesP& intersection() {
        query_result.clear();
        const VectorOfSetsP::size_type size_vector_of_sets = vector_of_sets.size();
        if (size_vector_of_sets == 0) {
          const result_collection_const_iterator end = result_collection.end();
          for (result_collection_const_iterator i = result_collection.begin(); i != end; ++i)
            query_result.push_back(&*i);
        }
        else {
          typedef SetResultNodesP::iterator set_iterator;
          typedef std::pair<set_iterator, set_iterator> Range;
          typedef std::vector<Range> VectorRanges;
          VectorRanges vr;
          vr.reserve(size_vector_of_sets);
          typedef VectorOfSetsP::const_iterator vector_sets_iterator;
          const vector_sets_iterator end = vector_of_sets.end();
          for (vector_sets_iterator i = vector_of_sets.begin(); i != end; ++i) {
            const SetResultNodesP* const set_pointer = *i;
            vr.push_back(Range(set_pointer -> begin(), set_pointer -> end()));
          }
          assert(vr.size() == size_vector_of_sets);
          ::OKlib::SetAlgorithms::intersection_sets(vr.begin(), vr.end(), std::back_inserter(query_result));
        }
        return query_result;
      }

    private :

      template <class Map>
      void delete_values(Map& m) {
        typedef typename Map::iterator iterator;
        std::for_each(m.begin(), m.end(), FunctionHandling::DeleteObjectSecond());
      }

      template <class ResultElement, class Map>
      void update_map(ResultNode& rn, const ResultElement e, Map& m, typename Map::const_iterator& m_it, const typename Map::iterator& end) {
        const ResultBasis* r(rn.rb);
        typedef typename Map::iterator iterator;
        const iterator it(m.find(e));
            if (it != end) {
              m_it = it;
              it -> second -> insert(&rn);
            }
            else {
              SetResultNodesP* set = new SetResultNodesP;
              set -> insert(&rn);
              m_it = m.insert(std::make_pair(e, set)).first;
            }
      }
    };

    // ------------------------------------------------------------------------------------------------------------------------------

    template <template <typename Result, typename CharT, typename ParseIterator> class ParserResult, class Result_ = Result, template <typename Value> class Container = std::vector>
    struct Result_database_from_file {

      typedef Result_ result_type;
      typedef Copy_results_from_file_to_container<ParserResult, Container, result_type> copy_type;
      typedef typename copy_type::container_type container_type;
      typedef typename container_type::const_iterator result_iterator_type;
      typedef ResultDatabase<result_iterator_type> database_type;

      container_type result_sequence;
      copy_type copy;
      database_type db;

      Result_database_from_file(const boost::filesystem::path& filename) : copy(filename, result_sequence), db(result_sequence.begin(), result_sequence.end()) {}
    };
    
  }

}

#endif
