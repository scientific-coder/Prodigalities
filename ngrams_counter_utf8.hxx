#ifndef NGRAMS_COUNTER_UTF8_HXX
#define NGRAMS_COUNTER_UTF8_HXX
// (c) Bernard Hugueney, GPL v3+

#include "ngrams_counter_base.hxx"

#include <glibmm/ustring.h>

#include "utf_codecs.hxx"

  // uses Glib::ustring to store the ngram, knows the ngram length and create a default ngram of this length
  // note: some incidental complexity of the Glib::ustring solution comes for the genericity required by the benchmark
  // a straightforward implemntation would be somewhat simpler.
  template<std::size_t N> struct glib_ustring_ngram : Glib::ustring{
    glib_ustring_ngram(): Glib::ustring(N, placeholder_char){};
  };

namespace std {

  template<std::size_t N> struct hash<glib_ustring_ngram<N> > : std::unary_function<glib_ustring_ngram<N>, std::size_t> , private std::hash<std::string> {
    std::size_t operator()(glib_ustring_ngram<N> const& s) const 
    { return  std::hash<std::string>::operator()(s.raw()) ; }
  };


  template<std::size_t N> struct tuple_size<glib_ustring_ngram<N> > { static std::size_t constexpr value= N; };


  template<std::size_t N> struct equal_to<glib_ustring_ngram<N> > : std::binary_function<glib_ustring_ngram<N>, glib_ustring_ngram<N>, bool> {
    std::size_t operator()(glib_ustring_ngram<N> const& s1, glib_ustring_ngram<N> const& s2) const 
#ifdef BASIC_UTF8_CMP
    { return  s1.raw() == s2.raw() ; }
#else
    { return  s1 == s2 ; }
#endif
  };
}


namespace {
  template<typename In>
  std::size_t utf8_nb_bytes(In b, In e){
    std::size_t res(0); // 0 = error seq too small
    if (b != e) {
      for(std::uint8_t header(*b),  mask(0x80); header & mask; ++res, header <<=1)
        { }
      // res=1 -> error : was not a header byte
      res= res ? res : 1; // res= 0 was ASCII -> 1 byte
      res = res > static_cast<std::size_t>(std::distance(b, e)) ? 0 : res; // seq to small: error
    }
    return res;
  }
}

template<std::size_t N>
struct ngrams_counter< glib_ustring_ngram<N> > : ngrams_counter_base< glib_ustring_ngram<N> > {
  static std::size_t constexpr size= N ;
  template<typename In> std::tuple<In, bool> process(In b, In e){
    bool processed (false);
    std::size_t const nb_bytes(utf8_nb_bytes(b, e));// how much bytes to read one utf-8 codepoint
    if (nb_bytes) { // we will be able to read a valid utf-8 codepoint in [b, e)
      gunichar const new_char(to_normal(g_utf8_get_char_validated(b, nb_bytes)));
      if((new_char != U'_' ) || (*this->current_ngram.rbegin() != '_')) { // '_' in UTF-8 is same as in ASCII
        this->current_ngram.append(1, new_char);
        this->current_ngram.erase(0, 1);
        processed= true;
      }
      std::advance(b, nb_bytes);
    } else { invalid_utf_sequence<8>(b, e); } // throws std::out_of_range
    //    std::cerr<< this->current_ngram.data << std::endl;
    return std::make_tuple(b, processed);
  }
};

#endif
