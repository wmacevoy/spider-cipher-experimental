#include <iostream>
#include <math.h>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

const int MAX_FDIST = 3;
const int MAX_RDIST = 3;

TEST(Search,Forward) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);

  while (search.fdist < MAX_FDIST) {
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

  while (search.rdist < MAX_RDIST) {
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

  while (!search.done()) {
    search.grow();
    std::cout << "finished search dist " << search.dist << std::endl;
  }

  auto path=search.paths[0];
  
  std::cout << "cycle path (len=" << path.size() << "): " << path << std::endl;
  for (auto card : path) {
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
