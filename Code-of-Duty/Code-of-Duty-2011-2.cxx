#include <fstream>
#include <iterator>
#include <numeric>
#include <vector>
#include <boost/utility.hpp> // prior and next

typedef std::vector<int> data_t;
typedef std::vector<data_t> solution_t;

struct process {
  // non-const ref because we swap the data out of it into the solution  
  solution_t operator()(data_t& d) const {
    solution_t res;
    if(!d.empty()) {
      int const sum(std::accumulate(d.begin(), d.end(), 0));
      if(sum) {
        if( sum % d.size() == 0 ) {
          int const mean(sum / d.size());
          res.push_back(data_t()); res.back().swap(d); // faster than res.push_back(d)
          while(compute_step(res, mean))
            {}
          res.pop_back();// we only test after the fact so we must pop afterward
        } else { res.push_back(data_t()); } // invalid data
      }
    }// res empty for end of file marker 0
    return res;
  }
  bool compute_step( solution_t& s, int mean) const {
    bool shifted(false);// set to true if we performed a shift during this step
    s.push_back(s.back());
    int delta(0);
    for(data_t::iterator it(s.back().begin()); it != s.back().end(); delta+= *it++ - mean){
      if ( delta < 0) {
        --(*it); ++(*boost::prior(it)); shifted= true;
      } else if ( *it > mean ) {
        --(*it); ++(*boost::next(it)); shifted= true;
      }
    }
    return shifted;
  }
};
template<typename Os> Os& operator<<(Os& os, solution_t const& s){
  if( !s.empty() ) {
    if(s.back().empty()) { os<< "-1"; } 
    else {
      os << s.size() << " :\n";
      for(std::size_t i(0); i != s.size(); ++i) {
        os << i;
        for(data_t::const_iterator it(s[i].begin()); it!= s[i].end(); ++it)
          { os << ((it== s[i].begin()) ? " : (" : ", ") << *it;}
        os << ")\n";
      }
    }
    os << "\n";
  }
  return os;
}
template<typename Is> Is& operator>>(Is& is, data_t& d) {
  std::size_t nb(0);
  is >> nb;
  if( nb ) {
    d.resize(nb);
    for(std::size_t i(0); i != nb; ++i)
      { is >> d[i]; }
  } else { d.clear(); }
  return is;
}
int main(int argc, char* argv[]){
  std::ifstream in("input.txt");
  std::ofstream out("output.txt");
  data_t d;
  process p;
  while ( in >> d ) { out << p(d); }
  return 0;
}
