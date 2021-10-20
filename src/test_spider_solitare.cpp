#include <math.h>
#include <wchar.h>
#include <iostream>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "spider_solitare.h"

const std::vector<std::wstring> TEST_STRINGS =
  {
   L"",
   L"x",
   L"xy",
   L"X",
   L"XY",
   L"1",
   L"12",
   L"♣",
   L"♣♠",
   L"xX1♣xyXY12♣♠",
   L"Spider Solitare",
   L"int main(int argc, char** argv) {\n" 
   L"  ::testing::InitGoogleTest(&argc, argv);\n" 
   L"  return RUN_ALL_TESTS();\n" 
   L"}\n",
   L"Q(0)A(1)23456789♣+0a0AQ♣0/0b1BA♣1/-39c2C2♣2/-38d3D3♣3/-37e4E4♣4/-36f5F5♣5/-35g6G6♣6/-34h7H7♣7/-33i8I8♣8/-32j9J9♣9/-31♦+10kAKQ♦10/-30lBLA♦11/-29mCM2♦12/-28nDN3♦13/-27oEO4♦14/-26pFP5♦15/-25q@Q6♦16/-24r=R7♦17/-23s\\S8♦18/-22t~T9♦19/-21♥+20u#UQ♥20/-20v$VA♥21/-19w%W2♥22/-18x^X3♥23/-17y&Y4♥24/-16z|Z5♥25/-15<-{6♥26/-14>+}7♥27/-13(/[8♥28/-12)*]9♥29/-11♠+30☐↩_Q♠30/-10,;:A♠31/-9.?!2♠32/-8\"\'`3♠33/-7👍😄❤️4♠34/-6👎😢💔5♠35/-5↓6♠36/-4↑7♠37/-3⇊8♠38/-2⇈9♠39/-1",
};

#define DECK_EQ(a,b) { for (int i=0; i<CARDS; ++i) { ASSERT_EQ(a[i],b[i]) << " index " << i; } }


struct CardNotRandIO {
  CardIO base;
  int state;
};

int CardNotRandIORead(CardIO *me) {
  CardNotRandIO *my=(CardNotRandIO *)me;
  int ans=0;
  if (my->state < PREFIX) {
    ans = my->state + 1; // 1--10 for prefix */
  } else {
    ans = CARDS-1-((my->state-PREFIX) % CARDS);
  }
  ++my->state;
  return ans;
}

void CardNotRandIOClose(CardIO *me) {}

void CardNotRandIOInit(CardNotRandIO *me) {
  me->base.read = &CardNotRandIORead;
  me->base.write = NULL;
  me->base.peek = NULL;
  me->base.close = &CardNotRandIOClose;
  me->state = 0;
}

TEST(Spider,Rand) {
  int n = 10*1000*1000;
  int counts1[CARDS];
  int counts2[CARDS][CARDS];
  CardRandIO rcg;
  CardRandIOInit(&rcg);

  for (int i=0; i<CARDS; ++i) {
    counts1[i]=0;
    for (int j=0; j<CARDS; ++j) {
      counts2[i][j]=0;
    }
  }

  int c1=rcg.base.read((CardIO*)&rcg);
  for (int i=0; i<n; ++i) {
    int c0=rcg.base.read((CardIO*)&rcg);
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

void testBackFrontShuffle(const Deck &in, Deck &out)
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

void testShuffle(const Deck &in, Deck &out) {
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

void testCut(const Deck &in, int cutPos, Deck &out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[(i+cutPos) % CARDS];
  }
}

int testFindCard(const Deck &deck, Card card) {
  for (int i=0; i<CARDS; ++i) {
    if (deck[i] == card) {
      return i;
    }
  }
  return -1;
}

Card testCutPad(const Deck &deck) {
  return deck[CUT_ZTH];
}

Card testCipherPad(const Deck &deck) {
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

TEST(Spider,FaceFromNo) {
  ASSERT_EQ(cardFaceFromNo(0),L'Q');
  ASSERT_EQ(cardFaceFromNo(1),L'A');
  ASSERT_EQ(cardFaceFromNo(2),L'2');
  ASSERT_EQ(cardFaceFromNo(3),L'3');
  ASSERT_EQ(cardFaceFromNo(4),L'4');
  ASSERT_EQ(cardFaceFromNo(5),L'5');
  ASSERT_EQ(cardFaceFromNo(6),L'6');
  ASSERT_EQ(cardFaceFromNo(7),L'7');
  ASSERT_EQ(cardFaceFromNo(8),L'8');
  ASSERT_EQ(cardFaceFromNo(9),L'9');
}

TEST(Spider,SuiteFromNo) {
  ASSERT_EQ(cardSuiteFromNo(0),0x2663); // Unicode club
  ASSERT_EQ(cardSuiteFromNo(1),0x2665); // Unicdoe diamond
  ASSERT_EQ(cardSuiteFromNo(2),0x2667); // Unicode heart
  ASSERT_EQ(cardSuiteFromNo(3),0x2669); // Unicode spade
}

TEST(Spider,CardFromFaceSuiteNo) {
  for (int suiteNo : {-1,0, 1, 2, 3,4}) {
    for (int faceNo : {-1,0,1,2,3,4,5,6,7,8,9,10}) {
      int card = 10*suiteNo+faceNo;
      if (suiteNo < 0 || suiteNo >= 4) { card = -1; }
      if (faceNo < 0 || faceNo >= 10) { card = -1; }      
      ASSERT_EQ(cardFromFaceSuiteNo(faceNo,suiteNo),card);
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

TEST(Spider,Init) {
  Deck deck;
  deckInit(deck);
  for (int i=0; i<CARDS; ++i) {
    ASSERT_EQ(deck[i],i);
  }
}

TEST(Spider,Cut) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,expect;
    deckInit(input);
    deckInit(output);
    deckInit(expect);
    testCut(input,cutLoc,expect);
    deckCut(input,cutLoc,output);
    DECK_EQ(output,expect);
  }
}

TEST(Spider,BackFrontShuffle) {
  Deck input,output,expect;
  deckInit(input);
  deckInit(output);
  deckInit(expect);
  testBackFrontShuffle(input,expect);
  deckBackFrontShuffle(input,output);

  for (int i=0; i<CARDS; ++i) {    
    ASSERT_EQ(output[i],expect[i]);
  }
}

TEST(Spider,FindCard) {
  Deck deck,shuffled;
  deckInit(deck);
  deckInit(shuffled);
  testShuffle(deck,shuffled);
  for (int i=0; i<CARDS; ++i) {
    ASSERT_EQ(testFindCard(deck,i),deckFindCard(deck,i));
    ASSERT_EQ(testFindCard(shuffled,i),deckFindCard(shuffled,i));    
  }
}

TEST(Spider,PseudoShuffle) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,tmp,expect;
    deckInit(input);
    deckInit(output);
    deckInit(tmp);
    deckInit(expect);
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
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);
  
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
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);

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
  for (int i=0; i<TEST_STRINGS.size(); ++i) {
    const wchar_t *str=TEST_STRINGS[i].c_str();
    int strLen = TEST_STRINGS[i].length();

    int cardsLen=encodeLen((wchar_t*)str,strLen);
    Card *cards=(Card*)malloc(sizeof(Card)*cardsLen);
    encodeArray((wchar_t*)str,strLen,cards,cardsLen);

    int decLen=decodeLen(cards,cardsLen);
    ASSERT_GE(decLen,0) << " i=" << i;

    wchar_t *decode = (wchar_t*)malloc(sizeof(wchar_t)*(decLen+1));
    decodeArray(cards,cardsLen,decode,decLen);
    decode[decLen]=0;

    int maxLen = strLen > decLen ? strLen : decLen;
    for (int j=0; j<maxLen; ++j) {
      ASSERT_EQ(j < strLen ? str[j] : -1,j < decLen ? decode[j] : -1) << " i=" << i << " j=" << j;
    }

    ASSERT_EQ(strLen,decLen) << " i=" << i;
  }
}

TEST(Spider,Envelope) {
  for (int i=0; i<TEST_STRINGS.size(); ++i) {
    const wchar_t *str=TEST_STRINGS[i].c_str();
    int strLen = TEST_STRINGS[i].length();

    CardNotRandIO nrcg;
    CardNotRandIOInit(&nrcg);
    Deck deck;
    deckInit(deck);
    
    int maxCardLen = strLen*6+PREFIX;
    std::vector<Card> cards(maxCardLen,0);

    int cardLen=encryptEnvelopeArray(deck,(wchar_t*)str,strLen,
				     (CardIO*)&nrcg,
				     &cards[0],maxCardLen);
    ASSERT_TRUE(cardLen > 0) << " test str # " << i;

    int decStrCap=strLen*6+PREFIX;
    std::vector<wchar_t> decStr(decStrCap,0);
    deckInit(deck);
    int decStrLen=decryptEnvelopeArray(deck,&cards[0],cardLen,&decStr[0],decStrCap);

    ASSERT_EQ(decStrLen,strLen);
    for (int i=0; i<strLen; ++i) {
      ASSERT_EQ(decStr[i],str[i]);
    }
  }
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
 ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


