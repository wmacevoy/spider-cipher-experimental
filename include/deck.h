#pragma once
#include <iostream>
#include <vector>
#include <set>

#include "card.h"

namespace spider {

  struct Deck {
    std::vector<Card> cards;
    int modulus() const;
    void reset();
    Deck(size_t size);
  
    int find(const Card &card) const;

    const Card& after(const Card &card) const;

    const Card& first() const;

    const Card& second() const;

    Card addMod(const Card &a, const Card &b) const;

    Card subMod(const Card &a, const Card &b) const;

    const Card &cipherPad() const;

    const Card &cutPad() const;

    void mix(const Card &plainCard);

    void unmix(const Card &plain);
    void pseudoShuffle(const Card &cut);
    bool operator<(const Deck &deck) const;
    bool operator==(const Deck &deck) const;
    bool operator!=(const Deck &deck) const;    
  };

  std::ostream& operator<<(std::ostream &out, const Deck &deck);
  std::ostream& operator<<(std::ostream &out, const std::set<Deck> &decks);

}
