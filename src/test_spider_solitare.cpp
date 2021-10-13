#include <math.h>
#include <wchar.h>
#include "gtest/gtest.h"
#include "spider_solitare.h"

void testBackFrontShuffle(Deck in, Deck out)
{
  int n=0;
  for (int i=0; i<CARDS; ++i) {
    int j=(i%2 == 0) ? n : 0;
    for (int k=n; k>j; --k) {
      out[k]=out[k-1];
    }
    out[j]=in[i];
    ++n;
  }
}

void testShuffle(Deck in, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[i];
  }
  for (int i=0; i<CARDS; ++i) {
    int n=(CARDS-i);
    int r = (17*n+3) % n;
    int j = i + r;
    int tmp=out[i];
    out[i]=out[j];
    out[j]=tmp;
  }
}

void testCut(Deck in, int cutPos, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[(i+cutPos) % CARDS];
  }
}

int testFindCard(Deck deck, Card card) {
  for (int i=0; i<CARDS; ++i) {
    if (deck[i] == card) {
      return i;
    }
  }
  return -1;
}

Card testCutPad(Deck deck) {
  return deck[CUT_ZTH];
}

Card testCipherPad(Deck deck) {
  int markCard = (deck[MARK_ZTH]+MARK_ADD) % CARDS;
  int markLoc = testFindCard(deck, markCard);
  return deck[(markLoc + 1) % CARDS];
}


TEST(Spider,FaceAndSuiteNo) {
  for (int suiteNo : {0, 1, 2, 3}) {
    for (int faceNo : {0,1,2,3,4,5,6,7,8,9}) {
      Card card = 10*suiteNo+faceNo;
      ASSERT_EQ(cardFaceNo(card),faceNo);
      ASSERT_EQ(cardSuiteNo(card),suiteNo);
    }
  }
}

TEST(Spider,Add) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x + y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result =cardAdd(x,y);
      ASSERT_EQ(z,result) << " x=" << x << " y=" << y;
    }
  }
}

TEST(Spider,Subtract) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x - y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result = cardSubtract(x,y);
      ASSERT_EQ(z,result) << " x=" << x << " y=" << y;
    }
  }
}

TEST(Spider,Cut) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,expect;
    for (int i=0; i<CARDS; ++i) {
      input[i]=i;
      expect[i]=cardAdd(i,cutLoc);
    }
    deckCut(input,cutLoc,output);
    for (int i=0; i<CARDS; ++i) {    
      ASSERT_EQ(output[i],expect[i]);
    }
  }
}

TEST(Spider,BackFrontShuffle) {
  Deck input,output,expect;
  for (int i=0; i<CARDS; ++i) {
    input[i]=i;
  }
  testBackFrontShuffle(input,expect);
  deckBackFrontShuffle(input,output);

  for (int i=0; i<CARDS; ++i) {    
    ASSERT_EQ(output[i],expect[i]);
  }
}

TEST(Spider,FindCard) {
  Deck deck,shuffled;
  for (int i=0; i<CARDS; ++i) {
    deck[i]=i;
  }
  testShuffle(deck,shuffled);
  for (int i=0; i<CARDS; ++i) {
    ASSERT_EQ(testFindCard(deck,i),deckFindCard(deck,i));
    ASSERT_EQ(testFindCard(shuffled,i),deckFindCard(shuffled,i));    
  }
}

TEST(Spider,PseudoShuffle) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,tmp,expect;
    for (int i=0; i<CARDS; ++i) {
      input[i]=i;
      output[i]=i;
    }
    testCut(input,cutLoc,tmp);
    testBackFrontShuffle(tmp,expect);
    deckPseudoShuffle(output,cutLoc);
    
    for (int i=0; i<CARDS; ++i) {    
      ASSERT_EQ(output[i],expect[i]);
    }
  }
}

TEST(Spider,Pads) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  for (int i=0; i<CARDS; ++i) {
    deck[i]=i;
    testDeck[i]=i;
  }
  
  for (int i=0; testString[i] != 0; ++i) {
    for (int j=0; j<CARDS; ++j) {
      ASSERT_EQ(testDeck[j],deck[j]);
    }
    ASSERT_EQ(testCutPad(testDeck),deckCutPad(deck));
    ASSERT_EQ(testCipherPad(testDeck),deckCipherPad(deck));
    Card plain = testString[i]-'a';
    Card cutCard = cardAdd(plain,deckCutPad(deck));
    int cutLoc = deckFindCard(deck,cutCard);
    
    testCut(testDeck,cutLoc,tmp);
    testBackFrontShuffle(tmp,testDeck);
    deckPseudoShuffle(deck,cutLoc);
  }
}

TEST(Spider,Ciphers) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  for (int i=0; i<CARDS; ++i) {
    deck[i]=i;
    testDeck[i]=i;    
  }

  for (int k=0; k<10; ++k) {
    for (int i=0; testString[i] != 0; ++i) {
      for (int j=0; j<CARDS; ++j) {
	ASSERT_EQ(testDeck[j],deck[j]);
      }
      for (int j=0; j<CARDS; ++j) {
	tmp[j]=deck[j];
      }
      Card plainCard = testString[i]-'a';
      Card cipherCard = cardAdd(plainCard,testCipherPad(testDeck));
      Card cutCard = cardAdd(plainCard,testCutPad(testDeck));
      int cutLoc = testFindCard(testDeck,cutCard);
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      ASSERT_EQ(testCipherPad(testDeck),deckCipherPad(deck));
      ASSERT_EQ(testCutPad(testDeck),deckCutPad(deck));      
      int cipher2 = deckEncryptCard(deck,plainCard);
      ASSERT_EQ(cipherCard,cipher2);
      
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      ASSERT_EQ(testCipherPad(testDeck),deckCipherPad(deck));
      ASSERT_EQ(testCutPad(testDeck),deckCutPad(deck));      
      int plain2 = deckDecryptCard(deck,cipherCard);
      ASSERT_EQ(plainCard,plain2) << " plain=" << ((unsigned)plainCard) << " cipher pad=" << ((unsigned)testCipherPad(testDeck)) << " k=" << k << " i=" << i;

      deckPseudoShuffle(testDeck,cutLoc);
      for (int j=0; j<CARDS; ++j) {
	ASSERT_EQ(testDeck[j],deck[j]);
      }
    }
  }
}

TEST(Spider,Encode) {
  const wchar_t *str0 =
    L"int main(int argc, char** argv) {\n" 
    L"  ::testing::InitGoogleTest(&argc, argv);\n" 
    L"  return RUN_ALL_TESTS();\n" 
    L"}\n";

  const wchar_t *str1 =
    L"Q(0)A(1)23456789♣+0a0AQ♣0/0b1BA♣1/-39c2C2♣2/-38d3D3♣3/-37e4E4♣4/-36f5F5♣5/-35g6G6♣6/-34h7H7♣7/-33i8I8♣8/-32j9J9♣9/-31♦+10kAKQ♦10/-30lBLA♦11/-29mCM2♦12/-28nDN3♦13/-27oEO4♦14/-26pFP5♦15/-25q@Q6♦16/-24r=R7♦17/-23s\\S8♦18/-22t~T9♦19/-21♥+20u#UQ♥20/-20v$VA♥21/-19w%W2♥22/-18x^X3♥23/-17y&Y4♥24/-16z|Z5♥25/-15<-{6♥26/-14>+}7♥27/-13(/[8♥28/-12)*]9♥29/-11♠+30☐↩_Q♠30/-10,;:A♠31/-9.?!2♠32/-8\"\'`3♠33/-7👍😄❤️4♠34/-6👎😢💔5♠35/-5↓6♠36/-4↑7♠37/-3⇊8♠38/-2⇈9♠39/-1";

  for (int i=0; i<2; ++i) {
    const wchar_t *str = (i == 0) ? str0 : str1;
    int strLen = 0; while (str[strLen] != 0) ++strLen;

    int cardsLen=cardEncodeLen(str,strLen);
    Card *cards=(Card*)malloc(sizeof(Card)*cardsLen);
    cardEncodeToArray(str,strLen,cards,cardsLen);

    int decodeLen=cardDecodeLen(cards,1,cardsLen);
    wchar_t *decode = (wchar_t*)malloc(sizeof(wchar_t)*(decodeLen+1));
    cardDecodeToArray(cards,1,cardsLen,decode,decodeLen);
    decode[decodeLen]=0;

    for (int j=0; j<=decodeLen; ++j) {
      ASSERT_EQ(str[j],decode[j]) << " i=" << i << " j=" << j;
    }

    setlocale(LC_CTYPE,"UTF-8");
    fputws(str,stdout);
    printf(" => ");
    fputws(decode,stdout);  
  }

}

struct TestRandParms {
  int state;
};

int testRand(void *voidParms) {
  TestRandParms *parms = (TestRandParms *) voidParms;
  ++parms->state;
    
  if (parms->state <= PREFIX) {
    return parms->state % CARDS;
  } else {
    int i = (parms->state-1-PREFIX) % CARDS;
    return CARDS-1-i;
  }
}

TEST(Spider,Rand) {
  int n = 100000000;
  int counts1[CARDS];
  int counts2[CARDS][CARDS];
  void *randParms = RandOpen();

  for (int i=0; i<CARDS; ++i) {
    counts1[i]=0;
    for (int j=0; j<CARDS; ++j) {
      counts2[i][j]=0;
    }
  }

  int c1=RandCard(randParms);
  for (int i=0; i<n; ++i) {
    int c0=RandCard(randParms);
    ++counts1[c0];
    ++counts2[c0][c1];
    c1=c0;
  }

  double p1 = 1.0/CARDS;
  double q1 = 1-p1;
  double mu1 = n*p1;
  double inv_sigma1 = 1/sqrt(mu1);
  double z1=0.0;

  for (int i=0; i<CARDS; ++i) {
    double zi=inv_sigma1*(counts1[i]-mu1);
    ASSERT_LE(fabs(zi),6.0) << " i=" << i << std::endl;
    z1 += pow(zi,2);
  }

  z1 = sqrt(2.0)*(sqrt(z1)-sqrt(CARDS-1.5));

  double p2 = pow(1.0/CARDS,2);
  double q2 = 1-p2;
  double mu2 = n*p2;
  double inv_sigma2 = 1/sqrt(mu2);
  double z2=0.0;

  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      double zij=inv_sigma2*(counts2[i][j]-mu2);
      ASSERT_LE(fabs(zij),6.0) << " i=" << i << " j=" << j << std::endl;
      z2 += pow(zij,2);
    }
  }

  z2 = sqrt(2.0)*(sqrt(z2)-sqrt(pow(CARDS,2)-1.5));

  ASSERT_LE(fabs(z1),6.0);
  ASSERT_LE(fabs(z2),6.0);  
}


TEST(Spider,HelloWorld) {
  const wchar_t *str=L"I❤️Spider - Solitaire is #1!";
  TestRandParms randParms;
  randParms.state = 0;
  int strLen = 0;
  while (str[strLen] != 0) ++strLen;
  Deck deck;
  int maxCardLen = 1000;
  Card cards[1000];

  for (int i=0; i<CARDS; ++i) {
    deck[i]=i;
  }
  int cardLen=deckEncryptEnvelopeToArray(deck,str,strLen,
					 testRand,&randParms,cards,maxCardLen);
  ASSERT_TRUE(cardLen > 0);

  for (int i=0; i<cardLen; ++i) {
    printf(" %02d",cards[i]);
    if (i % 10 == 9) printf("\n");
  }
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
 ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


