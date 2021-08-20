#include <iostream>
#include <math.h>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

TEST(Search,Forward) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);
  search.unique = true;
  while (search.fdist < 5) {
    search.growForward();
    ASSERT_EQ(search.fboundary.size(),pow(40,search.fdist));
    std::cout << "fdist = " << search.fdist << std::endl;
  }
}

TEST(Search,Reverse) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);
  search.unique = true;
  while (search.rdist < 5) {
    search.growReverse();
    ASSERT_EQ(search.rboundary.size(),pow(40,search.rdist));
    std::cout << "rdist = " << search.rdist << std::endl;    
  }
}
TEST(Search,Cycle) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);
  search.unique = true;
  search.find();
  std::cout << "cycle path (len=" << search.path.size() << "): " << search.path << std::endl;
  for (auto card : search.path) {
    std::cout << a << std::endl;
    a.mix(card);
  }
  std::cout << a << std::endl;  
  ASSERT_EQ(a,b);
  std::cout << "cycle test done." << std::endl;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
