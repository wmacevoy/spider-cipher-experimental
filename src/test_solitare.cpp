#include <iostream>
#include <math.h>
#include "gtest/gtest.h"
#include "solitaire.h"

using namespace std;
using namespace spider;

std::ostream &operator<<(std::ostream &out, const std::vector<int> &x) {
  out << "[";
  for (size_t i=0; i<x.size(); ++i) {
    if (i>0) out << ",";
    out << x[i];
  }
  out << "]";
  return out;
}

TEST(Solitare,KeyStream) {
  std::vector<int> expect = {4,49,10,53,24,8,51,44,6,4,33,20,39,19,34,42};
  Solitaire solitaire;
  for (int i=0; i<expect.size(); ++i) {
    solitaire.next();
    ASSERT_EQ(solitaire.pad()+1,expect[i]);
  }
}

struct Stats {
  double total;
  double sum;
  double sum2;
  double min;
  double max;
  void reset() {
    total = 0;
    sum  = 0;
    sum2 = 0;
    min = 0;
    max = 0;
  }

  Stats() {
    reset();
  }
  
  void add(double x, double w=1.0) {
    if (total == 0 || x < min) {
      min = x;
    }
    if (total == 0 || max < x) {
      max = x;
    }
    total += w;
    sum += w*x;
    sum2 += w*x*x;
  }

  double mean() const  {
    return sum / total;
  }
  double sd() const {
    return sqrt(sum2/total-pow(sum/total,2));
  }

  void println(std::ostream &out) const {
    out << "total=" << total << " mean=" << mean() << " sd=" << sd() << " min=" << min << " max=" << max << std::endl;
  }

};

double z_luck(const std::vector<int> &counts) {
  int n=0;
  for (int i=0; i<counts.size(); ++i) {
    n += counts[i];
  }
  double p = 1.0/double(counts.size());
  double q = 1-p;  
  double mu = n*p;
  double inv_sigma = 1/sqrt(n*p);
  double z=0.0;

  for (int i=0; i<counts.size(); ++i) {
    z += pow(inv_sigma*(counts[i]-mu),2);
  }

  z = sqrt(z)-sqrt((counts.size()-1)-0.5);

  return z;
}

double z2_luck(const std::vector< std::vector<int> > &counts) {
  int n2=0;
  for (int i=0; i<counts.size(); ++i) {
    for (int j=0; j<counts[i].size(); ++j) {
      n2 += counts[i][j];
    }
  }
  double p = 1.0/pow(double(counts.size()),2);
  double q = 1-p;
  double mu = n2*p;
  double inv_sigma = 1/sqrt(n2*p);
  double z2=0;

  for (int i=0; i<counts.size(); ++i) {
    for (int j=0; j<counts[i].size(); ++j) {    
      z2 += pow(inv_sigma*(counts[i][j]-mu),2);
    }
  }

  z2 = sqrt(z2)-sqrt(pow(counts.size()-1,2)-0.5);

  return z2;
}


TEST(Solitaire,Stats) {
  OS_RNG rng;
  int nt = 1000*1000;
  int nz = 100;

  Stats s,s2;
  for (int z=0; z<nz; ++z) {
    Solitaire solitaire;
    solitaire.shuffle(rng);
    auto c = std::vector< int > ( 54, 0 );
    auto c2 = std::vector< std::vector < int > > ( 54, std::vector<int>(54,0) );

    int c0=0,c1=0;
    for (int t=0; t<nt; ++t) {
      do {
	solitaire.next();
      } while (!solitaire.valid());
      c1=solitaire.pad();
      ++c[c1];
      if (t > 0) {
	++c2[c1][c0];
      }
      c0=c1;
    }
    s.add(z_luck(c));
    s2.add(z2_luck(c2));
  }

  std::cout << "nz=" << nz << " nt=" << nt << std::endl;
  std::cout << "s: "; s.println(std::cout);
  std::cout << "s2: "; s2.println(std::cout);  
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
