#include <iostream>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

TEST(Search,Cycle) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);
  search.find();
  std::cout << "cycle path (len=" << search.path.size() << "): " << search.path << std::endl;
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
