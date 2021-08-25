#include <iostream>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

TEST(Search,Forward) {
  int n=40;
  for (int len=1; len <= 3; ++len) {
    std::vector<Card> path;
    for (int i=0; i<len; ++i) {
      path.push_back(Card(i));
    }
    Deck a(n);
    Deck b(n);
    for (int i=0; i<len; ++i) {
      std::cout << b << ".mix(" << Card(i) << ")=";
      b.mix(path[i]);
      std::cout << b << std::endl;
    }

    Search search(a,b);
    while (!search.done()) {
      search.growForward();
    }
    ASSERT_EQ(search.paths[0],path);
  }
}

TEST(Search,Reverse) {
  int n=40;
  for (int len=1; len <= 3; ++len) {
    std::vector<Card> path;
    for (int i=0; i<len; ++i) {
      path.push_back(Card(i));
    }
    Deck a(n);
    Deck b(n);
    for (int i=0; i<len; ++i) {
      b.mix(path[i]);
    }

    Search search(a,b);
    while (!search.done()) {
      search.growReverse();
    }
    ASSERT_EQ(search.paths[0],path);
  }
}


TEST(Search,Path4) {
  int n=40;
  for (int len=1; len <= 6; ++len) {
    std::vector<Card> path;
    for (int i=0; i<len; ++i) {
      path.push_back(Card(i));
    }
    Deck a(n);
    Deck b(n);
    for (int i=0; i<len; ++i) {
      b.mix(path[i]);
    }

    Search search(a,b);

    search.find();

    ASSERT_EQ(search.paths[0],path);
  }
}

TEST(Search,Cycle) {
  int n=40;
  Deck a(n);
  Deck b(n);
  Search search(a,b);
  search.find();
  auto path = search.paths[0];
  std::cout << "cycle path (len=" << path.size() << "): " << path << std::endl;
  for (auto card : path) {
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
