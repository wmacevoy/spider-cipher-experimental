#include <math.h>
#include "gtest/gtest.h"

#include "rng.h"
#include "card.h"
#include "deck.h"

using namespace std;
using namespace spider;

template <typename X>
std::ostream &operator<<(std::ostream &out, const std::vector<X> &xs)
{
  out << "[";
  bool first = true;
  for (auto x : xs) {
    if (first) {
      first = false;
    } else {
      out << ",";
    }
    out << x;
  }
  out << "]";

  return out;
}


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


void stats(int cards, int trials) {
  Deck deck(cards);
  int m = deck.modulus();
  OS_RNG rng;
  std::vector<int> ciphers(cards,0);
  std::vector<int> cuts(cards,0);
  std::vector< std::vector < int > > ciphers2 ( cards, std::vector<int>(cards,0) );
  std::vector< std::vector < int > > cuts2 ( cards, std::vector<int>(cards,0) );
  std::vector< std::vector < int > > xy ( cards, std::vector<int>(cards,0) );  

  for (int i=0; i<cards; ++i) {
    int j=rng.next(i,cards-1);
    Card tmp=deck.cards[i];
    deck.cards[i]=deck.cards[j];
    deck.cards[j]=tmp;
  }
  
  int n = trials;

  int cipher0=0,cipher1=0,cut0=0,cut1=0,plain=0;
  for (int i=0; i<n; ++i) {
    cipher1=deck.cipherPad().order;
    cut1=deck.cutPad().order;
    ++ciphers.at(cipher1);
    ++cuts.at(cut1);
    ++xy.at(cut1).at(cipher1);
    if (i > 0) {
      ++ciphers2.at(cipher0).at(cipher1);
      ++cuts2.at(cut0).at(cut1);
    }
    cipher0=cipher1;
    cut0=cut1;
    if (i < 10 || i%2 == 0) {
      plain = rng.next(0,m-1);
    } else {
      plain = (plain + (m-1)) % m;
    }
    deck.mix(plain);
  }

  std::vector<double> z_ciphers(cards);
  std::vector<double> z_cuts(cards);
  std::vector< std::vector<double> > z_ciphers2(cards, std::vector<double> (cards,0.0) );
  std::vector< std::vector<double> > z_cuts2(cards, std::vector<double> (cards,0.0) );
  std::vector< std::vector<double> > z_xy(cards, std::vector<double> (cards,0.0) );  

  double p = 1.0/double(cards);
  double q = 1-p;
  double p2 = 1.0/double(pow(cards,2));
  double q2 = 1-p2;

  for (int i=0; i<cards; ++i) {
    z_ciphers[i]=(ciphers[i]-n*p)/sqrt(n*p*q);
    z_cuts[i]=(cuts[i]-n*p)/sqrt(n*p*q);
  }

  for (int i=0; i<cards; ++i) {
    for (int j=0; j<cards; ++j) {    
      z_ciphers2[i][j]=(ciphers2[i][j]-n*p2)/sqrt(n*p2*q2);
      z_cuts2[i][j]=(cuts2[i][j]-n*p2)/sqrt(n*p2*q2);
      z_xy[i][j]=(xy[i][j]-n*p2)/sqrt(n*p2*q2);    
    }
  }
  
  // std::cout << "ciphers: " << ciphers << std::endl;
  // std::cout << "cuts: " << cuts << std::endl;  

  // std::cout << "z_ciphers: " << z_ciphers << std::endl;
  // std::cout << "z_cuts: " << z_cuts << std::endl;  

  //  std::cout << "ciphers2: " << ciphers2 << std::endl;
  //  std::cout << "cuts2: " << cuts2 << std::endl;  

  // std::cout << "z_ciphers2: " << z_ciphers2 << std::endl;
  //  std::cout << "z_cuts2: " << z_cuts2 << std::endl;
  // std::cout << "z_xy: " << z_xy << std::endl;    

  for (int i=0; i<cards; ++i) {
    ASSERT_LT(fabs(z_ciphers[i]),8.0);
    ASSERT_LT(fabs(z_cuts[i]),8.0);    
  }

  double z_all_luck = 0;
  double z_ext_luck = 0;  
  int df = 0;
  int df_ext = 0;
  for (int i=0; i<cards; ++i) {
    z_all_luck = z_all_luck + pow(z_ciphers[i],2) + pow(z_cuts[i],2);
    z_ext_luck = z_ext_luck + pow(z_ciphers[i],2);
    df += 2;
    df_ext += 1;    
  }
  // https://github.com/wmacevoy/luck/blob/master/main.pdf
  z_all_luck = sqrt(z_all_luck) -sqrt(df-0.5);
  z_ext_luck = sqrt(z_ext_luck) -sqrt(df_ext-0.5);
  std::cout << "z_luck(ciphers) = " << z_luck(ciphers) << std::endl;
  std::cout << "z2_luck(ciphers2) = " << z2_luck(ciphers2) << std::endl;  
  std::cout << "z_luck(cuts) = " << z_luck(cuts) << std::endl;
  std::cout << "z2_luck(cuts2) = " << z2_luck(cuts2) << std::endl;  
  std::cout << "z2_luck(xy) = " << z2_luck(xy) << std::endl;  
  std::cout << "z_all_luck = " << z_all_luck << std::endl;
  std::cout << "z_ext_luck = " << z_ext_luck << std::endl;  
  ASSERT_LT(fabs(z_all_luck),8.0);

  double z2_luck = 0;
  double z2_ext_luck = 0;  
  int df2 = 0;
  int df2_ext = 0;  
  for (int i=0; i<cards; ++i) {
    for (int j=0; j<cards; ++j) {    
      z2_luck = z2_luck + pow(z_ciphers2[i][j],2) + pow(z_cuts2[i][j],2) + pow(z_xy[i][j],2);
      df2 += 3;
      z2_ext_luck = z2_ext_luck + pow(z_ciphers2[i][j],2);
      df2_ext += 1;
    }
  }
  // https://github.com/wmacevoy/luck/blob/master/main.pdf
  z2_luck = sqrt(z2_luck) -sqrt(df2-0.5);
  z2_ext_luck = sqrt(z2_ext_luck)-sqrt(df2_ext-0.5);  
  std::cout << "z2_luck = " << z2_luck << std::endl;
  std::cout << "z2_ext_luck = " << z2_ext_luck << std::endl;   
  ASSERT_LT(fabs(z2_luck),8.0);
}

TEST(Deck,Luck) {
  OS_RNG rng;

  int n = 100*1000*1000;

  int k = 40;

  std::vector < int > bins(k,0);
  std::vector < std::vector < int > > bins2(k, std::vector <int> (k, 0));
  std::vector < std::vector < std::vector < int > > > bins3(k, std::vector < std::vector <int> > (k,std::vector<int> (k, 0)));

  for (int i=0; i<n; ++i) {
    int a = rng.next(0,k-1);
    int b = rng.next(0,k-1);
    int c = rng.next(0,k-1);    
    ++bins[a];
    ++bins2[a][b];
    ++bins3[a][b][c];    
  }

  double z=z_luck(bins);

  double z2=z2_luck(bins2);


  std::cout << "z_luck(os_rand)=" << z << std::endl;
  std::cout << "z2_luck(os_rand)=" << z2 << std::endl;

  ASSERT_LT(fabs(z),8);
  ASSERT_LT(fabs(z2),8);  

}

TEST(Deck,Stats10) {
  int cards = 10;
  int trials = /* 10* */ 1000*1000;
  stats(cards,trials);
}

TEST(Deck,Stats40) {
  int cards = 40;
  int trials = 100*1000*1000;

  stats(cards,trials);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
