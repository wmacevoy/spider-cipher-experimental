#pragma once
#include <iostream>
#include <vector>
#include <set>

#include "retain.hpp"

#include "rng.h"
#include "card.h"

namespace spider {
  struct DeckConfig {
    static const int DEFAULT_CIPHER_ZTH;
    static const int DEFAULT_CIPHER_OFFSET;
    static const int DEFAULT_CUT_ZTH;
    static const int DEFAULT_CUT_OFFSET;

    DeckConfig();
    
    int cipherZth;
    int cipherOffset;
    int cutZth;
    int cutOffset;
    

    static const DeckConfig DEFAULT;
  };

  struct Deck {
    std::vector<Card> cards;

    static int padLoc(const std::vector<Card> &cards, int zth, int offset, int modulus);
    static const DeckConfig& config();
    //
    // Modulus is 10 or 40.
    //
    // 10 is the "test deck" size -- IT IS NOT SECURE
    // 40 is the "standard cipher deck"
    //
    // A deck can have more than 40 cards, they are shuffled, but
    // otherwise ignored (first and after skip past jokers and JQK
    // cards).  This way they contribute to entropy without complicating
    // the arithmetic.
    //
    int modulus() const;

    // replace with ordered deck (see card for order, it is not normal)
    void reset();
    void shuffle(RNG &rng);
    
    Deck(size_t size);
  
    int find(const Card &card) const;
    static int find(const std::vector<Card> &cards, const Card &card);

    // move forward delta card skipping J,Q,K
    static unsigned forward(const std::vector<Card> &cards, unsigned index, unsigned delta);

    // move back delta card skipping J,Q,K
    static unsigned back(const std::vector<Card> &cards, unsigned index, unsigned delta);
    
    // find card after given card, skipping J,Q,K and jokers.
    static const Card& after(const std::vector<Card> &cards, const Card &card);

    // find card after given card, skipping J,Q,K and jokers.
    static const Card& before(const std::vector<Card> &cards, const Card &card);

    Card addMod(const Card &a, const Card &b) const;

    Card subMod(const Card &a, const Card &b) const;

    // for the real deck - the card after (the first card + 11)
    // for a test deck (10 cards) - the card after (the first card + 1)
    // this is what is modulus added to the plain text to obtain the
    // the cipher text; or modulus subtracted from the cipher text to
    // get the plain text.
    Card cipherPad() const;

    // this is the second card (skipping JQK and jokers) of the deck.
    // this is what is modulus added to the plain text to obtain the
    // "cut" card to advance the deck for the next encode/decode step.
    Card cutPad() const;

    // apply the "plain text" card to shuffle the deck.
    void mix(const Card &plain);

    // unapply to reverse the deck shuffle.  This is only needed for
    // exploring mathematical / statistical properties of the the
    // permutation, you do not need it to encode or decode a cipher.
    // the reversibility of the permutation actually strengthens the
    // cipher and helps prove important properites.  Since it only
    // used as a symmetric cipher knowing how to undo the steps does
    // not reveal any information the user does not already know.
    void unmix(const Card &plain);

    static void cut(const std::vector<Card> &in, int cutLoc, std::vector<Card> &out);
    static void backFrontShuffle(const std::vector<Card> &in, std::vector<Card> &out);
    static void backFrontUnshuffle(const std::vector<Card> &in, std::vector<Card> &out);    
    // apply the core shuffle
    
    void pseudoShuffle(const Card &cut);

    bool operator<(const Deck &deck) const;
    bool operator==(const Deck &deck) const;
    bool operator!=(const Deck &deck) const;    
  };

  std::ostream& operator<<(std::ostream &out, const Deck &deck);
  std::ostream& operator<<(std::ostream &out, const std::set<Deck> &decks);
  int equivalent(const Deck &a, const Deck &b);

}
