#ifndef NGRAMS_COUNTER_BASE_HXX
#define NGRAMS_COUNTER_BASE_HXX
// (c) Bernard Hugueney, GPL v3+
#include <utility>
#include <unordered_map>

extern "C" {
#include <unicode/uchar.h>
}
#include <glibmm/ustring.h>


namespace { 
  char32_t constexpr placeholder_char= U'_'; 
  char32_t to_normal(char32_t c) { return u_isalpha(c) ? u_toupper(c) : placeholder_char; } //u_isUAlphabetic
  gunichar to_normal(gunichar c) { return Glib::Unicode::isalpha(c) ? Glib::Unicode::toupper(c) : placeholder_char;}
}
template<typename NgramType> struct ngrams_counter; // will inherit ngrams_counter_base<NgramType>

// base class factoring common code between the different implementations with crtp (no runtime cost)
template<typename NgramType> struct ngrams_counter_base {
  typedef NgramType ngram_type;
  static std::size_t constexpr size= std::tuple_size<ngram_type>::value;
  typedef std::unordered_map<ngram_type, std::size_t> counts_type;
  typedef typename counts_type::value_type internal_value_type;
  typedef std::pair<ngram_type, double> value_type;
  ngrams_counter_base() : processed(0){}
  // should we return e or b when [b e) does not contain a valid utf-8 codepoint ?
  // returning b would cause endless loop if we were passing the whole data and just waiting for it to be processed, returning e would loose [e b) data if it was just incomplete. Unless we give the caller the responsability to check that progress was done (returning b is a stop condition)<- chosen solution
  template<typename In>
  In operator()(In b, In e) {
    try {
      bool recorded; // recorded is false if the utf-8 codepoint starting @b was valid utf-8 but recording it in the ngram
      // would have given tow placeholders in a row
      std::tie(b, recorded)= static_cast<ngrams_counter<NgramType>&>(*this).process(b, e);
      // we do not store a new ngram if we did not record the codepoint or if we don't have a complete ngram yet
      if(recorded &&  (++processed >=size)) { ++data[current_ngram]; } // short-circuit && ftw
    } catch (std::out_of_range const&e) { /* returning unchanged b is the silent fail condition*/}
    return b;
  }
    // we only add ngrams after processing N-1 utf8 codepoints
  std::size_t total() const { return processed -(size-1) ; }

  // should I expose boost::iterator::transform_iterator instead ?
  template<typename Out>  Out output(Out o) const { 
    return std::transform( data.begin(), data.end(), o
                           , [&](internal_value_type const& v){ return std::make_pair(v.first, static_cast<double>(v.second) / this->total()); });
  }

protected:
  std::size_t processed;
  ngram_type current_ngram;
  counts_type data;
};
#endif
