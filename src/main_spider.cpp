#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <algorithm>

struct RNG {
  static RNG &DEFAULT;
  virtual uint32_t next_u32() = 0;
  
// 
// Debiased Modulo (Once) — Java's Method
// https://www.pcg-random.org/posts/bounded-rands.html
//
  uint32_t next_u32(uint32_t n) {
    uint32_t x, r;
    do {
      x = next_u32();
      r = x % n;
    } while (x - r > uint32_t(-n));
    return r;
  }
  
  int next(int a, int b) {
    return a + int((b>a) ? next_u32(b-a+1) : 0);
  }
  
  virtual ~RNG() {};
};


struct OS_RNG : RNG {
  FILE *fd;
  OS_RNG() {
    fd = fopen("/dev/urandom","rb");
  }
  ~OS_RNG() {
    fclose(fd);
  }

  uint32_t next_u32() {
    uint32_t x;
    fread(&x,sizeof(x),1,fd);
    return x;
  }
};

OS_RNG DEFAULT_OS_RNG;

RNG& RNG::DEFAULT(DEFAULT_OS_RNG);

struct Card {
  static const uint8_t JOKER_SUITE = 4;
  static const char * const SUITES[];
  static const char * const FACES[];
  static const char * const JOKER_FACES[];  

  uint8_t order;
  Card() : order(0) {}
  Card(const Card &card) : order(card.order) {}
  Card(uint8_t _order) : order(_order) {}

  uint8_t suiteNumber() const {
    if (order < 40) {
      return order / 10;
    } else if (order < 52) {
      return (order-40) / 3;
    } else {
      return JOKER_SUITE;
    }
  }

  const char * suite() const {
    return SUITES[suiteNumber()];
  }
  
  uint8_t faceNumber() const {
    if (order < 40) {
      return order % 10;
    } else if (order < 52) {
      return 10+((order-40) % 3);
    } else {
      return (order - 52);
    }
  }

  const char * face() const {
    return joker() ? JOKER_FACES[faceNumber()] : FACES[faceNumber()];
  }

  Card(uint8_t faceNumber, uint8_t suiteNumber) {
    if (suiteNumber == JOKER_SUITE) {
      order = 52 + (faceNumber);
    } else {
      if (faceNumber < 10) {
	order = suiteNumber*10 + faceNumber;
      } else {
	order = 40 + 3*suiteNumber + (faceNumber - 10);
      }
    }
  }

  bool joker() const {
    return order >= 52;
  }

  bool operator<(const Card &to) const {
    return order < to.order;
  }
  bool operator<=(const Card &to) const {
    return order <= to.order;
  }
  bool operator==(const Card &to) const {
    return order == to.order;
  }
  bool operator>=(const Card &to) const {
    return order >= to.order;
  }
  bool operator>(const Card &to) const {
    return order > to.order;
  }

  void operator=(const Card &card) {
    order = card.order;
  }
};

std::ostream& operator<<(std::ostream &out, const Card &card) {
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%02d(%s%s)",card.order,card.face(),card.suite());
  return out << buffer;
}

Card addMod(const Card &a, const Card &b, int n) {
  return Card((a.order + b.order) % n);
}


Card subMod(const Card &a, const Card &b, int n) {
  return Card((a.order + (n-b.order)) % n);
}

struct Deck {
  std::vector<Card> cards;
  int modulus() const { return cards.size() == 10 ? 10 : 40; }

  void order() {
    std::sort(std::begin(cards),std::end(cards));
  }

  Deck(size_t size) : cards(size) {
    for (size_t i=0; i<size; ++i) {
      cards[i]=Card(i);
    }
  }
  
  int find(const Card &card) const {
    auto loc = std::find(std::begin(cards), std::end(cards), card);
    return (loc != std::end(cards)) ? loc-std::begin(cards) : -1;
  }

  const Card& after(const Card &card) const {
    int loc = find(card);
    do {
      loc = (loc + 1) % modulus();
    } while (cards[loc].order >= 40);
    return cards[loc];
  }

  const Card& first() const {
    int loc = 0;
    while (cards[loc].order >= 40) {
      ++loc;
    }
    return cards[loc];
  }

  const Card& second() const {
    int loc = 0;
    while (cards[loc].order >= 40) {
      ++loc;
    }
    ++loc;
    while (cards[loc].order >= 40) {
      ++loc;
    }
    return cards[loc];
  }

  Card add(const Card &a, const Card &b) const {
    return addMod(a,b,modulus());
  }

Card sub(const Card &a, const Card &b) const {
    return subMod(a,b,modulus());
  }

  const Card &cipherPad() const {
    return after(add(first(), Card(modulus() == 10 ? 1 : 11)));
  }

  const Card &cutPad() const {
    return second();
  }

  void pseudoShuffle(const Card &cut) {
    std::vector<Card> temp(cards.size());

    // find cut card
    int cutloc = find(cut);

    // copy top of deck (up to but excluding cut card) to bottom of deck (move to temp)
    std::copy(std::begin(cards), std::begin(cards)+(cutloc),
	      std::end(temp)-(cutloc));
    std::copy(std::begin(cards)+(cutloc),std::end(cards),std::begin(temp));

    size_t top = cards.size()/2+1;
    size_t bottom = top-1;
    for (size_t i=0; i<cards.size(); ++i) {
      if (i % 2 == 0) {
	++bottom;
	cards.at(bottom)=temp[i];
      } else {
	--top;
	cards.at(top)=temp[i];
      }
    }
  }
};

const char * const Card::SUITES[] = { "C","D","H","S","J" };
const char * const Card::FACES[] = { "10","A","2","3","4","5","6","7","8","9", "J", "Q", "K" };
const char * const Card::JOKER_FACES[] = { "A","B","C","D"};

std::ostream& operator<<(std::ostream &out, const Deck &deck) {
  for (size_t i=0; i<deck.cards.size(); ++i) {
    if (i > 0) out << " ";
    out << deck.cards[i];
  }
  return out;
}



struct Messenger {
  Deck m_key;
  RNG &m_rng;
  int m_cards;
  int m_prefixLen;
  int m_minSuffixLen;
  int m_mulLen;
  std::string m_text;
  std::vector<Card> m_plaincards;
  std::vector<Card> m_ciphercards;
  
  Messenger(const Deck &key,
	    RNG &rng = DEFAULT_OS_RNG,
	    int cards = 40,
	    int prefixLen = 10,
	    int minSuffixLen = 5,
	    int mulLen = 10)
    : m_key(key),
      m_rng(rng),
      m_cards(cards),
      m_prefixLen(prefixLen),
      m_minSuffixLen(minSuffixLen),
      m_mulLen(mulLen) {
  }
  
  const std::string &text() const { return m_text; }
  void text(const std::string &value) { m_text = value; }

  const std::vector<Card> plaincards() const { return m_plaincards; }
  const std::vector<Card> ciphercards() const { return m_ciphercards; }
  void ciphercards(const std::vector<Card> &value) { m_ciphercards = value; }
  
  void addPrefix() {
    for (int i=0; i<prefixLen; ++i) {
      int order = rng.next(0,cards-1);
      Card card(order);
      plaintext.push_back(card);
    }
  }

  void addSuffix() {
    for (int i=0; i<minSuffixLen; ++i) {
      Card card((i==0 && cards == 10) ? 0 : cards-1);
      cards.push_back(card);
    }
    while (cards.size() % mulLen != 0) {
      cards.push_back(Card(cards-1));
    }
  }

  void removeSuffix() {
    while (plaincards.size() > 0 && plaincards[plaincards.size()-1].order = cards-1) {
      plaincards.pop_back();
    }
    if (cards == 10 && plaincards.size() > 0 && plaincards[plaincards.size()-1].order == 0) {
	plaincards.pop_back();
    }
  }

  void addModPrefix() {
    for (int i=prefixLen; i<plaincards.size(); ++i) {
      plaincards[i] = Card((plaincards[i].order + plaincards[i % modDiv].order) % cards);
    }
  }

  void removeModPrefix() {
    for (int i=prefixLen; i<plaincards.size(); ++i) {
      plaincards[i] = Card((plaincards[i].order + (cards-plaincards[i % modDiv].order)) % cards);
    }
  }

  int m_shift;
  bool m_lock;
  void add(const Card &card) {
    plaincards.push_back(card);
    if (!m_lock) {
      m_shift = 0;
    }
  }

  void shift(int value, bool lock = false) {
    if (value < -1) value = -1;
    if (value >  1) value = 1;
    while (m_shift != value) {
      if (m_shift < value) {
	plaincards.push_back(lock ? SHIFT_UP_LOCK : SHIFT_UP);
	++m_shift;
      } else {
	plaincards.push_back(lock ? SHIFT_DOWN_LOCK : SHIFT_DOWN);
	--m_shift;
      }
    }
    if (m_shift != 0
  }

  void encodeAsOctet(uint8_t value) {
    assert(false);
  }

  void encode() {
    int shift = 0;
    int i = 0;
    while (i < m_text.size()) {
      int unLen = 0;
      while (i + unLen < m_text.size() && UN.find(m_text[i+unLen]) >= 0) ++unLen;
      int downLen = 0;
      while (i + downLen < m_text.size() && DOWN.find(m_text[i+downLen]) >= 0) ++downLen;
      int upLen = 0;
      while (i + upLen < m_text.size() && UP.find(m_text[i+upLen]) >= 0) ++upLen;

      int maxLen = max(max(downLen,upLen),unLen);
      if (maxLen == 0) {
	encodeOctet(m_text[i]);
	++i;
      } else {
	if ((shift == 0 && unLen > 0) || (unLen == maxLen)) {
	  if (shift != 0) {
	    if (shift == -1) {
	      plaincards.push_back(CARD_SHIFT_LOCK_UP);
	    } else {
	      plaincards.push_back(CARD_SHIFT_LOCK_DOWN);	      
	    }
	    shift = 0;
	  }
	  for (int j=0; j<unLen; ++j) {
	    plaincards.push_back(Cards(UN.find(m_text[i])));
	    ++i;
	  }
	} else if ((shift == 1 && upLen > 0) || (upLen == maxLen)) {
	  if (shift != 1) {
	    if (shift == -1) {
	      plaincards.push_back(CARD_SHIFT_LOCK_UP);
	    }
	    plaincards.push_back(upLen > 1 ? CARD_SHIFT_LOCK_UP : CARD_SHIFT_UP);
	  }
	  for (int j=0; j<upLen; ++j) {
	    plaincards.push_back(Cards(UP.find(m_text[i])));
	    ++i;
	  }
	  shift = (upLen > 1) ? 1 : 0;
	} else {
	  if (shift != -1) {
	    if (shift == 1) {
	      plaincards.push_back(CARD_SHIFT_LOCK_DOWN);
	    }
	    plaincards.push_back(downLen > 1 ? CARD_SHIFT_LOCK_DOWN : CARD_SHIFT_DOWN);
	  }
	  for (int j=0; j<upLen; ++j) {
	    plaincards.push_back(Cards(UP.find(m_text[i])));
	    ++i;
	  }
	  shift = (downLen > 1) ? -1 : 0;
	}
      }
    }
  }
  
  void encrypt() {
    Deck work(m_key);
    plaincards.clear();
    ciphercards.clear();
    addPrefix();
    encode();
    addSuffix();
    addModPrefix();
    for (int i=0; i<plaincards.size(); ++i) {
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card cutCard = work.add(plaincards[i],cutPad);
      Card cipherCard = work.add(plaincards[i],cipherPad);
      ciphercards.push_back(cipherCard);
      work.pseudoShuffle(cutCard);
    }
    return 0;
  }
  
  bool decrypt() {
    Deck work(m_key);
    plaincards.clear();
    for (int i=0; i<ciphercards.size(); ++i) {
      Card cipherCard = ciphercards[i];
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card cutCard = work.add(plaincards[i],cutPad);
      Card plainCard = work.sub(cipherCard,cipherPad);
      plaincards.push_back(plaincarCard);
      work.pseudoShuffle(cutCard);
    }
    return 0;
  }
  }


int main(int argc, char *argv[])
{
  Deck deck10(10);
  std::cout << deck10 << std::endl;

  Deck deck40(40);
  std::cout << deck40 << std::endl;

  Deck deck52(52);
  std::cout << deck52 << std::endl;

  Deck deck54(54);
  std::cout << deck54 << std::endl;

  return 0;
}
