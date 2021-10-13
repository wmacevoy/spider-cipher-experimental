#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "spider_solitare.h"

#define ADD(x,y) (((x)+(y))%CARDS)
#define SUB(x,y) (((x)+(CARDS-(y)))%CARDS)

static const unsigned MASK[] = {0U,~0U};

#define CUT_PAD(deck) deck[CUT_ZTH]
#define CUT_CARD(deck,plain) ADD(CUT_PAD(deck),plain)

#define MARK_CARD(deck) ADD(deck[MARK_ZTH],MARK_ADD)

#if (1 == 1) == 1
#define FIND1(deck,offset,card) \
  ((offset) & MASK[(deck)[offset]==(card)])
#elif (1 == 1) == ~0
#define FIND1(deck,offset,card) \
  ((offset) & (deck)[offset]==(card))
#else
#error C compiler defines true as neither 1 nor ~0
#endif

#define FIND5(deck,offset,card)				\
  FIND1(deck,offset+0,card) |				\
    FIND1(deck,offset+1,card) |				\
    FIND1(deck,offset+2,card) |				\
    FIND1(deck,offset+3,card) |				\
    FIND1(deck,offset+4,card)
  
#define FIND10(deck,offset,card)	\
  FIND5(deck,offset,card)|FIND5((deck),offset+5,card)

#define FIND20(deck,offset,card)			\
  FIND10(deck,offset,card)|FIND10(deck,offset+10,card)

#define FIND(deck,card)				\
  (FIND20(deck,0,card)|FIND20(deck,20,card))

int cardFaceNo(Card card)
{
  return card%10;
}

int cardSuiteNo(Card card)
{
  return card/10;
}

wchar_t cardFaceFromNo(int cardFaceNo)
{
  return L"QA23456789"[cardFaceNo];
}

wchar_t cardSuiteFromNo(int cardSuiteNo)
{
  return L"\u2663\u2665\u2667\u2669"[cardSuiteNo];
}

Card cardAdd(Card x, Card y)
{
  return ADD(x,y);
}

Card cardSubtract(Card x, Card y)
{
  return SUB(x,y);
}

void deckCut(Deck input, int cutLoc,Deck output)
{
  for (int i=0; i<CARDS; ++i) {
    output[i]=input[(i+cutLoc) % CARDS];
  }
}

void deckBackFrontShuffle(Deck input, Deck output)
{
  int back = CARDS/2;
  int front = CARDS/2-1;
  for (int i=0; i<CARDS; i += 2) {
    output[back]=input[i];
    output[front]=input[i+1];
    ++back;
    --front;
  }
}

int deckFindCard(Deck deck,Card card)
{
  return FIND(deck,card);
}

void deckPseudoShuffle(Deck deck, int cutLoc) {
  Deck temp;
  deckCut(deck,cutLoc,temp);
  deckBackFrontShuffle(temp,deck);
}

Card deckCutPad(Deck deck) {
  Card cutPad=deck[CUT_ZTH];
  return cutPad;
}

Card deckCipherPad(Deck deck) {
  Card markCard = MARK_CARD(deck);
  int markLoc = deckFindCard(deck,markCard);
  Card cipherPad = deck[ADD(markLoc,1)];
  return cipherPad;
}

void deckAdvance(Deck deck, Card inputCard, Card *outputCard, int mode) {
  Card plainCard,cipherCard;

  if (mode == 1) {
    plainCard=inputCard;
    if (outputCard != NULL) {
      Card cipherPad = deckCipherPad(deck);
      cipherCard=cardAdd(plainCard,cipherPad);
      *outputCard=cipherCard;
    }
  } else if (mode == -1) {
    cipherCard=inputCard;
    Card cipherPad = deckCipherPad(deck);    
    plainCard=cardSubtract(cipherCard,cipherPad);
    if (outputCard != NULL) {
      *outputCard=plainCard;
    }
  } else {
    assert(0);
  }

  Card cutPad = deckCutPad(deck);
  Card cutCard = cardAdd(plainCard,cutPad);
  int cutLoc = deckFindCard(deck,cutCard);
  deckPseudoShuffle(deck,cutLoc);
}

Card deckEncryptCard(Deck deck, Card plainCard) {
  Card cipherCard=~0;
  deckAdvance(deck,plainCard,&cipherCard,1);
  return cipherCard;
}

Card deckDecryptCard(Deck deck, Card cipherCard) {
  Card plainCard=~0;
  deckAdvance(deck,cipherCard,&plainCard,-1);
  return plainCard;
}

#define CODE_LEN        36
#define SHIFT_DOWN      36
#define SHIFT_UP        37
#define SHIFT_LOCK_DOWN 38
#define SHIFT_LOCK_UP   39

const wchar_t *DOWN_CODES = L"0123456789ABCDEF@=\\~#$%^&|-+/*\n;?\'\U0001F622\U0001F604";
const wchar_t *     CODES = L"abcdefghijklmnopqrstuvwxyz<>() ,.\"\U0001F44E\U0001F44D";
const wchar_t *  UP_CODES = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ{}[]_:!`\U0001F494\u2764";
const wchar_t *ALL_CODES[] = { DOWN_CODES, CODES, UP_CODES };


static int find(const wchar_t *str, int len, int code)
{
  if (code >= 0) {
    for (int pos=0; pos<len; ++pos) {
      if (str[pos] == code) {
	return pos;
      }
    }
  }
  return -1;
}

static int ord(int shift, int code)
{
  if (code < 0) return -1;
  
  if (shift >= 0) {
    return find(ALL_CODES[shift],CODE_LEN,code);
  } else {
    for (int i=0; i<3; ++i) {
      if (find(ALL_CODES[i],CODE_LEN,code) >= 0) return -1;
    }
    return code;
  }
}

#define WRITE(card) { if (write != NULL) { int writeStatus = write(card,writeParms); if (writeStatus != 0) return writeStatus; } ; ++count; }
#define CHR(i) (((i) < strLen) ? str[i] : -1)
#define ORD(shift,chr) ord(shift,chr)


int cardEncodeWrite(const wchar_t *str, int strLen, CardWrite *write, void *writeParms)
{
  int count = 0;
  int shift = 1;
  int next = 1;
  
  for (;;) {
    int chr = CHR(0);
    if (chr < 0) return count;
    int pos = ORD(shift,chr);
    
    if (pos >= 0) {
      if (shift == -1) {
	int s=4;
	while ((uint64_t(1)<<s) < uint64_t(chr)) { s += 4; }
	while (s >= 4) {
	  s -= 4;
	  uint8_t hex = (chr >> s) & 0xF;
	  WRITE(hex);
	}
	if (ORD(-1,CHR(1)) >= 0) {
	  WRITE(SHIFT_LOCK_DOWN);
	  next=-1;
	} else {
	  WRITE(SHIFT_LOCK_UP);
	  next=0;
	}
      } else {
	WRITE(pos);
      }
      shift = next;
      ++str;
      --strLen;
      continue;
    }

    // find best next shift state en[q] for shift state q-1
    // preferring nearby shift states for ties.  Ties are
    // only possible for capital letter A-F sequences, which are
    // encoded UP and DOWN as CAPS and HEX.  At most 10 (PREFIX)
    // characters are looked at for

    // look forward only until ties are broken
    int en[4]={0,0,0,0};
    for (int el=0; ; ++el) {
      int ties = 0;
      for (int q=-1; q<3; ++q) {
	if (en[q+1] == el && ORD(q,CHR(el)) >= 0) { ++en[q+1]; ++ties; }
      }
      if (ties < 2) break;
    }

    // choose longer encoding or nearest (larger) shift state for ties
    int m=-1;
    for (int q=0; q<3; ++q) {
      if (en[m+1] < en[q+1] || (en[m+1] == en[q+1] && abs(q-next) <= abs(m-next))) {
	m = q;
      }
    }

    // lock for unicode, multiple matches or next shift state makes no progress
    int lock = (m == -1) || (en[m+1] > 1) || (ORD(next,CHR(1)) < 0);

    while (shift != m) {
      if (shift < m) {
	++shift;
	Card up = lock ? SHIFT_LOCK_UP : SHIFT_UP;
	WRITE(up);
      }
      if (m < shift) {
	--shift;
	Card down = lock ? SHIFT_LOCK_DOWN : SHIFT_DOWN;
	WRITE(down);
      }      
    }
    if (lock) { next = shift; }
  }
}

int cardEncodeLen(const wchar_t *str, int len) {
  return cardEncodeWrite(str,len,NULL,NULL);
}

struct CardWriteArrayParms {
  Card *cards;
  int size;
  int capacity;
};

int cardWriteToArray(Card card, void *voidParms) {
  CardWriteArrayParms *parms=(CardWriteArrayParms*)voidParms;
  if (parms->size >= parms->capacity) {
    return -1;
  }
  parms->cards[parms->size]=card;
  ++parms->size;
  return 0;
}

int cardEncodeToArray(const wchar_t *str, int strLen,
		  Card *cards, int capacity)
{
  CardWriteArrayParms parms;
  parms.cards=cards;
  parms.size=0;
  parms.capacity=capacity;
  int status = cardEncodeWrite(str,strLen,cardWriteToArray,&parms);
  return status;
}

int cardDecodeWrite(CardRead *read, void *readParms, WideCharWrite *write, void *writeParms) {
  int count = 0;
  int shift = 1;
  int next = 1;
  int unicode = -1;
  for (;;) {
    int card = read(readParms);
    if (shift == -1) {
      if (0 <= card && card < 16) { // base 16 encoding
	unicode = (unicode > 0 ? 16*unicode : 0) + card;
      } else if (20 <= card && card < 30) { // base 10 encoding
	unicode = (unicode > 0 ? 10*unicode : 0) + (card-20);
      } else {
	if (write != NULL && unicode >= 0) {
	  int status = write(unicode,writeParms);
	  if (status != 0) {
	    return status;
	  }
	}
	unicode=-1;
	shift = next;
	++count;
      }
    } else if (0 <= card && card < 36) {
      if (write != NULL && shift >= 0) {
	int code = ALL_CODES[shift][card];
	int status = write(code,writeParms);
	if (status != 0) {
	  return status;
	}
      }
      shift = next;
      ++count;
    }
    if (card < 0) return (card == -1) ? count : card;

    if ((card == SHIFT_UP || card == SHIFT_LOCK_UP) && shift <= 1) {
      ++shift;
    }
    if ((card == SHIFT_DOWN || card == SHIFT_LOCK_DOWN) && shift >= 0) {
      --shift;
    }
    if (card == SHIFT_LOCK_DOWN || card == SHIFT_LOCK_UP) {
      next = shift;
    }
  }
}

struct CardStepReadParms {
  Card *cards;
  int step;
  int len;
};

static int cardStepRead(void *voidParms) {
  CardStepReadParms *parms = (CardStepReadParms *) voidParms;
  if (parms->len > 0) {
    int card = parms->cards[0];
    parms->cards += parms->step;
    parms->len -= parms->step;
  } else {
    return -1;
  }
}

int cardDecodeLen(Card *cards, int step, int len) {
  CardStepReadParms parms;
  parms.cards=cards;
  parms.step=step;
  parms.len=len;
  return cardDecodeWrite(cardStepRead,&parms,NULL,NULL);
}

struct WideCharArrayWrite {
  wchar_t *str;
  int capacity;
  int size;
};

int wideCharWriteToArray(wchar_t chr, void *voidParms) {
  WideCharArrayWrite *parms=(WideCharArrayWrite*)voidParms;
  if (parms->size >= parms->capacity) {
    return -1;
  }
  parms->str[parms->size]=chr;
  ++parms->size;
  return 0;
}

int cardDecodeToArray(Card *cards, int cardsStep, int cardsLen,
		      wchar_t *str, int capacity) {

  CardStepReadParms readParms;
  readParms.cards=cards;
  readParms.step=cardsStep;
  readParms.len=cardsLen;
  WideCharArrayWrite writeParms;
  writeParms.str = str;
  writeParms.size = 0;
  writeParms.capacity = capacity;
  return cardDecodeWrite(cardStepRead,&readParms,wideCharWriteToArray,&writeParms);
}

int cardEnvelopeLen(int encodeLen) {
  int envelopeLen = encodeLen;
  envelopeLen += PREFIX/2;
  if ((envelopeLen % PREFIX) != 0) {
    envelopeLen += (PREFIX-(envelopeLen%PREFIX));
  }
  envelopeLen *= 2;
  envelopeLen += PREFIX;

  return envelopeLen;
}


struct DeckEncryptEnvelopeWriteParms {
  Card *deck;
  CardRead *rng;
  void *rngParms;
  int position;
  CardWrite *write;
  void *writeParms;
};

int deckEncryptEnvelopeWrite(int plain, void *voidParms) {
  DeckEncryptEnvelopeWriteParms *parms=
    (DeckEncryptEnvelopeWriteParms *)voidParms;
  
  if (plain < 0) return plain;
  int cipher = deckEncryptCard(parms->deck,plain);
  return parms->write(cipher,parms->writeParms);
}

int deckEncryptEnvelopEncodeWrite(Card plainCard, void *voidParms) {
  DeckEncryptEnvelopeWriteParms *parms=
    (DeckEncryptEnvelopeWriteParms *)voidParms;

  int inject = parms->rng(parms->rngParms);
  int status = deckEncryptEnvelopeWrite(inject,voidParms);
  if (status < 0) return status;
  return deckEncryptEnvelopeWrite(plainCard,voidParms);
}

int deckEncryptEnvelopeWrite(Deck deck, const wchar_t *str, int strLen, CardRead *rng, void *rngParms, CardWrite *write, void *writeParms)
{
  DeckEncryptEnvelopeWriteParms parms;
  int status;
  
  parms.deck = deck;
  parms.rng = rng;
  parms.rngParms = rngParms;
  parms.write = write;
  parms.writeParms = writeParms;

  for (int i=0; i<PREFIX; ++i) {
    int randCard = rng(rngParms);
    int status = deckEncryptEnvelopeWrite(randCard,&parms);
    if (status < 0) { 
      return status;
    }
  }

  int encLen = cardEncodeWrite(str,strLen,deckEncryptEnvelopEncodeWrite,&parms);
  if (encLen < 0) {
    return encLen;
  }

  int pad = 0;
  while (pad < PREFIX/2 || (encLen+pad) % PREFIX != 0) {
    int status = deckEncryptEnvelopEncodeWrite(CARDS-1,&parms);
    if (status < 0) { 
      return status;
    }
    ++pad;
  }
  return PREFIX + 2*(encLen+pad);
}

void *RandOpen() {
  return (void*)fopen("/dev/urandom","rb");
}
void RandClose(void *voidParms) {
  fclose((FILE*)voidParms);
}
int RandCard(void *voidParms) {
  FILE *parms = (FILE*) voidParms;
  uint8_t x;
  
  do {
    int status = fread((char*) &x,sizeof(x),1,parms);
    if (status < 0) return status;
  } while (x >= (256-256%CARDS));
  return x % CARDS;
}

int deckEncryptEnvelopeToArray(Deck deck, const wchar_t *str, int strLen, CardRead *rng, void *rngParms, Card *cards, int capacity)
{
  CardWriteArrayParms parms;
  parms.cards=cards;
  parms.size=0;
  parms.capacity=capacity;
  int status = deckEncryptEnvelopeWrite(deck,str,strLen,rng,rngParms,cardWriteToArray,&parms);
  return status;
}

int deckDecryptEnvelopeToArray(Deck deck, const Card *env, int envLen, WideCharWrite *write, void *writeParms) {
  int j=0;
  for (int i=0; i<PREFIX; ++i) {
    Card card = deckDecryptCard(deck,env[i]);
    if (j < strLen) str[j--]=cardFaceFromNo(cardFaceNo(card));
    if (j < strLen) str[j--]=cardSuiteFromNo(cardSuiteNo(card));
    if (j < strLen) str[j--]=(i < PREFIX-1) ? ' ' : ':';
  }
  int strLen=cardDecodeToArray(env+PREFIX,2,envLen-PREFIX,str+3*PREFIX,capacity-3*PREFIX);
  
  for (int i=(envLen/2)*2; i>=PREFIX; i -= 2) {
    
  }

  int i,ends=0;
  for (i=0; i<PREFIX; ++i) {
    if (i < envLen) {
      
    } else {
      return -1;
    }
  }
  
  state = 0;
  while (i <= envLen) {
    if (i < envLen) {
      env[i] = deckDecryptCard(deck,env[i]);
    }
    if (i == envLen || env[i] != CARD-1) {
      if (ends >= PREFIX/2) {
	return (i-PREFIX)/2-ends;
      }
      ends=0;
    } else {
      ++ends;
    }
    ++i;
  }
  return -(i+1);

  
  
    int status = deckEncryptEnvelopeWrite(randCard,&parms);
    if (status < 0) { 
      return status;
    }
  }

  int encLen = cardEncodeWrite(str,strLen,deckEncryptEnvelopEncodeWrite,&parms);
  if (encLen < 0) {
    return encLen;
  }

  int pad = 0;
  while (pad < PREFIX/2 || (encLen+pad) % PREFIX != 0) {
    int status = deckEncryptEnvelopEncodeWrite(CARDS-1,&parms);
    if (status < 0) { 
      return status;
    }
    ++pad;
  }
  return PREFIX + 2*(encLen+pad);

}

