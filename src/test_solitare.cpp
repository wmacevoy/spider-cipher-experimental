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

  z = sqrt(2.0)*(sqrt(z)-sqrt((counts.size()-1)-0.5));

  return z;
}

TEST(Solitaire,Stats) {
  OS_RNG rng;
  int nt = 1000*1000;
  int nz = 100;

  std::cout << "nz=" << nz << std::endl;  
  std::cout << "nt=" << nt << std::endl;

  Stats padStats1,padStats2,padStats3,padStats4;
  Stats rngStats1,rngStats2,rngStats3,rngStats4;
  for (int z=0; z<nz; ++z) {
    Solitaire solitaire;
    solitaire.shuffle(rng);
    auto padCounts1 = std::vector< int > ( 52, 0 );
    auto padCounts2 = std::vector< int > ( 52*52, 0);
    auto padCounts3 = std::vector< int > ( 52*52*52, 0 );
    auto padCounts4 = std::vector< int > ( 52*52*52*52, 0);

    auto rngCounts1 = std::vector< int > ( 52, 0 );
    auto rngCounts2 = std::vector< int > ( 52*52, 0);
    auto rngCounts3 = std::vector< int > ( 52*52*52, 0);
    auto rngCounts4 = std::vector< int > ( 52*52*52*52, 0);

    int pad0=0,pad1=0,pad2=0,pad3=0,rng0=0,rng1=0,rng2=0,rng3=0;
    for (int t=0; t<nt; ++t) {
      do {
	solitaire.next();
      } while (!solitaire.valid());
      pad0=solitaire.pad();
      rng0=rng.next(0,51);
      
      ++padCounts1[pad0];
      ++rngCounts1[rng0];
      if (t % 2 == 1) {
	++padCounts2[pad1*52+pad0];
	++rngCounts2[rng1*52+rng0];
      }
      if (t % 3 == 2) {
	++padCounts3[pad2*52*52+pad1*52+pad0];
	++rngCounts3[rng2*52*52+rng1*52+rng0];
      }
      if (t % 4 == 3) {
	++padCounts4[pad3*52*52*52+pad2*52*52+pad1*52+pad0];
	++rngCounts4[rng3*52*52*52+rng2*52*52+rng1*52+rng0];
      }
      pad3=pad2;
      pad2=pad1;
      pad1=pad0;

      rng3=rng2;
      rng2=rng1;
      rng1=rng0;
    }
    padStats1.add(z_luck(padCounts1));
    padStats2.add(z_luck(padCounts2));
    padStats3.add(z_luck(padCounts3));
    padStats4.add(z_luck(padCounts4));
    rngStats1.add(z_luck(rngCounts1));
    rngStats2.add(z_luck(rngCounts2));
    rngStats3.add(z_luck(rngCounts3));
    rngStats4.add(z_luck(rngCounts4));
  }

  std::cout << "padStats1: "; padStats1.println(std::cout);
  std::cout << "padStats2: "; padStats2.println(std::cout);  
  std::cout << "padStats3: "; padStats3.println(std::cout);
  std::cout << "padStats4: "; padStats4.println(std::cout);  

  std::cout << "rngStats1: "; rngStats1.println(std::cout);
  std::cout << "rngStats2: "; rngStats2.println(std::cout);
  std::cout << "rngStats3: "; rngStats3.println(std::cout);
  std::cout << "rngStats4: "; rngStats4.println(std::cout);  
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
