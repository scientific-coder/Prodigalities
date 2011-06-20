// (C) Bernard Hugueney, GPL v3 or later.
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <functional>
#include <boost/utility.hpp>

struct entry {
  // int because it may store -1 during shifts internal step
  typedef std::vector<int> data_t;
  template<typename Is> Is& read_from(Is& is) {
    std::size_t size(0);//eof will leave size unchanged
    is >> size;
    data.resize(1);
    data.back().resize(size);
    if( size ) {
      for(std::size_t i(0); i != size; ++i)
        { is >> data.back()[i]; }
      int const sum(std::accumulate(data.back().begin(), data.back().end(), 0));
      if( 0 == sum % size)
        { compute(sum/size);}
      else { data.back().clear();} // one empty vector for invalid data
    } else { data.clear();} // 0 vector for eof 
    return is;
  }
  template<typename Os>  Os& write_to(Os& os) const {
    if( !data.empty() ) {
      if(data.back().empty()) { os<< "-1\n"; } 
      else {
        os << data.size() << " :\n";
        for(std::size_t i(0); i != data.size(); ++i) 
          { write_step(i, data[i], os); }
        os << "\n";
      }
    }
    return os;
  }  
private:  
  template<typename Os>  void write_step(std::size_t i, data_t const&d, Os& os) const {
    os << i;
    for(data_t::const_iterator it(d.begin()); it!= d.end(); ++it)
      { os << ((it== d.begin()) ? " : (" : ", ") << *it;}
    os << ")\n";
  }
  void compute(int const mean) {
    do{ data.push_back(data.back()); }
    while(compute_step(mean));
    data.pop_back();// we only test after the fact so we must pop afterward
  }
  // shift one unit from *it to next or previous elts
  template<bool to_previous> bool do_shift(data_t::iterator it) {
    --(*it);
    ++(*(to_previous ? boost::prior(it): boost::next(it)));
    return true; 
  }
  bool compute_step( int mean) {
    bool shifted(false);// set to true if we performed a shift during this step
    data_t & work_b(data.back());
    int delta(0);
    for(data_t::iterator it(work_b.begin()); it != work_b.end(); ++it){
      shifted|= (delta <0)
        ? do_shift<true>(it)
        :( (*it > mean) ? do_shift<false>(it) : false);
      delta+= *it - mean;
    }
    return shifted;
  }
  std::vector<data_t> data;
};

template<typename Is> Is& operator>>(Is& is, entry& e) 
{return e.read_from(is);}

template<typename Os> Os& operator<<(Os& os, entry const& e) 
{return e.write_to(os);}

int main(int argc, char* argv[]){
  std::ifstream in("input.txt");
  std::ofstream out("output.txt");
  std::istream_iterator<entry> input_begin(in), input_end;
  std::ostream_iterator<entry> output(out);
  std::copy(input_begin, input_end, output);
  return 0;
}
