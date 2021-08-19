#include <vector>
#include <algorithm>

#include "config.h"
#include "deck.h"

namespace spider {
  int Deck::modulus() const { return cards.size() == 10 ? 10 : 40; }
  void Deck::reset() {
    sort(cards.begin(),cards.end());
  }

  Deck::Deck(size_t size) : cards(size) {
    for (size_t i=0; i<size; ++i) {
      cards[i]=Card(i);
    }
  }
  
  int Deck::find(const Card &card) const {
    auto loc = std::find(std::begin(cards), std::end(cards), card);
    return (loc != std::end(cards)) ? loc-std::begin(cards) : -1;
  }

  const Card& Deck::after(const Card &card) const {
    int loc = find(card);
    assert(loc >= 0);
    int m = modulus();
    do {
      loc = (loc + 1) % m;
    } while (cards[loc].order >= 40);
    return cards[loc];
  }

  const Card& Deck::zth(unsigned z) const {
    unsigned loc = 0, n = cards.size();
    for (;;) {
      while (cards[loc].order >= 40) {
	++loc;
	if (++loc == n) { loc = 0; }
      }
      if (z == 0) return cards[loc];
      --z;
      ++loc;
    }
  }

  const Card& Deck::first() const {
    return zth(0);
  }

  const Card& Deck::second() const {
    return zth(1);
  }

  const Card& Deck::third() const {
    return zth(2);
  }
  
  Card Deck::addMod(const Card &a, const Card &b) const {
    return spider::addMod(a,b,modulus());
  }

  Card Deck::subMod(const Card &a, const Card &b) const {
    return spider::subMod(a,b,modulus());
  }

  const Card &Deck::cipherPad() const {
    return after(addMod(first(), second()));
  }

  const Card &Deck::cutPad() const {
    return third();
  }

  void Deck::mix(const Card &plain) {
    Card cut = addMod(cutPad(),plain);
    pseudoShuffle(cut);
  }

  void Deck::unmix(const Card &plain) {
    std::vector<Card> temp(cards.size());
    int bottom = cards.size();
    int top = -1;
    for (int i=cards.size()-1; i >= 0; --i) {
      if (i % 2 == 0) {
	--bottom;
	temp[i]=cards.at(bottom);
      } else {
	++top;
	temp[i]=cards.at(top);
      }
    }

    Card cut = temp[0];
    Card pad = subMod(cut,plain);

    auto loc = std::find(std::begin(temp), std::end(temp), pad);
    int oldThirdLoc = (loc != std::end(temp)) ? loc-std::begin(temp) : -1;
    int cutloc = (oldThirdLoc + cards.size()-2) % cards.size();

    std::copy(std::begin(temp), std::begin(temp)+(cutloc),
	      std::end(cards)-(cutloc));
    std::copy(std::begin(temp)+(cutloc),std::end(temp),std::begin(cards));
  }

  void Deck::pseudoShuffle(const Card &cut) {
    std::vector<Card> temp(cards.size());

    // find cut card
    int cutloc = find(cut);

    // copy top of deck (up to but excluding cut card) to bottom of deck (move to temp)
    std::copy(std::begin(cards), std::begin(cards)+(cutloc),
	      std::end(temp)-(cutloc));
    std::copy(std::begin(cards)+(cutloc),std::end(cards),std::begin(temp));

    if (DEBUG >= 10) {
      std::cout << "cut " << cut << ":" << std::endl;
      std::cout << " before: " << cards << std::endl;
      std::cout << "  after: " << temp << std::endl;
    }

    size_t bottom = cards.size()/2;
    size_t top = bottom-1;
    for (size_t i=0; i<cards.size(); ++i) {
      if (i % 2 == 0) {
	cards.at(bottom)=temp[i];
	++bottom;
      } else {
	cards.at(top)=temp[i];
	--top;
      }
    }

    if (DEBUG >= 10) {
      std::cout << "shuffle:" << std::endl;
      std::cout << " before: " << temp << std::endl;
      std::cout << "  after: " << cards << std::endl;
    }
  }

  bool Deck::operator<(const Deck &deck) const {
    if (cards.size() != deck.cards.size()) {
      return cards.size() < deck.cards.size();
    }
    for (size_t i=0; i<cards.size(); ++i) {
      if (cards[i] != deck.cards[i]) {
	return cards[i] < deck.cards[i];
      }
    }
    return false;
  }

  bool Deck::operator==(const Deck &deck) const {
    if (cards.size() != deck.cards.size()) {
      return false;
    }
    for (size_t i=0; i<cards.size(); ++i) {
      if (cards[i] != deck.cards[i]) {
	return false;
      }
    }
    return true;
  }

  bool Deck::operator!=(const Deck &deck) const {
    return ! (*this == deck);
  }

  std::ostream& operator<<(std::ostream &out, const Deck &deck) {
    return out << deck.cards;
  }

  std::ostream& operator<<(std::ostream &out, const std::set<Deck> &decks) {
    out << "{";
    bool first = true;
    for (auto deck : decks) {
      if (first) {
	first = false;
      } else {
	out << ",";
      }
      out << deck;
    }
    out << "}";
  
    return out;
  }
}
