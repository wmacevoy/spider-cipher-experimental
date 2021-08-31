#include <iostream>
#include "gtest/gtest.h"
#include "retain.hpp"
#include "deck.h"

using namespace std;
using namespace spider;


void io(std::string expect) {
  std::istringstream iss(expect);
  std::vector<Card> expected;
  expected.clear();
  ASSERT_TRUE(iss >> expected);
  std::ostringstream oss;
  oss << expected;
  ASSERT_EQ(oss.str(),expect);
}


void ps(int n,int k, std::string text) {
  Deck deck(n),expect(n);
  std::istringstream iss(text);
  iss >> expect.cards;
  std::ostringstream oss;
  oss << expect.cards;
  ASSERT_EQ(text,oss.str());
  deck.pseudoShuffle(Card(k));
  ASSERT_EQ(deck, expect);
}


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

TEST(Deck,TestShuffle) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    shuffle(a);
    int counts[n];
    for (int i=0; i<n; ++i) {
      counts[i]=0;
    }
    for (int i=0; i<a.cards.size(); ++i) {
      ASSERT_LE(Card(0),a.cards[i]);
      ASSERT_LT(a.cards[i],Card(n));
      ++counts[a.cards[i].order];
    }
    for (int i=0; i<n; ++i) {
      ASSERT_EQ(counts[i],1);
    }
  }
}

TEST(Deck,DefaultConfig) {
  const DeckConfig &cfg=Deck::config();
  ASSERT_EQ(cfg.cipherZth,DeckConfig::DEFAULT_CIPHER_ZTH);
  ASSERT_EQ(cfg.cipherOffset,DeckConfig::DEFAULT_CIPHER_OFFSET);
  ASSERT_EQ(cfg.cutZth,DeckConfig::DEFAULT_CUT_ZTH);
  ASSERT_EQ(cfg.cutOffset,DeckConfig::DEFAULT_CUT_OFFSET);
}

void test_recall() {
  const DeckConfig &cfg=Deck::config();
  ASSERT_EQ(cfg.cipherZth,-1);
  ASSERT_EQ(cfg.cipherOffset,-2);
  ASSERT_EQ(cfg.cutZth,-3);
  ASSERT_EQ(cfg.cutOffset,-4);
}

TEST(Deck,RetainConfig) {
  DeckConfig cfg;
  cfg.cipherZth = -1;
  cfg.cipherOffset = -2;
  cfg.cutZth = -3;
  cfg.cutOffset = -4;
  retain<const DeckConfig> as(&cfg);
  test_recall();
}

std::vector<DeckConfig> configs() {
  std::vector<DeckConfig> cfgs;
  DeckConfig cfg;  
  for (auto cipherZth : {0,1,2,3,4,5}) {
    cfg.cipherZth = cipherZth;
    for (auto cipherOffset : {-1,0,1,2,3,4,5,35,36,37,38,39}) {
      cfg.cipherOffset=cipherOffset;
      for (auto cutZth : {0,1,2,3,4,5}) {
	cfg.cutZth = cutZth;
	for (auto cutOffset : {-1,0,1,2,3,4,5,35,36,37,38,39}) {
	  cfg.cutOffset=cutOffset;
	  cfgs.push_back(cfg);
	}
      }
    }
  }
  return cfgs;
}

TEST(Deck,Equivalent) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n),b(n),c(n);
    ASSERT_EQ(equivalent(a,b),0);
    shuffle(a);
    shuffle(b);
    ASSERT_EQ(equivalent(a,b),0);
    ASSERT_NE(equivalent(a,c),0);

    if (n <= 40) continue;
    
    Deck::cut(a.cards,Deck::find(a.cards,Card(n-1)),c.cards);
    Deck::cut(a.cards,Deck::forward(a.cards,Deck::find(a.cards,Card(n-1)),0),b.cards);
    ASSERT_NE(c,b);
    ASSERT_EQ(equivalent(c,b),0);
    ASSERT_EQ(c.cipherPad(),b.cipherPad());
    ASSERT_EQ(c.cutPad(),b.cutPad());
    for (auto cfg : configs()) {
      retain<const DeckConfig> as(&cfg);
      for (int i=0; i<a.modulus(); ++i) {
	Deck bmix(b);
	Deck cmix(c);
	bmix.mix(Card(i));
	cmix.mix(Card(i));
	ASSERT_EQ(bmix,cmix);
      }
    }
  }
}

TEST(Deck,Cut) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n),b(n);
    shuffle(a);
    for (int cutLoc=0; cutLoc<n; ++cutLoc) {
      Deck::cut(a.cards,cutLoc,b.cards);
      for (int i=0; i<n; ++i) {
	ASSERT_EQ(b.cards[i],a.cards[(i+cutLoc)%n]);
      }
    }
  }
}

TEST(Deck,BackFrontShuffle) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n),b(n);
    shuffle(a);
    
    std::vector<Card> bfShuffle;
    for (int i=0; i<n; ++i) {
      if (i % 2 == 0) {
	bfShuffle.insert(bfShuffle.end(),a.cards[i]);
      } else {
	bfShuffle.insert(bfShuffle.begin(),a.cards[i]);
      }
    }
    Deck::backFrontShuffle(a.cards,b.cards);

    ASSERT_EQ(b.cards,bfShuffle);
  }
}

TEST(Deck,BackFrontUnshuffle) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n),b(n),c(n);
    shuffle(a);
    Deck::backFrontShuffle(a.cards,b.cards);
    Deck::backFrontUnshuffle(b.cards,c.cards);
    ASSERT_EQ(a.cards,c.cards);
  }
}

TEST(Deck,Forward) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    shuffle(a);
    

    for (unsigned loc=0; loc<n; ++loc) {
      for (unsigned delta=0; delta<n; ++delta) {
	unsigned newLoc = loc;
	while (a.cards[newLoc].order >= 40) {
	  newLoc = (newLoc+1) % n;
	}
	for (int i=0; i<delta; ++i) {
	  newLoc = (newLoc+1) % n;
	  while (a.cards[newLoc].order >= 40) {
	    newLoc = (newLoc+1) % n;
	  }
	}
	ASSERT_EQ(Deck::forward(a.cards,loc,delta),newLoc);
      }
    }
  }
}

TEST(Deck,Back) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    shuffle(a);
    
    for (unsigned loc=0; loc<n; ++loc) {
      for (unsigned delta=0; delta<n; ++delta) {
	unsigned newLoc = loc;
	while (a.cards[newLoc].order >= 40) {
	  newLoc = (newLoc+(n-1)) % n;
	}
	for (int i=0; i<delta; ++i) {
	  newLoc = (newLoc+(n-1)) % n;
	  while (a.cards[newLoc].order >= 40) {
	    newLoc = (newLoc+(n-1)) % n;
	  }
	}
	ASSERT_EQ(Deck::back(a.cards,loc,delta),newLoc);
      }
    }
  }
}

TEST(Deck,Modulus) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    int m = (n == 10) ? 10 : 40;
    ASSERT_EQ(a.modulus(),m);
  }
}

TEST(Deck,Reset) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n),b(n);
    shuffle(a);
    a.reset();
    ASSERT_EQ(a,b);
  }
}

TEST(Deck,Find) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    shuffle(a);
    for (int i=0; i<n; ++i) {
      int loc = Deck::find(a.cards,Card(i));
      ASSERT_EQ(a.cards[loc],Card(i));
    }
  }
}

TEST(Deck,After) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    for (int i=0; i<n; ++i) {
      if (a.cards[i].order < a.modulus()) {
	Card after = Deck::after(a.cards,a.cards[i]);
	int afterLoc = (i + 1) % n;
	while (a.cards[afterLoc] >= 40) {
	  afterLoc = (afterLoc + 1) % n;
	}
	ASSERT_EQ(Deck::find(a.cards,after),afterLoc);
      }
    }
  }
}

TEST(Deck,Before) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    for (int i=0; i<n; ++i) {
      if (a.cards[i].order < a.modulus()) {
	Card before = Deck::before(a.cards,a.cards[i]);
	int beforeLoc = (i + (n-1)) % n;
	while (a.cards[beforeLoc] >= 40) {
	  beforeLoc = (beforeLoc + (n-1)) % n;
	}
	ASSERT_EQ(Deck::find(a.cards,before),beforeLoc);
      }
    }
  }
}

TEST(Deck,Mix) {
  for (auto cfg : configs()) {
    retain<const DeckConfig> as(&cfg);
    for (auto n : {10, 40, 41, 52, 54}) {
      for (int len=1; len<10; ++len) {
	std::vector<Deck> decks;
	decks.push_back(Deck(n));
	std::vector<Card> path;
	for (int i=0; i<len; ++i) {
	  path.push_back(Card((i*31+17)%(n==10 ? 10 : 40)));
	  decks.push_back(decks[decks.size()-1]);
	  decks[decks.size()-1].mix(path[path.size()-1]);
	}

	for (int i=len-1; i>=0; --i) {
	  ASSERT_EQ(&Deck::config(),&cfg);
	
	  Deck deck(decks[i+1]);
	  deck.unmix(path[i]);
	  if (equivalent(decks[i],deck) != 0) {
	    Deck a(decks[i]);
	    Deck b(decks[i+1]);
	    Deck amix(a);
	    Deck bunmix(b);
	    amix.mix(path[i]);
	    bunmix.unmix(path[i]);
	    std::cout << "a=" << decks[i] << std::endl;
	    std::cout << "b=" << decks[i+1] << std::endl;
	    std::cout << "a.mix(" << path[i] << ")=" << amix << std::endl;
	    std::cout << "b.unmix(" << path[i] << ")=" << bunmix << std::endl;	  
	    ASSERT_EQ(decks[i],deck);
	  }

	}
      }
    }
  }
}

TEST(Deck,Unmix) {
  for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    Deck b(n);

    ASSERT_EQ(a,b);

    Card card1(1);
    b.mix(card1);
    b.unmix(card1);  
    ASSERT_EQ(a,b);

    a.mix(card1);
    b.mix(card1);

    Card card2(2);
    b.mix(card2);
    b.unmix(card2);
    ASSERT_EQ(a,b);
  }
}



TEST(Deck,CipherPad) {
  for (auto cfg : configs()) {
    retain<const DeckConfig> as(&cfg);
    for (auto n : {10, 40, 41, 52, 54}) {
      Deck a(n);
      const DeckConfig &cfg=a.config();
      shuffle(a);
    
      int zth = 0;
      while (a.cards[zth].order >= 40) {
	zth = (zth+1) % n;
      }
      for (int z=0; z<cfg.cipherZth; ++z) {
	zth = (zth+1) % n;
	while (a.cards[zth].order >= 40) {
	  zth = (zth+1) % n;
	}
      }
      ASSERT_EQ(Deck::forward(a.cards,0,cfg.cipherZth),zth);

      int cipherPadLoc;    

      if (cfg.cipherOffset >= 0) {
	int mark = (a.cards[zth].order + cfg.cipherOffset) % (n == 10 ? 10 : 40);
	int markLoc = -1;
	for (int i=0; i < n; ++i) {
	  if (a.cards[i].order == mark) {
	    ASSERT_EQ(markLoc,-1);
	    markLoc = i;
	  }
	}
	ASSERT_EQ(Deck::find(a.cards,Card(mark)),markLoc);

	cipherPadLoc = (markLoc + 1) % n;
	while (a.cards[cipherPadLoc].order >= 40) {
	  cipherPadLoc = (cipherPadLoc+1) % n;
	}
	ASSERT_EQ(Deck::forward(a.cards,markLoc,1),cipherPadLoc);
      } else {
	cipherPadLoc = zth;
      }

      ASSERT_EQ(a.cipherPad(),a.cards[cipherPadLoc]) << a;
    }
  }
}


TEST(Deck,CutPad) {
  for (auto cfg : configs()) {
    retain<const DeckConfig> as(&cfg);
    for (auto n : {10, 40, 41, 52, 54}) {
    Deck a(n);
    const DeckConfig &cfg=a.config();
    shuffle(a);
    
    int zth = 0;
    while (a.cards[zth].order >= 40) {
      zth = (zth+1) % n;
    }
    for (int z=0; z<cfg.cutZth; ++z) {
      zth = (zth+1) % n;
      while (a.cards[zth].order >= 40) {
	zth = (zth+1) % n;
      }
    }
    ASSERT_EQ(Deck::forward(a.cards,0,cfg.cutZth),zth);

    int cutPadLoc;

    if (cfg.cutOffset >= 0) {

      int mark = (a.cards[zth].order + cfg.cutOffset) % (n == 10 ? 10 : 40);
      int markLoc = -1;
      for (int i=0; i < n; ++i) {
	if (a.cards[i].order == mark) {
	  ASSERT_EQ(markLoc,-1);
	  markLoc = i;
	}
      }
      ASSERT_EQ(Deck::find(a.cards,Card(mark)),markLoc);
      
      cutPadLoc = (markLoc + 1) % n;
      while (a.cards[cutPadLoc].order >= 40) {
	cutPadLoc = (cutPadLoc+1) % n;
      }
      ASSERT_EQ(Deck::forward(a.cards,markLoc,1),cutPadLoc);
      
    } else {
      cutPadLoc = zth;
    }
      
    ASSERT_EQ(a.cutPad(),a.cards[cutPadLoc]) << a;
  }
}
}


TEST(Deck,PseudoShuffle10) {
  ps(10,0,"[9C(9),7C(7),5C(5),3C(3),AC(1),10C(0),2C(2),4C(4),6C(6),8C(8)]");
  ps(10,1,"[10C(0),8C(8),6C(6),4C(4),2C(2),AC(1),3C(3),5C(5),7C(7),9C(9)]");
  ps(10,2,"[AC(1),9C(9),7C(7),5C(5),3C(3),2C(2),4C(4),6C(6),8C(8),10C(0)]");
  ps(10,3,"[2C(2),10C(0),8C(8),6C(6),4C(4),3C(3),5C(5),7C(7),9C(9),AC(1)]");
  ps(10,4,"[3C(3),AC(1),9C(9),7C(7),5C(5),4C(4),6C(6),8C(8),10C(0),2C(2)]");
  ps(10,5,"[4C(4),2C(2),10C(0),8C(8),6C(6),5C(5),7C(7),9C(9),AC(1),3C(3)]");
  ps(10,6,"[5C(5),3C(3),AC(1),9C(9),7C(7),6C(6),8C(8),10C(0),2C(2),4C(4)]");
  ps(10,7,"[6C(6),4C(4),2C(2),10C(0),8C(8),7C(7),9C(9),AC(1),3C(3),5C(5)]");
  ps(10,8,"[7C(7),5C(5),3C(3),AC(1),9C(9),8C(8),10C(0),2C(2),4C(4),6C(6)]");
  ps(10,9,"[8C(8),6C(6),4C(4),2C(2),10C(0),9C(9),AC(1),3C(3),5C(5),7C(7)]");
}

TEST(Deck,PseudoShuffle40) {
  // int n = 40;
  // for (int i=0; i<40; ++i) {
  //   Deck deck(n);
  //   deck.pseudoShuffle(Card(i));
  //   std::cout << "ps(" << n << "," << i << ",\"" << deck << "\");" << std::endl;
  // }
  ps(40,0,"[9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38)]");
  ps(40,1,"[10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39)]");
  ps(40,2,"[AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0)]");
  ps(40,3,"[2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1)]");
  ps(40,4,"[3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2)]");
  ps(40,5,"[4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3)]");
  ps(40,6,"[5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4)]");
  ps(40,7,"[6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5)]");
  ps(40,8,"[7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6)]");
  ps(40,9,"[8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7)]");
  ps(40,10,"[9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8)]");
  ps(40,11,"[10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9)]");
  ps(40,12,"[AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10)]");
  ps(40,13,"[2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11)]");
  ps(40,14,"[3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12)]");
  ps(40,15,"[4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13)]");
  ps(40,16,"[5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14)]");
  ps(40,17,"[6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15)]");
  ps(40,18,"[7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16)]");
  ps(40,19,"[8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17)]");
  ps(40,20,"[9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18)]");
  ps(40,21,"[10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19)]");
  ps(40,22,"[AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20)]");
  ps(40,23,"[2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21)]");
  ps(40,24,"[3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22)]");
  ps(40,25,"[4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23)]");
  ps(40,26,"[5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24)]");
  ps(40,27,"[6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25)]");
  ps(40,28,"[7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),9H(29),8H(28),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26)]");
  ps(40,29,"[8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),10S(30),9H(29),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27)]");
  ps(40,30,"[9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),AS(31),10S(30),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28)]");
  ps(40,31,"[10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),2S(32),AS(31),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29)]");
  ps(40,32,"[AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),3S(33),2S(32),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30)]");
  ps(40,33,"[2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),4S(34),3S(33),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31)]");
  ps(40,34,"[3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),5S(35),4S(34),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32)]");
  ps(40,35,"[4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),6S(36),5S(35),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33)]");
  ps(40,36,"[5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),7S(37),6S(36),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34)]");
  ps(40,37,"[6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),8S(38),7S(37),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35)]");
  ps(40,38,"[7S(37),5S(35),3S(33),AS(31),9H(29),7H(27),5H(25),3H(23),AH(21),9D(19),7D(17),5D(15),3D(13),AD(11),9C(9),7C(7),5C(5),3C(3),AC(1),9S(39),8S(38),10C(0),2C(2),4C(4),6C(6),8C(8),10D(10),2D(12),4D(14),6D(16),8D(18),10H(20),2H(22),4H(24),6H(26),8H(28),10S(30),2S(32),4S(34),6S(36)]");
  ps(40,39,"[8S(38),6S(36),4S(34),2S(32),10S(30),8H(28),6H(26),4H(24),2H(22),10H(20),8D(18),6D(16),4D(14),2D(12),10D(10),8C(8),6C(6),4C(4),2C(2),10C(0),9S(39),AC(1),3C(3),5C(5),7C(7),9C(9),AD(11),3D(13),5D(15),7D(17),9D(19),AH(21),3H(23),5H(25),7H(27),9H(29),AS(31),3S(33),5S(35),7S(37)]");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
