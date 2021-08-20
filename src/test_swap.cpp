#include <iostream>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

TEST(Search,Swap10) {
  int n=10;
  Deck a(n);
  Deck b(n);
  b.cards[0]=Card(1);
  b.cards[1]=Card(0);
  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;  
  Search search(a,b);
  search.find();
  std::cout << "swap path (len=" << search.path.size() << "): " << search.path << std::endl;
  for (auto card : search.path) {
    std::cout << a << std::endl;
    a.mix(card);
  }
  std::cout << a << std::endl;  
  ASSERT_EQ(a,b);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
