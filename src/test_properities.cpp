#include <iostream>
#include <sstream>
#include <iomanip>
#include "gtest/gtest.h"
#include "deck.h"
#include "search.h"

using namespace std;
using namespace spider;


void print(const Deck &deck,
	   std::ostream &out=std::cout,
	   bool delta=true,
	   int width=10) {
  int n = deck.cards.size();
  for (int i=0; i<n; ++i) {
    int j = deck.cards[i].order-(delta ? i : 0);
    if (i % width == 0) out << " ";
    out << " " << std::setw(2)
	      << i << (delta ? "+= " : " = " )
	      << std::setw(3) << j;
    if (i+1 < n) out << ",";
    if ((i+1) % width == 0) {
      out << std::endl;
    }
  }
}

void T(Deck &deck,int t=1)  {
  t = ((t%40)+40)  % 40;
  std::vector<Card> tmp(deck.cards.size());
  Deck::cut(deck.cards,t,tmp);
  deck.cards.swap(tmp);
}

void P(Deck &deck,int p=1) {
  p = ((p%27)+27) % 27;
  for (int  i=0; i<p; ++i) {
    deck.pseudoShuffle(deck.cards[0]);
  }
}

void R(Deck &deck, int p=1) {
  int n = deck.cards.size();
  p = ((p%2)+2)%2;
  for (int k=0; k<p; ++k) {  
    for (int  i=0; i<n/2; ++i) {
      Card tmp = deck.cards[i];
      deck.cards[i]=deck.cards[n-1-i];
      deck.cards[n-1-i]=tmp;
    }
  }
}

void S(Deck &deck, int p=1) {
  int n = deck.cards.size();
  p = ((p%2)+2)%2;
  for (int k=0; k<p; ++k) {
    Card tmp = deck.cards[0];
    deck.cards[0]=deck.cards[n-1];
    deck.cards[n-1]=tmp;
  }
}


void X(Deck &deck,int p=1) {
  int n = deck.cards.size();
  p = ((p%2)+2)%2;
  for (int k=0; k<p; ++k) {
    for (int  i=0; i<n; i += 2) {
      Card tmp = deck.cards[i];
      deck.cards[i]=deck.cards[i+1];
      deck.cards[i+1]=tmp;
    }
  }
}

void Y(Deck &deck,int p=1) { // CDHS -> DCSH (switch suite pairs)
  int n = deck.cards.size();
  p = ((p%2)+2)%2;
  for (int k=0; k<p; ++k) {
    for (int  i=0; i<n; ++i) {
      int suite = i/10;
      int face = i % 10;
      int newSuite = (suite % 2 == 0) ? suite+1 : suite-1;
      
      int j=newSuite*10+face;
      
      if (newSuite < suite) {
	Card tmp = deck.cards[i];
	deck.cards[i]=deck.cards[j];
	deck.cards[j]=tmp;
      }
    }
  }
}

void Z(Deck &deck,int p=1) { // CDHS -> CSHD(switch D(+10) with spades (+30))
  int n = deck.cards.size();
  p = ((p%2)+2)%2;
  for (int k=0; k<p; ++k) {
    for (int  i=0; i<n; ++i) {
      int suite = i/10;
      int face = i % 10;
      if (suite == 1) {
	int newSuite = 3;
	int j=newSuite*10+face;

	Card tmp = deck.cards[i];
	deck.cards[i]=deck.cards[j];
	deck.cards[j]=tmp;
      }
    }
  }
}


void xformStream(Deck &deck, std::istringstream &in) {
  int p=1;
  char t;
  if (!(in >> std::skipws >> t)) {
    return;
  }
  if (in.peek() == '^') {
    char z;
    in >> z;
    in >> p;
  }
  xformStream(deck,in);
  switch(t) {
  case 'P': P(deck,p); break;
  case 'T': T(deck,p); break;
  case 'S': S(deck,p); break;    
  case 'R': R(deck,p); break;
  case 'X': X(deck,p); break;
  case 'Y': Y(deck,p); break;
  case 'Z': Z(deck,p); break;        
  default: fprintf(stderr,"unkown op"); assert(false); break;
  }
}

Deck xform(const std::string &xs, const Deck &deck=Deck(40)) {
  Deck copy(deck);
  std::istringstream iss(xs);
  xformStream(copy,iss);
  return copy;
}

Deck p(const std::string x, std::ostream &out=std::cout, bool delta=true) {
  std::cout << x << ": " << std::endl;

  Deck res(40);
  Deck output(xform(x,res));
  print(output,out,delta);
  return res;
}

TEST(Properties,Cycle) {
  int n=40;
  Deck deck(n);
  for (int i=0; i<9; ++i) {
    deck.pseudoShuffle(deck.cards[2]);
  }
  ASSERT_EQ(deck,Deck(n));
}

TEST(Properties,Translations) {
  int n=40;
  for (int t=0; t<40; ++t) {
    Deck deck(n);
    Deck translated(n);
    Deck::cut(deck.cards,t,translated.cards);
    for (int i=0; i<9; ++i) {
      deck.pseudoShuffle((i == 0) ? deck.cards[(2+t)%n] : deck.cards[2]);
    }
    ASSERT_EQ(deck,translated);
  }
}

TEST(Properties,BackFrontCycle40is27) {
  int n=40;
  int c=27;
  Deck id(n);
  Deck deck(n);
  Deck tmp(n);
  for (int i=0; i<c; ++i) {
    Deck::backFrontShuffle(deck.cards,tmp.cards);
    deck.cards.swap(tmp.cards);
    if (i != c-1) {
      ASSERT_FALSE(id==deck);
    } else {
      ASSERT_EQ(id,deck);
    }
  }
}

TEST(Properties,BackFrontShuffle) {
  int n = 40;
  Deck deck(n);
  Deck backFrontShuffle(n);
  Deck::backFrontShuffle(deck.cards,backFrontShuffle.cards);
  deck.pseudoShuffle(deck.cards[0]);
  ASSERT_EQ(deck,backFrontShuffle);
}

TEST(Properties,InverseBackFrontShuffle) {
  int n = 40;
  Deck deck(n);
  Deck backFrontShuffle(n);
  Deck::backFrontShuffle(deck.cards,backFrontShuffle.cards);

  for (int i=0; i<17; ++i) {
    int t=2;
    Card cutCard = backFrontShuffle.cards[(i==8)?(2+t)%n:2];    
    backFrontShuffle.pseudoShuffle(cutCard);
  }

  ASSERT_EQ(deck,backFrontShuffle);
}


TEST(Properties,Reverse) {
  int n = 40;
  Deck deck(n);
  Deck deck2(n);  
  Deck reversed(n);
  for (int i=0; i<n; ++i) {
    reversed.cards[i]=(n-1)-i;
  }

  P(deck,1);
  T(deck,20);
  P(deck,-1);

  ASSERT_EQ(deck,reversed);
  ASSERT_EQ(xform("P^-1 T^20 P"),reversed);
  ASSERT_EQ(xform("R"),reversed);  
}

TEST(Properties,ExchangePairs) {
  int n = 40;
  Deck deck(n);
  Deck exchanged(n);
  for (int i=0; i<n; i += 2) {
    exchanged.cards[i]=Card(i+1);
    exchanged.cards[i+1]=Card(i);
  }

  P(deck,1);
  R(deck);
  P(deck,-1);

  ASSERT_EQ(deck,exchanged);
  ASSERT_EQ(xform("P^-1 R P^1"), exchanged);
  ASSERT_EQ(xform("X"), exchanged);  
}

TEST(Properties,InversePseudoShuffle) {
  int n = 40;
  for (int c=0; c<n; ++c) {
    Deck deck(n);
    Deck ps(n);
    ps.pseudoShuffle(ps.cards[c]);

    for (int i=0; i<17; ++i) {
      int t=(2+(n-c))%40;
      Card cutCard = ps.cards[(i==8)?(2+t)%n:2];    
      ps.pseudoShuffle(cutCard);
    }
    ASSERT_EQ(deck,ps);

    if (c == 2) { // two ways to make (PT^2)^(-1)
      deck = Deck(n);
      ps = Deck(n);
      Deck psi(n);
      ps.pseudoShuffle(ps.cards[c]);      
      for (int i=0; i<8; ++i) {
	ps.pseudoShuffle(ps.cards[2]);
	psi.pseudoShuffle(psi.cards[2]);
      }
      ASSERT_EQ(deck,ps);
    }
  }
}

TEST(Properties,ExchangeSuitePairs) {
  // CDHS -> DCSH
  int n = 40;
  Deck deck(n);
  Deck exchanged(n);
  for (int i=0; i<n; ++i) {
    int suite = i/10;
    int face = i % 10;
    int newSuite = (suite % 2 == 0) ? suite+1 : suite-1;
    exchanged.cards[i]=newSuite*10+face;
  }

  P(deck,-1);
  T(deck,20);
  P(deck,1);

  ASSERT_EQ(deck,exchanged);
  ASSERT_EQ(xform("P T^20 P^-1"),exchanged);
  ASSERT_EQ(xform("Y"),exchanged);  
}

TEST(Properties,SwapFirstLast) {
  int n = 40;
  Deck deck(n);
  Deck swapped(n);
  swapped.cards[0]=39;
  swapped.cards[39]=0;
  
  P(deck);
  T(deck,21);
  P(deck,-1);
  R(deck);
  X(deck);
  T(deck);
  X(deck);
  T(deck,-1);
  
  ASSERT_EQ(deck,swapped);
  ASSERT_EQ(xform("T^-1 X T X R P^-1 T^21 P"),swapped);
  ASSERT_EQ(xform("S"),swapped);  
}

//
// since bubble sort only uses neighbouring pair swaps,
// this is sufficient to guarantee any permutation is reachable.  QED!
//
TEST(Properties,Bubble) {
  int n=40;
  for (int k=0; k<39; ++k) {
    Deck deck(n);
    Deck swapped(n);
    swapped.cards[k]=k+1;
    swapped.cards[k+1]=k;
    
    T(deck,k+1);
    S(deck);
    T(deck,-(k+1));

    ASSERT_EQ(deck,swapped);
  }
}

TEST(Properties,ExchangeDiamondsSpades) {
  // CDHS -> DCSH
  int n = 40;
  Deck deck(n);
  Deck exchanged(n);
  for (int i=0; i<n; ++i) {
    int suite = i/10;
    int face = i % 10;
    int newSuite = suite;
    if (suite == 1) newSuite += 2;
    if (suite == 3) newSuite -= 2;
    exchanged.cards[i]=newSuite*10+face;
  }

  Y(deck,1);
  T(deck,10);

  ASSERT_EQ(deck,exchanged);
  ASSERT_EQ(xform("T^10 Y"),exchanged);
  ASSERT_EQ(xform("Z"),exchanged);  
}

int main(int argc, char** argv) {
  if (argc > 1 && argv[1][0] != '-') {
    for (int i=1; i<argc; ++i) {
      p(argv[i]);
    }
  } else {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
}
