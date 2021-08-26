#include <iostream>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;

void shuffle(Deck &deck, int a=33, int b=17) {
  int n=deck.cards.size();
  for (int i=0; i<n; ++i) {
    //    int m = (i == 0) ? (n == 10 ? 10 : 40) : n-i;
    int j=i+((a*i+b) % (n-i));
    Card tmp = deck.cards[i];
    deck.cards[i]=deck.cards[j];
    deck.cards[j]=tmp;
  }
}

TEST(Search,SearchSet) {
  for (auto n : {10, 40, 41, 52, 54}) {  
    SearchSet s;
    Deck a(n),b(n);
    shuffle(a);
    shuffle(b);
        
    s.insert(a);
    s.insert(b);
    ASSERT_EQ(s.size(),1);
    ASSERT_NE(s.find(a),s.end());
    ASSERT_NE(s.find(b),s.end());
    if (n > 40) {
      SearchSet t;
      Deck c(n);
      Deck::cut(a.cards,Deck::find(a.cards,Card(n-1)),c.cards);
      Deck::cut(a.cards,Deck::forward(a.cards,Deck::find(a.cards,Card(n-1)),0),b.cards);
      ASSERT_EQ(SearchSetCmp()(b,c),false);
      ASSERT_EQ(SearchSetCmp()(c,b),false);
      t.insert(b);
      t.insert(c);
      ASSERT_EQ(t.size(),1);
      ASSERT_NE(t.find(b),s.end());
      ASSERT_NE(t.find(c),s.end());
    }
  }
}

TEST(Search,Forward) {
  for (auto n : {10, 40, 41, 52, 54}) {
    for (auto len : {1,2,3}) {
      Deck a(n);
      Deck b(n);
      std::vector<Card> path(len);
      for (int i=0; i<len; ++i) {
	path[i]=(33*i+17) % a.modulus();
      }
      for (int i=0; i<len; ++i) {
	b.mix(path[i]);
      }
      Search search(a,b);
      search.maxDist = (len+1);
      while (!search.done()) {
	search.growForward();

      }
      ASSERT_EQ(search.paths.size(),1) << "n=" << n << " len=" << len << " path=" << path;
      ASSERT_EQ(search.paths[0],path);
    }
  }
}

TEST(Search,Reverse) {
  for (auto n : {10, 40, 41, 52, 54}) {
    for (auto len : {1,2,3}) {
      Deck a(n);
      Deck b(n);
      std::vector<Card> path(len);
      for (int i=0; i<len; ++i) {
	path[i]=(33*i+17) % a.modulus();
      }
      for (int i=0; i<len; ++i) {
	b.mix(path[i]);
      }
      Search search(a,b);
      search.maxDist = (len+1);
      while (!search.done()) {
	search.growReverse();
      }
      ASSERT_EQ(search.paths.size(),1) << "n=" << n << " len=" << len << " path=" << path;
      ASSERT_EQ(search.paths[0],path);
    }
  }
}

TEST(Search,Bidirection) {
  for (auto n : {10, 40}) {
    for (auto len : {1,2,3,4,5}) {
      Deck a(n);
      Deck b(n);
      std::vector<Card> path(len);
      for (int i=0; i<len; ++i) {
	path[i]=(33*i+17) % a.modulus();
      }
      for (int i=0; i<len; ++i) {
	b.mix(path[i]);
      }
      Search search(a,b);
      search.maxDist = (len+1);
      while (!search.done()) {
	search.grow();
      }
      ASSERT_EQ(search.paths.size(),1) << "n=" << n << " len=" << len << " path=" << path;
      ASSERT_EQ(search.paths[0],path);
    }
  }
}

TEST(Search,Cycle) {
  for (auto n : { 10, 40}) {
    Deck a(n);
    Deck b(n);
    Search search(a,b);
    search.find();
    auto path = search.paths[0];
    std::cout << "cycle path (n=" << n << " len=" << path.size() << "): " << path << std::endl;
    for (auto card : path) {
      a.mix(card);
    }
    ASSERT_EQ(a,b);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
