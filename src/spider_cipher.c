#define N 40

#define CUT_ZTH 2
#define CIPHER_ZTH 0
#define CIPHER_OFFSET 39

typedef unsigned char Card;
typedef Card Deck[N];

int cardFaceNo(Card card) { return card%10; }

int cardSuiteNo(Card card) { return card/10; }

char cardFaceFromNo(int cardFaceNo) {
  return "QA23456789"[cardFaceNo];
}

char cardSuiteFromNo(int cardSuiteNo) {
  return "CDHS"[cardSuiteNo];
}

Card cardAdd(Card x, Card y) {
  return (x+y)%N;
}

Card cardSubtract(Card x, Card y) {
  return (x+(N-y))%N;
}

void deckCut(Deck in, int cutLoc,Deck out) {
  for (int i=0; i<N; ++i) {
    out[i]=in[(i+cutLoc) % N];
  }
}

void deckBackFrontShuffle(Deck in, Deck out) {
  int back = N/2;
  int front = N/2-1;
  for (int i=0; i<N; i += 2) {
    out[back]=in[i];
    ++back;
    out[front]=in[i+1];
    --front;
  }
}

int deckFindCard(Deck deck,Card card) {
  // fixed runtime find
  static int mask[]={0,~0};
  int at = 0;
  for (int i=0; i<N; ++i) {
    at |= (mask[deck[i] == card] & i);
  }
  return at;
}

void deckPseudoShuffle(Deck deck, int cutLoc) {
  Deck temp;
  deckCut(deck,cutLoc,temp);
  deckBackFrontShuffle(temp,deck);
}

Card deckCutPad(Deck deck) {
  Card padCard=deck[CUT_ZTH];
  return padCard;
}

Card deckCipherPad(Deck deck) {
  Card markCard = (deck[CIPHER_ZTH]+CIPHER_OFFSET) % N;
  int markLoc = deckFindCard(deck,markCard);
  int padLoc = (markLoc+1)%N;
  Card padCard = deck[padLoc];
  return padCard;
}

Card deckTranslate(Deck deck, Card inputCard, int mode) {
  Card plainCard,cipherCard,outputCard;
  Card cipherPad = deckCipherPad(deck);

  if (mode == 1) {
    plainCard=inputCard;
    cipherCard=cardAdd(inputCard,cipherPad);
    outputCard=cipherCard;
  } else if (mode == -1) {
    cipherCard=inputCard;
    plainCard=cardSubtract(inputCard,cipherPad);
    outputCard=plainCard;
  } else {
    assert(0);
  }

  Card cutPad=deckCutPad(deck);
  Card cutCard=cardAdd(plainCard,cutPad);
  int cutLoc = deckFindCard(deck,cutCard);
  deckPseudoShuffle(deck,cutLoc);
  return outputCard;
}

Card deckEncryptCard(Deck deck, Card plainCard) {
  Card cipherCard=deckTranslateCard(deck,plainCard,1);
  return cipherCard;
}

Card deckDecryptCard(Deck deck, Card cipherCard) {
  Card plainCard=deckTranslateCard(deck,cipherCard,-1);
  return plainCard;
}

const wchar_t *DOWN_CODES = L"0123456789ABCDEF@=\\~#$%^&|-+/*\n;?\'\U0001F622\U0001F604";
const wchar_t *     CODES = L"abcdefghijklmnopqrstuvwxyz<>() ,.\"\U0001F44E\U0001F44D";
const wchar_t *  UP_CODES = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ{}[]_:!`\U0001F494\u2764";
const wchar_t *ALL_CODES[] = { DOWN_CODES, CODES, UP_CODES };

int encode(const wchar_t *utf8, int len, CardWrite *write, void *writeMisc) {
  int count = 0;
  int shift = 1;
  int next = 1;
  wchar_t buf[5];
  int bufLen = 0;
  
  while (bufLen + len > 0) {
    wchar_t c = (bufLen > 0) ? buffer[bufLen-1] : utf8[0];
    int p = wstrchr(ALL_CODES[shift],c);
    if (p >= 0) {
      ++count; if (write) write(p,writeMisc);
      shift = next;
      if (bufLen > 0) {
	--bufLen;
      } else {
	++utf8;
	--len;
      }
      continue;
    }
    
    int en[3]={0,0,0},m=-1;
    for (int query=0; query<3; ++query) {
      while (en[query] < len && wstrchr(ALL_CODES[query],utf8[en[query]]) >= 0) ++en[query];
      if (m == -1 || en[m] < en[query]) { m = query; }
    }
    int lock = (en[m] != 1 || (en[m]==1 && (len > 1 && wstrchr(ALL_CODES[next],utf8[1]) < 0)));
    while (shift != m) {
      if (shift < m) {
	++shift;
	++count; if (write) write(lock ? SHIFT_LOCK_UP : SHIFT_UP,writeMisc);
      }
      if (m < shift) {
	--shift;
	++count; if (write) write(lock ? SHIFT_LOCK_DOWN : SHIFT_DOWN,writeMisc);
      }      
    }
    if (lock) { next = shift; }

    if (en[m]==0) {
      unsigned c=utf8[0];
      if (c <= 0xFF) {
	bufLen = 4;
	buf[bufLen-1]='\\'
	buf[bufLen-2]='x'
      } else if (c <= 0xFFFF) {
	bufLen = 6;	
	buf[bufLen-1]='\\'
	buf[bufLen-2]='u'
      } else {
	bufLen = 10;
	buf[bufLen-1]='\\'
	buf[bufLen-2]='U'
      }
      for (int i=0; i<bufLen-2; ++i) {
	buf[bufLen-3-i]=DOWN_CODES[(c >> (i*4))&0xF];
      }
    }
  }
  return count;
}




