#include <vector>
#include <algorithm>

#include "config.h"
#include "deck.h"

namespace spider {

  unsigned Deck::forward(unsigned loc, unsigned delta) const {
    unsigned n = cards.size();
    for (;;) {
      while (cards[loc].order >= 40) {
	++loc;
	if (loc >= n) { loc = 0; }
      }
      if (delta == 0) return loc;
      --delta;
      ++loc;
      if (loc >= n) { loc = 0; }      
    }
  }

  unsigned Deck::back(unsigned loc, unsigned delta) const {
    unsigned n = cards.size();
    for (;;) {
      while (cards[loc].order >= 40) {
	if (loc == 0) { loc = n; }
	--loc;
      }
      if (delta == 0) return loc;
      --delta;
      if (loc == 0) { loc = n; }
      --loc;
    }
  }

  int Deck::modulus() const { return cards.size() == 10 ? 10 : 40; }
  void Deck::reset() {
    sort(cards.begin(),cards.end());
  }

  Deck::Deck(size_t size) : cards(size) {
    for (size_t i=0; i<size; ++i) {
      cards[i]=Card(i);
    }
  }

  int Deck::find(const std::vector<Card> &cards, const Card &card) {
    auto loc = std::find(std::begin(cards), std::end(cards), card);
    return (loc != std::end(cards)) ? loc-std::begin(cards) : -1;
  }
  
  int Deck::find(const Card &card) const {
    return find(cards,card);
  }

  const Card& Deck::after(const Card &card) const {
    int loc = find(card);
    assert(loc >= 0);
    return cards[forward(loc,1)];
  }

  const Card& Deck::before(const Card &card) const {
    int loc = find(card);
    assert(loc >= 0);
    return cards[back(loc,1)];
  }

  Card Deck::addMod(const Card &a, const Card &b) const {
    return spider::addMod(a,b,modulus());
  }

  Card Deck::subMod(const Card &a, const Card &b) const {
    return spider::subMod(a,b,modulus());
  }

  Card Deck::cipherPad() const {
    return after(addMod(forward(0,CIPHER_ZTH),Card(CIPHER_OFFSET)));
  }
  
  Card Deck::cutPad() const {
    // z=0 d=-1 z_luck = 0.516776 z_ext_luck = 0.700353 z2_luck = 1535.57 z2_ext_luck = 1.37824
    // z=1 d=-1 z_luck = -0.489242 z_ext_luck = -0.270511 z2_luck = 37.3675 z2_ext_luck = 2.17101
    // z=2 d=-1 z_luck = -0.228129 z_ext_luck = 0.0897686 z2_luck = 29.3273 z2_ext_luck = 2.38971
    // z=3 d=-1 z_luck = 0.78019 z_ext_luck = 0.676883 z2_luck = 20.3466 z2_ext_luck = 1.00941
    // *** z=4 d=-1 z_luck = -0.169236 z_ext_luck = 0.449471 z2_luck = 19.0554 z2_ext_luck = 0.88177
    // *** z=4 d=-1 z_luck = -0.230149 z_ext_luck = 0.521112 z2_luck = 19.4455 z2_ext_luck = -0.257503
    // z=4 d=1  z_luck = -0.496205 z_ext_luck = -0.572652 z2_luck = 1534.44 z2_ext_luck = 1.16215
    // z=4 d=21 z_luck = -0.981116 z_ext_luck = -0.538815 z2_luck = 18.8439 z2_ext_luck = -0.227434
    // z=4 d=2 z_luck = -0.981116 z_ext_luck = -0.538815 z2_luck = 18.8439 z2_ext_luck = -0.227434
    // z=5 d=-1 z_luck = 0.955431 z_ext_luck = -1.43765 z2_luck = 20.1322 z2_ext_luck = -0.916298
 
    // z=6 d=-1 z_luck = 0.100936 z_ext_luck = -0.011939 z2_luck = 18.6241 z2_ext_luck = -0.152169
    // z=7 d=-1 z_luck = -0.688455 z_ext_luck = -0.76271 z2_luck = 18.2414 z2_ext_luck = 0.126769

    //return zth(4); // 6.67
    // return zth(1); // 469.25
    //return zth(2); // 8.22
    //return zth(3); // 8.27
    //    return zth(4); // 8.16
    //return zth(5); // 7.68
    // return zth(6); // 7.2
    //return zth(7); // 8.5
    // return zth(2); // 8.5     
    return after(addMod(forward(0,CUT_ZTH),Card(CUT_OFFSET)));
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
    Card beforePad = before(pad);
    Card zthCard = subMod(beforePad,CUT_OFFSET);
    int zthLoc = find(temp,zthCard);
    int topLoc = back(zthLoc,CUT_ZTH);
					
    std::copy(std::begin(temp), std::begin(temp)+(topLoc),
	      std::end(cards)-(topLoc));
    std::copy(std::begin(temp)+(topLoc),std::end(temp),std::begin(cards));
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
