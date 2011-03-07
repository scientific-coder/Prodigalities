// (c) Bernard Hugueney, GPL v3+
// had to patch local boost repo for constexpr correctness cf. [post V.Botet] :(
//g++-snapshot -std=c++0x bench_generic_ngrams.cxx -DBASIC_UTF8_CMP  -o bench_generic_ngrams  -I/home/bernard/Code/boost-trunk/ $(pkg-config --cflags --libs glibmm-2.4)  -L/usr/lib/debug/usr/lib/ -L/usr/lib/debug/lib/ -lstdc++ -Wall -O4 -march=native -licuuc -g3

// define BASIC_UTF8_CMP to replace default unicode aware Glib::ustring equality test with a byte for byte one in utf8 implementation
// define CUSTOM_ARRAY_ROTATE to replace call to std::rotate with a hand crafted custom unrolled implementation in utf32 implementation

#include <array>
#include <functional>
#include <algorithm>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <boost/progress.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "ngrams_counter_utf8.hxx"
#include "ngrams_counter_utf32.hxx"
#include "ngrams_counter_bitfields.hxx"

// 
struct bench {
  typedef char const* const_iterator;// iterator over file content
  // name of file ot process and nb of most frequent ngrams to display
  explicit bench(char const* filename, std::size_t n)// using Boost.Interprocess memory mapped files
  : mapping(filename, boost::interprocess::read_only), mapped_rgn(mapping, boost::interprocess::read_only)
  , beg(reinterpret_cast<const_iterator const>(mapped_rgn.get_address())), end(beg + mapped_rgn.get_size())
  , nb(n)
  {}
  
  // count ngrams in mmaped file and diplays the most frequent ones with their frequency
  // using a ngram counter of the given type. (used for benchmarking various ngram counter implementations)
  template<typename NgramCounter> void operator()(NgramCounter counter) const {
    std::cout<< "Counting "<< NgramCounter::size <<"-grams in ";
    {// begin local scope for progress_timer
      boost::progress_timer t;  // start timing
      // tmp is used to check that progress was done (i.e. no invalid /incomplete utf-) :
      // on error current == tmp as counter consumed no data.
      for (const_iterator current(beg), tmp(end)
             ; (current != end) && (current != tmp)
             ; tmp= current, current= counter(tmp, end)) 
        { }
    }
    std::cout<<" have been counting "<< counter.total() <<" ngrams\n";
    // value_type is implementation specific, we (partial) sort on copies stored in a vector
    typedef typename NgramCounter::value_type value_type;
    std::vector<value_type> res;
    counter.output(std::back_inserter(res));
    auto head (res.begin() + nb);
    std::partial_sort(res.begin(), head, res.end()
                      , [](value_type const& v1, value_type const& v2) { return v1.second > v2.second; });
    std::for_each(res.begin(), head, [](value_type const& v){ std::cout << v.first << " : "<< v.second << std::endl;});
    std::cout<<std::endl;
  }

  boost::interprocess::file_mapping mapping;
  boost::interprocess::mapped_region mapped_rgn;
  const_iterator const beg,  end;
  std::size_t const nb;
};

int main(int argc, char* argv[]) {
  std::size_t constexpr N= 3; // N as in 'N-gram'
  // for each of the 3 ngrams counter implementations, run the bench
  boost::mpl::for_each< boost::mpl::vector<
    ngrams_counter<glib_ustring_ngram<N> >
    , ngrams_counter<std::array<char32_t, N> >
    , ngrams_counter<ngram<N> >
    > >(std::cref(bench(argv[1], 10)));
  return 0;
}
