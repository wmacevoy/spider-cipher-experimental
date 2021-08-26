#include <vector>
#include <algorithm>

#include "config.h"
#include "deck.h"

namespace spider {
  unsigned Deck::forward(const std::vector<Card> &cards, unsigned loc, unsigned delta) {
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

  unsigned Deck::back(const std::vector<Card> &cards, unsigned loc, unsigned delta) {
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
    for (size_t i=0; i<cards.size(); ++i) {
      cards[i]=Card(i);
    }
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

  const Card& Deck::after(const std::vector<Card> &cards, const Card &card) {
    int loc = find(cards,card);
    assert(loc >= 0);
    return cards[forward(cards,loc,1)];
  }

  const Card& Deck::before(const std::vector<Card> &cards,const Card &card) {
    int loc = find(cards,card);
    assert(loc >= 0);
    int beforeLoc = back(cards,loc,1);
    return cards[beforeLoc];
  }

  Card Deck::addMod(const Card &a, const Card &b) const {
    return spider::addMod(a,b,modulus());
  }

  Card Deck::subMod(const Card &a, const Card &b) const {
    return spider::subMod(a,b,modulus());
  }

  Card Deck::cipherPad() const {
    int zthLoc = forward(cards,0,CIPHER_ZTH);
    Card zth=cards[zthLoc];
    Card mark=addMod(zth,Card(CIPHER_OFFSET));
    int markLoc = find(cards,mark);
    int cipherPadLoc = forward(cards,markLoc,1);
    return cards[cipherPadLoc];
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

    int zthLoc = forward(cards,0,CUT_ZTH);
    Card zth=cards[zthLoc];
    Card mark=addMod(zth,Card(CUT_OFFSET));
    int markLoc = find(cards,mark);
    int cutPadLoc = forward(cards,markLoc,1);
    return cards[cutPadLoc];
  }

  void Deck::mix(const Card &plain) {
    Card cut = addMod(cutPad(),plain);
    pseudoShuffle(cut);
  }


  void Deck::unmix(const Card &plainCard) {
    std::vector<Card> temp(cards.size());
    backFrontUnshuffle(cards,temp);

    Card cutCard = temp[0];
    Card cutPad = subMod(cutCard,plainCard);
    int cutPadLoc = find(temp,cutPad);
    int markLoc = back(temp,cutPadLoc,1);
    Card mark = temp[markLoc];
    Card zth=subMod(mark,Card(CUT_OFFSET));
    int zthLoc = find(temp,zth);
    int topLoc = back(temp,zthLoc,CUT_ZTH);

    cut(temp,topLoc,cards);
  }

  void Deck::cut(const std::vector<Card> &in, int cutLoc, std::vector<Card> &out) {
    out.resize(in.size());
    // copy bottom of deck (starting from cutLoc) to top of deck
    std::copy(std::begin(in)+(cutLoc),std::end(in),std::begin(out));
    // copy top of deck (up to but excluding cut card) to bottom of deck
    std::copy(std::begin(in), std::begin(in)+(cutLoc),
	      std::end(out)-(cutLoc));

  }
  
  void Deck::backFrontShuffle(const std::vector<Card> &in, std::vector<Card> &out) {
    out.resize(in.size());
    size_t back = in.size()/2;
    size_t front = back-1;
    for (size_t i=0; i<in.size(); ++i) {
      if (i % 2 == 0) {
	out[back]=in[i];
	++back;
      } else {
	out[front]=in[i];
	--front;
      }
    }
  }
  
  void Deck::backFrontUnshuffle(const std::vector<Card> &in, std::vector<Card> &out) {
    out.resize(in.size());
    int back = in.size();
    int front = -1;
    for (int i=in.size()-1; i >= 0; --i) {
      if (i % 2 == 0) {
	--back;
	out[i]=in[back];

      } else {
	++front;
	out[i]=in[front];
      }
    }
  }

  void Deck::pseudoShuffle(const Card &cutCard) {
    std::vector<Card> temp(cards.size());

    int cutLoc = find(cutCard);
    assert(cutLoc >= 0);
    cut(cards,cutLoc,temp);
    assert(temp[0] == cutCard);

    backFrontShuffle(temp,cards);
    int topLoc = forward(cards,0,0);
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

  int equivalent(const Deck &a, const Deck &b) {
    if (a.cards.size() != b.cards.size()) {
      return a.cards.size() < b.cards.size() ? -1 : 1;
    }
    unsigned a0 = Deck::forward(a.cards,0,0);
    unsigned b0 = Deck::forward(b.cards,0,0);
    int n = a.cards.size();
    for (int i=0; i<n; ++i) {
      int ai=a.cards[(a0+i) % n].order;
      int bi=b.cards[(b0+i) % n].order;
      if (ai != bi) {
	return (ai < bi) ? -1 : 1;
      }
    }
    return 0;
  }
  
}
