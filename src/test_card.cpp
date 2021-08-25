#include <iostream>
#include "gtest/gtest.h"
#include "card.h"

using namespace std;
using namespace spider;

static void io(const char *name, int order) {
  for (int ws=0; ws<2; ++ws) {
    std::ostringstream oss1,oss2,oss3;
    if (ws) {
      oss1 << " " << name << " ( " << order << " ) ";
    } else {
      oss1 << name << "(" << order << ")";    
    }
    oss2 << name << "(" << order << ")";

    std::istringstream iss(oss1.str());

    Card card;
    iss >> card;
    ASSERT_TRUE(iss) << " ws=" << ws << std::endl;
    ASSERT_EQ(iss.peek(),(ws) ? ' ' : -1) << " ws=" << ws << std::endl;
    ASSERT_EQ(card.order,order) << " ws=" << ws << std::endl;

    oss3 << card;
    ASSERT_EQ(oss2.str(),oss3.str()) << " ws=" << ws << std::endl;
  }
}

TEST(Card,Jokers) {
  Card AJ(52);
  Card BJ(53);

  ASSERT_TRUE(AJ.joker());
  ASSERT_TRUE(BJ.joker());
  ASSERT_EQ(AJ.faceNumber(),0);
  ASSERT_EQ(BJ.faceNumber(),1);
  ASSERT_EQ(AJ.suiteNumber(),4);
  ASSERT_EQ(BJ.suiteNumber(),4);
  ASSERT_EQ(AJ.suite(),std::string("J"));
  ASSERT_EQ(BJ.suite(),std::string("J"));  
  ASSERT_EQ(AJ.face(),std::string("A"));
  ASSERT_EQ(BJ.face(),std::string("B"));
}

TEST(Card,Names) {
  io("AC",1);
  io("AD",11);
  io("AH",21);
  io("AS",31);
  io("10C",0);
  io("10D",10);
  io("10H",20);
  io("10S",30);
  io("2C",2);
  io("2D",12);
  io("2H",22);
  io("2S",32);
  io("9C",9);
  io("9D",19);
  io("9H",29);
  io("9S",39);
  io("AJ",52);
  io("BJ",53);
}

TEST(Card,Vec0) {
  std::string text="[]";
  std::vector<Card> cards;
  std::istringstream iss(text);
  std::vector<Card> inCards;
  ASSERT_TRUE(iss >> inCards);
  ASSERT_EQ(cards,inCards);
  std::ostringstream oss;
  oss << cards;
  ASSERT_EQ(text,oss.str());
}

TEST(Card,Vec1) {
  std::string text="[3H(23)]";
  std::vector<Card> cards;
  cards.push_back(Card(23));
  std::istringstream iss(text);
  std::vector<Card> inCards;
  ASSERT_TRUE(iss >> inCards);
  ASSERT_EQ(cards,inCards);
  std::ostringstream oss;
  oss << cards;
  ASSERT_EQ(text,oss.str());
}

TEST(Card,Vec2) {
  std::string text="[3H(23),AC(1),10D(10),9S(39)]";
  std::vector<Card> cards;
  
  cards.push_back(Card(23));
  cards.push_back(Card(1));
  cards.push_back(Card(10));
  cards.push_back(Card(39));
  
  std::istringstream iss(text);
  std::vector<Card> inCards;
  ASSERT_TRUE(iss >> inCards);
  ASSERT_EQ(cards,inCards);
  std::ostringstream oss;
  oss << cards;
  ASSERT_EQ(text,oss.str());
}

TEST(Card,AddMod) {
  for (auto n : {10, 40, 41, 52, 54}) {  
    for (auto m : {10, 40}) {
      for (int i=0; i<n; ++i) {
	for (int j=0; j<n; ++j) {
	  Card a(i);
	  Card b(j);
	  Card c((i+j) % m);
	  ASSERT_EQ(a.order,i);
	  ASSERT_EQ(b.order,j);
	  ASSERT_EQ(addMod(a,b,m),c);
	}
      }
    }
  }
}

TEST(Card,SubMod) {
  for (auto n : {10, 40, 41, 52, 54}) {  
    for (auto m : {10, 40}) {
      for (int i=0; i<n; ++i) {
	for (int j=0; j<n; ++j) {
	  Card a(i);
	  Card b(j);
	  int ans = i-j;
	  while (ans < 0) { ans += m; }
	  Card c(ans);
	  ASSERT_EQ(a.order,i);
	  ASSERT_EQ(b.order,j);
	  ASSERT_EQ(subMod(a,b,m),c);
	}
      }
    }
  }
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
