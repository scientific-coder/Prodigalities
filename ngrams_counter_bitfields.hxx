#ifndef NGRAMS_COUNTER_BITFIELDS_HXX
#define NGRAMS_COUNTER_BITFIELDS_HXX
// (c) Bernard Hugueney, GPL v3+

#include <bitset>
#include <functional>
#include <numeric>
#include <tuple>

#include <boost/utility/enable_if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/back.hpp>

#include <boost/mpl/find_if.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/times.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/bind.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/less_equal.hpp>

#include "ngrams_counter_base.hxx"
#include "utf_codecs.hxx"

namespace {
  typedef boost::mpl::vector<unsigned char, unsigned short, unsigned int, unsigned long, std::uintmax_t
                             
                             , __int128
                             
                             > integral_types;
  
  typedef boost::mpl::back<integral_types>::type largest_integral_type;
  
  std::size_t constexpr bits_per_codepoint {21};
  char32_t constexpr codepoint_mask {(1<<bits_per_codepoint)-1};
  std::size_t constexpr max_efficient_size {sizeof(largest_integral_type)*8 / bits_per_codepoint};
}

// ngram is a packed 'array' of 21 bits bitfields optimized for speed by using integral types when possible
// could be a packed_bitfields<21, N>
template<std::size_t N> struct ngram {
  // mpl compile-time function goes here
  // smallest integral type large enough for 21*N bits
  // if none, should use a struct { ngram<max_efficient_size> head; ngram<N-max_efficient_size> rest; }
  // or better? { ngram< N/2> most; ngram<(N+1)/2 > least; } what about some compile-time knapsack ? :)
  typedef char32_t value_type; // external codepoint type
  static std::size_t constexpr size= N;

  static bool constexpr is_composite = N > max_efficient_size; // current implementation does not handle 
  static_assert(!is_composite, "composite ngram not yet implemented");
  typedef typename boost::mpl::deref<boost::mpl::find_if<integral_types
                                                       , boost::mpl::bind< boost::mpl::less_equal<>
                                                                           , boost::mpl::size_t<N* bits_per_codepoint>
                                                                           , boost::mpl::bind< boost::mpl::times<>
                                                                                               , boost::mpl::int_<8>
                                                                                               ,  boost::mpl::bind<boost::mpl::sizeof_<>
                                                                                                                   ,boost::mpl::placeholders::_1> > > > >::type::type
  data_type;

  explicit ngram( char32_t c=0 ):data(c){}

  //  void pop_front() { data= (data << bits_per_codepoint) & ~(static_cast<data_type>(~codepoint_mask)<< (N-1)*bits_per_codepoint); } // TODO mask
  template<std::size_t K> char32_t get() const { return (data >> ((N-K-1)*bits_per_codepoint)) & codepoint_mask; }

template<std::size_t K> void set(char32_t c) 
  { data= (data & ~(static_cast<data_type>(codepoint_mask)<<((N-K-1)*bits_per_codepoint))) | (static_cast<data_type>(c)<<(N-K-1)*bits_per_codepoint); }
  
  void push_back(char32_t c) { data= ((data << bits_per_codepoint) & ~(static_cast<data_type>(~codepoint_mask)<< (N-1)*bits_per_codepoint)) | c; }// todo mask

bool operator==(ngram<N> const& other) const { return data == other.data; }
bool operator<(ngram<N> const& other) const { return data < other.data; }

data_type data;
};

namespace {
  template<std::size_t N, bool IsSmall=true> 
  struct  hash_helper 
    : std::unary_function< ngram<N> , std::size_t> {
    std::size_t operator()(ngram<N> const& ng) const { return ng.data; }
  };
  
  template<std::size_t N> 
  struct  hash_helper<N, false> 
    : std::unary_function< ngram<N> , std::size_t> {
    std::size_t operator()(ngram<N> const& ng) const {
      // todo static assert sizeof(
      static_assert(sizeof(typename ngram<N>::data_type)%sizeof(std::size_t)==0, "data_type in ngram<N> should have a size that is a multiple of sizeof(std::size_t)"); 
      std::size_t const* const tmp(reinterpret_cast<std::size_t const*const>(&ng.data));
      // should I unroll it by hand ?
      return std::accumulate(tmp, tmp+ sizeof(typename ngram<N>::data_type)/sizeof(std::size_t), static_cast<std::size_t>(0)
                             , [](std::size_t v1, std::size_t v2) { return v1^v2;});
    }
  };
}
namespace std {
  template<std::size_t N> 
  struct hash< ngram<N> > : hash_helper<N, sizeof(typename ngram<N>::data_type) <= sizeof(std::size_t)> 
  {};

  template<std::size_t N> 
  struct tuple_size< ngram<N> > { static std::size_t constexpr value= N; };

}

template<std::size_t N>
struct ngrams_counter<ngram<N> > : ngrams_counter_base< ngram<N> > {
  static std::size_t constexpr size= N ;
  template<typename In> std::tuple<In, bool> process(In b, In e){
    bool processed(false);
    char32_t const new_char(to_normal(from_utf8(b, e)));
    if((new_char != U'_') || (this->current_ngram.template get<N-1>() != U'_')){
      processed= true;
      this->current_ngram.push_back(new_char);
    }
    return std::make_tuple(b, processed);
  }
};


template<typename Out, std::size_t N> Out& operator<<(Out& out, ngram<N> const& ng) {
  std::bitset<N*bits_per_codepoint> bs(ng.data);
  out<<bs;
  return out;
}

#endif
