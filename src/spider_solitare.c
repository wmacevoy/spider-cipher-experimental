#include <stdio.h>
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

#define CODE_LEN 36
#define SHIFT_DOWN 36
#define SHIFT_UP   37
#define SHIFT_LOCK_DOWN 38
#define SHIFT_LOCK_UP   39

const wchar_t *DOWN_CODES = L"0123456789ABCDEF@=\\~#$%^&|-+/*\n;?\'\U0001F622\U0001F604";
const wchar_t *     CODES = L"abcdefghijklmnopqrstuvwxyz<>() ,.\"\U0001F44E\U0001F44D";
const wchar_t *  UP_CODES = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ{}[]_:!`\U0001F494\u2764";
const wchar_t *ALL_CODES[] = { DOWN_CODES, CODES, UP_CODES };


int find(const wchar_t *str, int len, int code)
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

#define WRITE(card) { if (write != NULL) { int writeStatus = write(card,writeParms); if (writeStatus != 0) return writeStatus; } ; ++count; }
#define CHR(i) ((i) < bufLen ? buf[bufLen-1-(i)] : (i)-bufLen < len ? str[(i)-bufLen] : -1)
#define ORD(shift,chr) find(ALL_CODES[shift],CODE_LEN,chr)

int cardEncodeWrite(const wchar_t *str, int len, CardWrite *write, void *writeParms)
{
  int count = 0;
  int shift = 1;
  int next = 1;
  wchar_t buf[16];
  int bufLen = 0;
  
  for (;;) {
    int chr = CHR(0);
    if (chr < 0) return count;
    int pos = ORD(shift,chr);
    
    if (pos >= 0) {
      WRITE(pos);
      shift = next;
      if (bufLen > 0) {
	--bufLen;
      } else {
	++str;
	--len;
      }
      continue;
    }
    
    int en[3]={0,0,0},m=-1;
    for (int query=0; query<3; ++query) {
      while (ORD(query,CHR(en[query])) >= 0) {
	++en[query];
      }
      if (m == -1 || en[m] < en[query]) {
	m = query;
      }
    }

    if (en[m]==0) {
      if (chr <= 0xFF) {
	bufLen = 4;
	buf[bufLen-1]='\\';
	buf[bufLen-2]='x';
      } else if (chr <= 0xFFFF) {
	bufLen = 6;	
	buf[bufLen-1]='\\';
	buf[bufLen-2]='u';
      } else {
	bufLen = 10;
	buf[bufLen-1]='\\';
	buf[bufLen-2]='U';
      }
      for (int i=0; i<bufLen-2; ++i) {
	buf[i]=DOWN_CODES[chr % 16];
	chr = chr / 16;
      }
      ++str;
      --len;
      continue;
    }
    
    int lock = (en[m] > 1) || (ORD(next,CHR(1)) < 0);
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

int cardDecodeWrite(Card *cards, int cardsLen, WideCharWrite *write, void *writeParms) {
  int count = 0;
  int shift = 1;
  int next = 1;
  for (;;) {
    int card = (cardsLen > 0) ? cards[0] : -1;
    --cardsLen;
    ++cards;
    if (card < 0) return (card == -1) ? count : card;
    if (card < 36) {
      if (write != NULL) {
	int code = ALL_CODES[shift][card];
	int status = write(code,writeParms);
	if (status != 0) {
	  return status;
	}
      }
      shift = next;
      ++count;
    } else {
      if ((card == SHIFT_UP || card == SHIFT_LOCK_UP) && shift <= 1) {
	++shift;
      }
      if ((card == SHIFT_DOWN || card == SHIFT_LOCK_DOWN) && shift >= 1) {
	--shift;
      }
      if (card == SHIFT_LOCK_DOWN || card == SHIFT_LOCK_UP) {
	next = shift;
      }
    }
  }
}

int cardDecodeLen(Card *cards, int cardsLen) {
  return cardDecodeWrite(cards,cardsLen,NULL,NULL);
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

int cardDecodeToArray(Card *cards, int cardsLen,
		      wchar_t *str, int capacity) {

  WideCharArrayWrite parms;
  parms.str = str;
  parms.size = 0;
  parms.capacity = capacity;
  return cardDecodeWrite(cards,cardsLen,wideCharWriteToArray,&parms);
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
  parms.writeParms = 0;

  for (int i=0; i<PREFIX; ++i) {
    int randCard = rng(rngParms);
    int status = deckEncryptEnvelopeWrite(randCard,&parms);
    if (status < 0) { 
      return status;
    }
  }

  int len = cardEncodeWrite(str,len,deckEncryptEnvelopEncodeWrite,&parms);
  if (len < 0) {
    return len;
  }

  int pad = 0;
  while (pad < PREFIX || (len+pad) % 2*PREFIX != 0) {
    int status = deckEncryptEnvelopEncodeWrite(CARDS-1,&parms);
    if (status < 0) { 
      return status;
    }
    pad += 2;
  }
  return len + pad;
}

void *RandOpen() {
  return (void*)fopen("/dev/urandom","rb");
}
void RandClose(void *voidParms) {
  fclose((FILE*)voidParms);
}
int RandCard(void *voidParms) {
  FILE *parms = (FILE*) voidParms;
  uint32_t x,r;
  
  do {
    int status = fread((char*) &x,sizeof(x),1,parms);
    if (status < 0) return status;
    r = x % CARDS;
  } while (x - r > uint32_t(-CARDS));
  return r;
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
