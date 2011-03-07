#ifndef NGRAMS_COUNTER_UTF32_HXX
#define NGRAMS_COUNTER_UTF32_HXX
// (c) Bernard Hugueney, GPL v3+

#include <array>
#include <functional>
#include <algorithm>

#include "ngrams_counter_base.hxx"

#include "utf_codecs.hxx"

namespace {
  template<std::size_t shift, typename T> 
  inline typename boost::enable_if<boost::is_integral<T>, T>::type rotate_left(T v)
  { return (v << shift) | (v >> (sizeof(T)*8 - shift)); }  
}

namespace std {
  template<std::size_t N> struct hash<std::array<char32_t, N> > {
    std::size_t operator()(std::array<char32_t, N> const& a) const {
      return std::accumulate(a.begin(), a.end(), static_cast<std::size_t>(0)
                             , [](std::size_t i, char32_t c){ return rotate_left<21>(i) ^ static_cast<std::size_t>(c);})
        /*^ N */; // the usual final ^N is not needed only because all hashed arrays are of the same size.
      }
  };
}

namespace {
#ifdef CUSTOM_ARRAY_ROTATE
  template<std::size_t I, std::size_t N> struct rotate_helper {
    void operator()(std::array<char32_t, N>& a) const{
      a[I]=a[I+1];
      rotate_helper<I+1, N>()(a);
    }
  };
  template<std::size_t N> struct rotate_helper<N, N> {
    void operator()(std::array<char32_t, N>& a) const {}
  };
  template<std::size_t N> void rotate(std::array<char32_t, N>& a){
    char32_t const tmp(a[0]);
    rotate_helper<0,N>()(a);
    a[N-1]= tmp;
  }

#else
 template<std::size_t N> void rotate(std::array<char32_t, N>& a)
 { std::rotate(a.begin(), a.begin()+1, a.end()); }
#endif

}

template<std::size_t N>
struct ngrams_counter<std::array<char32_t, N> > : ngrams_counter_base< std::array<char32_t, N> > {
  static std::size_t constexpr size= N;
  template<typename In> std::tuple<In, bool> process(In b, In e) {
    bool processed(false);
    char32_t const new_char(to_normal(from_utf8(b, e)));
    if((new_char != U'_') || this->current_ngram[N-1] != U'_'){
      processed= true;
      this->current_ngram[0]= new_char;
      rotate(this->current_ngram);
    }
    return std::make_tuple(b, processed);
  }
};

#endif
