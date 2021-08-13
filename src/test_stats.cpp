#include <math.h>
#include "gtest/gtest.h"

#include "card.h"
#include "deck.h"

using namespace std;
using namespace spider;

TEST(Deck,Stats) {
  Deck deck40(40);
  Deck spin(deck40);
  int ciphers[40],cuts[40];
  for (int i=0; i<40; ++i) {
    ciphers[i]=0;
    cuts[i]=0;
  }
  int n = 100000000;
  for (int i=0; i<n; ++i) {
    ++ciphers[spin.cipherPad().order];
    ++cuts[spin.cutPad().order];
    spin.pseudoShuffle(Card((spin.cutPad().order+(i%10))%40));
  }

  double z_ciphers[40],z_cuts[40];
  double p = 1.0/40.0;
  double q = 39.0/40.0;

  for (int i=0; i<40; ++i) {
    z_ciphers[i]=(ciphers[i]-n*p)/sqrt(n*p*q);
    z_cuts[i]=(cuts[i]-n*p)/sqrt(n*p*q);
  }

  for (int i=0; i<40; ++i) {
    ASSERT_LT(fabs(z_ciphers[i]),6.0);
    ASSERT_LT(fabs(z_cuts[i]),6.0);    
  }

}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
