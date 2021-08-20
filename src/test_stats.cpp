#include <math.h>
#include "gtest/gtest.h"

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


TEST(Deck,Stats10) {
  int cards = 10;
  Deck deck(cards);
  Deck spin(deck);
  std::vector<int> ciphers(cards,0);
  std::vector<int> cuts(cards,0);
  std::vector< std::vector < int > > ciphers2 ( cards, std::vector<int>(cards,0) );
  std::vector< std::vector < int > > cuts2 ( cards, std::vector<int>(cards,0) );
  uint32_t plain = 0;

  for (int i=0; i<2*cards; ++i) {
    plain = 1664525*plain + 1013904223;    
    spin.mix(Card(plain));
  }

  int n = 10000000;

  int cipher0=0,cipher1=0,cut0=0,cut1=0;
  for (int i=0; i<n; ++i) {
    cipher1=spin.cipherPad().order;
    cut1=spin.cutPad().order;
    ++ciphers.at(cipher1);
    ++cuts.at(cut1);
    if (i > 0) {
      ++ciphers2.at(cipher0).at(cipher1);
      ++cuts2.at(cut0).at(cut1);
    }
    cipher0=cipher1;
    cut0=cut1;
    // https://en.wikipedia.org/wiki/Linear_congruential_generator - Numerical Recipies
    plain = 1664525*plain + 1013904223;
    spin.mix(Card(plain % cards));
  }

  std::vector<double> z_ciphers(cards);
  std::vector<double> z_cuts(cards);
  std::vector< std::vector<double> > z_ciphers2(cards, std::vector<double> (cards,0.0) );
  std::vector< std::vector<double> > z_cuts2(cards, std::vector<double> (cards,0.0) );

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
    }
  }
  
  std::cout << "ciphers: " << ciphers << std::endl;
  std::cout << "cuts: " << cuts << std::endl;  

  std::cout << "z_ciphers: " << z_ciphers << std::endl;
  std::cout << "z_cuts: " << z_cuts << std::endl;  

  std::cout << "ciphers2: " << ciphers2 << std::endl;
  std::cout << "cuts2: " << cuts2 << std::endl;  

  std::cout << "z_ciphers2: " << z_ciphers2 << std::endl;
  std::cout << "z_cuts2: " << z_cuts2 << std::endl;  

  for (int i=0; i<cards; ++i) {
    ASSERT_LT(fabs(z_ciphers[i]),8.0);
    ASSERT_LT(fabs(z_cuts[i]),8.0);    
  }

  double z_luck = 0;
  int df = 0;
  for (int i=0; i<cards; ++i) {
    z_luck = z_luck + pow(z_ciphers[i],2) + pow(z_cuts[i],2);
    df += 2;
  }
  // https://github.com/wmacevoy/luck/blob/master/main.pdf
  z_luck = sqrt(z_luck) -sqrt(df-0.5);
  std::cout << "z_luck = " << z_luck << std::endl;
  ASSERT_LT(fabs(z_luck),8.0);

  double z2_luck = 0;
  int df2 = 0;
  for (int i=0; i<cards; ++i) {
    for (int j=0; j<cards; ++j) {    
      z2_luck = z2_luck + pow(z_ciphers2[i][j],2) + pow(z_cuts2[i][j],2);
      df2 += 2;
    }
  }
  // https://github.com/wmacevoy/luck/blob/master/main.pdf
  z2_luck = sqrt(z2_luck) -sqrt(df2-0.5);
  std::cout << "z2_luck = " << z2_luck << std::endl;
  ASSERT_LT(fabs(z2_luck),8.0);
}

TEST(Deck,Stats40) {
  int cards = 40;
  Deck deck(cards);
  Deck spin(deck);
  std::vector<int> ciphers(cards,0);
  std::vector<int> cuts(cards,0);
  std::vector< std::vector < int > > ciphers2 ( cards, std::vector<int>(cards,0) );
  std::vector< std::vector < int > > cuts2 ( cards, std::vector<int>(cards,0) );
  std::vector< std::vector < int > > xy ( cards, std::vector<int>(cards,0) );  
  uint32_t plain = 0;

  for (int i=0; i<2*cards; ++i) {
    plain = 1664525*plain + 1013904223;    
    spin.mix(Card(plain));
  }

  int n = 100000000;

  int cipher0=0,cipher1=0,cut0=0,cut1=0;
  for (int i=0; i<n; ++i) {
    cipher1=spin.cipherPad().order;
    cut1=spin.cutPad().order;
    ++ciphers.at(cipher1);
    ++cuts.at(cut1);
    ++xy.at(cut1).at(cipher1);
    if (i > 0) {
      ++ciphers2.at(cipher0).at(cipher1);
      ++cuts2.at(cut0).at(cut1);
    }
    cipher0=cipher1;
    cut0=cut1;
    // https://en.wikipedia.org/wiki/Linear_congruential_generator - Numerical Recipies
    plain = 1664525*plain + 1013904223;
    spin.mix(Card(plain % cards));
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
  
  std::cout << "ciphers: " << ciphers << std::endl;
  std::cout << "cuts: " << cuts << std::endl;  

  std::cout << "z_ciphers: " << z_ciphers << std::endl;
  std::cout << "z_cuts: " << z_cuts << std::endl;  

  //  std::cout << "ciphers2: " << ciphers2 << std::endl;
  //  std::cout << "cuts2: " << cuts2 << std::endl;  

  //  std::cout << "z_ciphers2: " << z_ciphers2 << std::endl;
  //  std::cout << "z_cuts2: " << z_cuts2 << std::endl;
  std::cout << "z_xy: " << z_xy << std::endl;    

  for (int i=0; i<cards; ++i) {
    ASSERT_LT(fabs(z_ciphers[i]),8.0);
    ASSERT_LT(fabs(z_cuts[i]),8.0);    
  }

  double z_luck = 0;
  double z_ext_luck = 0;  
  int df = 0;
  int df_ext = 0;
  for (int i=0; i<cards; ++i) {
    z_luck = z_luck + pow(z_ciphers[i],2) + pow(z_cuts[i],2);
    z_ext_luck = z_ext_luck + pow(z_ciphers[i],2);
    df += 2;
    df_ext += 1;    
  }
  // https://github.com/wmacevoy/luck/blob/master/main.pdf
  z_luck = sqrt(z_luck) -sqrt(df-0.5);
  z_ext_luck = sqrt(z_ext_luck) -sqrt(df_ext-0.5);  
  std::cout << "z_luck = " << z_luck << std::endl;
  std::cout << "z_ext_luck = " << z_ext_luck << std::endl;  
  ASSERT_LT(fabs(z_luck),8.0);

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
  z2_ext_luck = sqrt(z2_luck) -sqrt(df2_ext-0.5);  
  std::cout << "z2_luck = " << z2_luck << std::endl;
  std::cout << "z2_ext_luck = " << z2_ext_luck << std::endl;   
  ASSERT_LT(fabs(z2_luck),8.0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
