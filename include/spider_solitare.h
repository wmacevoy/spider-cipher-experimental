#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// number of cards in deck
#define CARDS 40
// 0-based index of cut pad card
#define CUT_ZTH 0
// 0-based index of cipher mark card
#define MARK_ZTH 2
// add-offset to mark card
#define MARK_OFFSET 39

/* 0..39 values only */
typedef unsigned char Card;
typedef Card Deck[CARDS];

/* 0,1,2,3 ... 8,9 for Q,A,23 ... 8,9 of any suite */
int cardFaceNo(Card card);

/* 0,1,2,3 for club, diamond, heart, spade of any face */
int cardSuiteNo(Card card);

/* 'Q','A','1','2','3', ... '8','9' for 0,1,2,3,...,8,9 */
wchar_t cardFaceFromNo(int cardFaceNo);

/* 'Q','A','1','2','3', ... '8','9' for 0,1,2,3,...,8,9 */
wchar_t cardFaceFromNo(int cardFaceNo);

/* unicode club, diamond, heart, spade from 0,1,2,3 */
wchar_t cardSuiteFromNo(int cardSuiteNo);

/* modular 40 add */
Card cardAdd(Card x, Card y);

/* modular 40 subtract */
Card cardSubtract(Card x, Card y);

/* fixed time deck cut at location, so output[0]=input[cutLoc]  */
void deckCut(Deck input, int cutLoc,Deck output);

/* back-front shuffle of card deck */
void deckBackFrontShuffle(Deck input, Deck output);

/* fixed time find card in deck (assumes card exists uniquely in deck) */
int deckFindCard(Deck deck,Card card);

/* in-place fixed-time pseudo-shuffle at given cut location */
void deckPseudoShuffle(Deck deck, int cutLoc);

/* determine cut pad card fron deck */
Card deckCutPad(Deck deck);

/* determine cipher pad card fron deck */
Card deckCipherPad(Deck deck);

/* encrypt (mode 1) or decrypt (mode -1) input and pseudo-shuffle deck,
   OutputCard can be null; this saves time if the result is not
   needed (for example, to key a deck) */
void deckAdvance(Deck deck, Card inputCard, Card *outputCard, int mode);


/* return encrypted card and advance deck */
Card deckEncryptCard(Deck deck, Card plainCard);

/* return decrypted card and advance deck */
Card deckDecryptCard(Deck deck, Card plainCard);

typedef int CardWrite(Card card, int pos, void *parms);

typedef int WideCharWrite(wchar_t code, int pos, void *parms);
/* 
   encode wide string, returns card count and (if not NULL) calls 
   write or each card.  write should return a 0, otherwise the
   encode stops and returns this value 
*/
int cardEncodeWrite(const wchar_t *str, int strLen,
		  CardWrite *write, void *writeParms);
int cardEncodeLen(const wchar_t *str, int strLen);
int cardEncodeToArray(const wchar_t *str, int strLen,
		  Card *cards, int capacity);
  
int cardDecodeWrite(Card *cards,int cardsLen,
		  WideCharWrite *write, void *writeParms);
int cardDecodeLen(Card *cards,int cardsLen);
int cardDecodeToArray(Card *cards,int cardsLen,
		  wchar_t *str, int capacity);

#ifdef __cplusplus
} // extern "C"
#endif
