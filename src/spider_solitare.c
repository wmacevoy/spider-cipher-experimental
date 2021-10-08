#include <stdio.h>
#include <assert.h>
#include "spider_solitare.h"

#define ADD(x,y) (((x)+(y))%CARDS)
#define SUB(x,y) (((x)+(CARDS-(y)))%CARDS)

static const unsigned MASK[] = {0U,~0U};

#define CUT_PAD(deck) deck[CUT_ZTH]
#define CUT_CARD(deck,plain) ADD(CUT_PAD(deck),plain)

#define MARK_CARD(deck) ADD(deck[MARK_ZTH],MARK_OFFSET)

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
  FIND1(deck,offset+0,card)				\
    FIND1(deck,offset+1,card)				\
    FIND1(deck,offset+2,card)				\
    FIND1(deck,offset+3,card)				\
    FIND1(deck,offset+4,card)

#define FIND10(deck,offset,card)	\
  FIND5(deck,offset,card) |		\
    FIND5((deck),offset+5,card)

#define FIND20(deck,offset,card)	\
  FIND10(deck,offset,card) |		\
    FIND10(deck,offset+10,card)

#define FIND(deck,card)	 (FIND20(deck,0,card) | FIND20(deck,20,card))

#define AFTER(deck,card)  ((deck)[ADD(FIND(deck,card),1)])

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
  return L"CDHS"[cardSuiteNo];
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
  unsigned ans = 0;
  for (unsigned i=0; i<CARDS; ++i) {
    ans |= (i & MASK[deck[i]==card]);
  }
  return ans;
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

#define WRITE(card) { if (write != NULL) { int writeStatus = write(card,count,writeParms); if (writeStatus != 0) return writeStatus; } ; ++count; }
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
  int capacity;
};

int cardWriteToArray(Card card, int position, void *voidParms) {
  CardWriteArrayParms *parms=(CardWriteArrayParms*)voidParms;
  if (position >= parms->capacity) {
    return -1;
  }
  parms->cards[position]=card;
  return 0;
}

int cardEncodeToArray(const wchar_t *str, int strLen,
		  Card *cards, int capacity)
{
  CardWriteArrayParms parms;
  parms.cards=cards;
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
	int status = write(code,count,writeParms);
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
};

int wideCharWriteToArray(wchar_t chr, int position, void *voidParms) {
  WideCharArrayWrite *parms=(WideCharArrayWrite*)voidParms;
  if (position >= parms->capacity) {
    return -1;
  }
  parms->str[position]=chr;
  return 0;
}

int cardDecodeToArray(Card *cards, int cardsLen,
		      wchar_t *str, int capacity) {

  WideCharArrayWrite parms;
  parms.str = str;
  parms.capacity = capacity;
  return cardDecodeWrite(cards,cardsLen,wideCharWriteToArray,&parms);
}






