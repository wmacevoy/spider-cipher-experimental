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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
