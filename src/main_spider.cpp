#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

const int DEBUG = 0;

struct RNG {
  static RNG &DEFAULT;
  virtual uint32_t next_u32() = 0;
  
// 
// Debiased Modulo (Once) — Java's Method
// https://www.pcg-random.org/posts/bounded-rands.html
//
  virtual uint32_t next_u32(uint32_t n) {
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
  static const Card SHIFT_LOCK_DOWN;
  static const Card SHIFT_LOCK_UP;
  static const Card SHIFT_DOWN;
  static const Card SHIFT_UP;
  static const Card NEWLINE;
  static const Card BACKSLASH;
};

const char * const Card::SUITES[] = { "C","D","H","S","J" };
const char * const Card::FACES[] = { "10","A","2","3","4","5","6","7","8","9", "J", "Q", "K" };
const char * const Card::JOKER_FACES[] = { "A","B","C","D"};

const Card Card::SHIFT_LOCK_DOWN(39);
const Card Card::SHIFT_LOCK_UP(38);
const Card Card::SHIFT_DOWN(37);
const Card Card::SHIFT_UP(36);
const Card Card::NEWLINE(35);
const Card Card::BACKSLASH(34);

std::ostream& operator<<(std::ostream &out, const Card &card) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%02d(%s%s)",card.order,card.face(),card.suite());
  return out << buffer;
}

Card addMod(const Card &a, const Card &b, int n) {
  return Card((a.order + b.order) % n);
}


Card subMod(const Card &a, const Card &b, int n) {
  return Card((a.order + (n-b.order)) % n);
}

std::ostream& operator<<(std::ostream &out, const std::vector<Card> &cards) {
  out << "[";
  for (size_t i=0; i<cards.size(); ++i) {
    if (i > 0) out << ",";
    out << cards[i];
  }
  out << "]";
  
  return out;
}

struct Deck {
  std::vector<Card> cards;
  int modulus() const { return cards.size() == 10 ? 10 : 40; }
  void reset() {
    sort(cards.begin(),cards.end());
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

  Card addMod(const Card &a, const Card &b) const {
    return ::addMod(a,b,modulus());
  }

  Card subMod(const Card &a, const Card &b) const {
    return ::subMod(a,b,modulus());
  }

  const Card &cipherPad() const {
    return after(addMod(first(), Card(modulus() == 10 ? 1 : 11)));
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
};

std::ostream& operator<<(std::ostream &out, const Deck &deck) {
  return out << deck.cards;
}

struct Messenger {
  Deck m_key;
  RNG &m_rng;
  int m_prefixLen;
  int m_minSuffixLen;
  int m_mulLen;
  std::string m_text;
  std::vector<Card> m_plaincards;
  std::vector<Card> m_ciphercards;

  int modulus() const { return m_key.modulus(); }
  Card addMod(const Card &a, const Card &b) const { return m_key.addMod(a,b); }
  Card subMod(const Card &a, const Card &b) const { return m_key.subMod(a,b); }  
  
  Messenger(RNG &rng,
	    int keyLen,
	    int prefixLen,
	    int minSuffixLen,
	    int mulLen)
    : m_rng(rng),
      m_key(keyLen),
      m_prefixLen(prefixLen),
      m_minSuffixLen(minSuffixLen),
      m_mulLen(mulLen) {
  }

  const Deck& key() const { return m_key;}
  void key(const Deck &value) { assert(value.cards.size() == m_key.cards.size()); m_key = value; }
  
  const std::string &text() const { return m_text; }
  void text(const std::string &value) { m_text = value; }

  const std::vector<Card>& plaincards() const { return m_plaincards; }
  void plaincards(const std::vector<Card> &value) { m_plaincards = value; }  
  const std::vector<Card>& ciphercards() const { return m_ciphercards; }
  void ciphercards(const std::vector<Card> &value) { m_ciphercards = value; }
  
  void addPrefix() {
    for (int i=0; i< m_prefixLen; ++i) {
      int order = m_rng.next(0,modulus()-1);
      Card card(order);
      m_plaincards.push_back(card);
    }
  }

  void addSuffix() {
    for (int i=0; i<m_minSuffixLen; ++i) {
      Card card((i==0 && modulus() == 10) ? 0 : modulus()-1);
      m_plaincards.push_back(card);
    }
    while (m_plaincards.size() % m_mulLen != 0) {
      m_plaincards.push_back(Card(modulus()-1));
    }
  }

  bool removeSuffix() {
    int suffixLen = 0;
    while (m_plaincards.size() > 0 && m_plaincards[m_plaincards.size()-1].order == modulus()-1) {
      m_plaincards.pop_back();
      ++suffixLen;
    }
    if (modulus() == 10 && m_plaincards.size() > 0 && m_plaincards[m_plaincards.size()-1].order == 0) {
      m_plaincards.pop_back();
      ++suffixLen;
    }
    return (suffixLen >= m_minSuffixLen);
  }

  void addModPrefix() {
    for (int i=m_prefixLen; i<m_plaincards.size(); ++i) {
      m_plaincards[i] = addMod(m_plaincards[i],m_plaincards[i % m_prefixLen]);
    }
  }

  void subModPrefix() {
    for (int i=m_prefixLen; i<m_plaincards.size(); ++i) {
      m_plaincards[i] = subMod(m_plaincards[i],m_plaincards[i % m_prefixLen]);
    }
  }

  static const std::string UN;
  static const std::string DOWN;
  static const std::string UP;  

  void encode() {
    if (m_plaincards.size() == 0) {
      addPrefix();
    }
    int i=0;
    while (i < m_text.size()) {
      int upLen = 0;
      while (i + upLen < m_text.size() && UP.find(m_text[i+upLen]) != std::string::npos) ++upLen;
      int unLen = 0;
      while (i + unLen < m_text.size() && UN.find(m_text[i+unLen]) != std::string::npos) ++unLen;
      int downLen = 0;
      // encode all unfound codes as octets
      while (i + downLen < m_text.size() && (DOWN.find(m_text[i+downLen]) != std::string::npos || (UN.find(m_text[i+downLen]) == std::string::npos && UP.find(m_text[i+downLen]) == std::string::npos))) {
	++downLen;
      }

      if (DEBUG >= 100) {
	std::cout << "upLen=" << upLen << ",unLen=" << unLen << ",downLen=" << downLen << std::endl;
      }

      int maxLen = std::max(std::max(downLen,upLen),unLen);

      if (DEBUG >= 100) {      
	std::cout << "maxLen=" << maxLen << std::endl;
      }
      
      if (unLen == maxLen) {
	std::vector<Card> cards;
	for (int j=0; j<unLen; ++j) {
	  cards.push_back(Card(UN.find(m_text[i])));
	  ++i;
	}
	if (DEBUG >= 100) {
	  std::cout << "un cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
      } else if (upLen == maxLen) {
	std::vector<Card> cards;	
	for (int j=0; j<upLen; ++j) {
	  cards.push_back(Card(UP.find(m_text[i])));
	  ++i;
	}
	m_plaincards.push_back(cards.size() > 1 ? Card::SHIFT_LOCK_UP : Card::SHIFT_UP);
	if (DEBUG >= 100) {
	  std::cout << "up cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
	if (cards.size() > 1 && i < m_plaincards.size()) {
	  m_plaincards.push_back(Card::SHIFT_LOCK_DOWN);
	}
      } else {
	std::vector<Card> cards;
	for (int j=0; j<downLen; ++j) {
	  uint8_t ch = m_text[i];
	  int order = DOWN.find(ch);
	  if (order >= 0) {
	    cards.push_back(Card(order));
	  } else {
	    uint8_t o2 = (ch >> 6) & 0x3;
	    uint8_t o1 = (ch >> 3) & 0x7;
	    uint8_t o0 = (ch >> 0) & 0x7;
	    cards.push_back(Card::BACKSLASH);
	    cards.push_back(Card(o2));
	    cards.push_back(Card(o1));
	    cards.push_back(Card(o0));
	  }
	  ++i;
	}
	m_plaincards.push_back(cards.size() > 1 ? Card::SHIFT_LOCK_DOWN : Card::SHIFT_DOWN);
	if (DEBUG >= 100) {
	  std::cout << "down cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
	if (cards.size() > 1 && i < m_plaincards.size()) {
	  m_plaincards.push_back(Card::SHIFT_LOCK_UP);
	}
      }
    }
  }

  void decode() {
    int shift = 0;
    bool lock = true;
    m_text.clear();
    for (int i = m_prefixLen; i < m_plaincards.size(); ++i) {
      Card &card=m_plaincards[i];
      if (card == Card::SHIFT_LOCK_UP || card == Card::SHIFT_UP) {
	++shift;
	if (shift > 1) shift = 1;
	lock = (shift == 0 || card == Card::SHIFT_LOCK_UP);
	continue;
      } else if (card == Card::SHIFT_LOCK_DOWN || card == Card::SHIFT_DOWN) {
	--shift;
	if (shift < -1) shift = -1;
	lock = (shift == 0 || card == Card::SHIFT_LOCK_DOWN);
	continue;
      }
      if (shift == 0) {
	if (card.order < UN.length()) {
	  m_text.push_back(UN[card.order]);
	}
      } else if (shift == 1) {
	if (card.order < UP.length()) {
	  m_text.push_back(UP[card.order]);
	}
	if (!lock) {
	  shift = 0;
	  lock = true;
	}
      } else if (shift == -1) {
	if (card == Card::BACKSLASH && lock && i + 4 < m_plaincards.size() &&
	    m_plaincards[i+1].order < 8 && m_plaincards[i+2].order < 8 && m_plaincards[i+3].order < 8) {
	  uint8_t o2 = m_plaincards[i+1].order;
	  uint8_t o1 = m_plaincards[i+2].order;
	  uint8_t o0 = m_plaincards[i+3].order;
	  uint8_t b = (o2 << 6) | (o1 << 3) | o0;
	  m_text.push_back(b);
	} else {
	  if (card.order < DOWN.length()) {
	    m_text.push_back(DOWN[card.order]);
	  }
	  if (!lock) {
	    shift = 0;
	    lock = true;
	  }
	}
      }
    }
  }
  
  void encrypt() {
    Deck work(m_key);
    for (int i=0; i<m_plaincards.size(); ++i) {
      Card plainCard = m_plaincards[i];
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card cutCard = work.addMod(plainCard,cutPad);
      Card cipherCard = work.addMod(m_plaincards[i],cipherPad);
      if (DEBUG >= 100) {
	std::cout << "cipher pad = " << cipherPad << ", cut pad = " << cutPad << ", plain = " << plainCard << ", cipher=" << cipherCard << std::endl;
      }
      m_ciphercards.push_back(cipherCard);
      work.pseudoShuffle(cutCard);
    }
  }
  
  void decrypt() {
    Deck work(m_key);
    m_plaincards.clear();
    for (int i=0; i<m_ciphercards.size(); ++i) {
      Card cipherCard = m_ciphercards[i];
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card plainCard = work.subMod(cipherCard,cipherPad);
      Card cutCard = work.addMod(plainCard,cutPad);
      if (DEBUG >= 100) {
	std::cout << "cipher pad = " << cipherPad << ", cut pad = " << cutPad << ", plain = " << plainCard << ", cipher=" << cipherCard << std::endl;
      }
      m_plaincards.push_back(plainCard);
      work.pseudoShuffle(cutCard);
    }
  }

  void reset() {
    m_key.reset();
    m_text.clear();
    m_plaincards.clear();
    m_ciphercards.clear();
  }
};

const std::string Messenger::UP  ("ABCDEFGHIJKLMNOPQRSTUVWXYZ[]=~-;!`\\\n");
const std::string Messenger::UN  ("abcdefghijklmnopqrstuvwxyz<>{} :.\"\\\n");
const std::string Messenger::DOWN("0123456789ABCDEF+-~@#$%^&|()*/_,?\'\\\n");

struct TEST_RNG : RNG {
  uint32_t m_state;
  TEST_RNG(uint32_t state=0) : m_state(state) { }
  virtual uint32_t next_u32() {
    ++m_state;
    return m_state;
  }
  virtual uint32_t next_u32(uint32_t n) {
    ++m_state;
    return m_state % n;
  }
};

int main(int argc, char *argv[])
{
  std::cout << "SHIFT_LOCK_DOWN: " << Card::SHIFT_LOCK_DOWN << std::endl;
  std::cout << "SHIFT_LOCK_UP: " << Card::SHIFT_LOCK_UP << std::endl;
  std::cout << "SHIFT_DOWN: " << Card::SHIFT_DOWN << std::endl;
  std::cout << "SHIFT_UP: " << Card::SHIFT_UP << std::endl;
  Deck deck10(10);
  std::cout << deck10 << std::endl;

  Deck deck40(40);
  std::cout << deck40 << std::endl;

  Deck deck52(52);
  std::cout << deck52 << std::endl;

  Deck deck54(54);
  std::cout << deck54 << std::endl;

  assert(deck10.cipherPad() == Card(2));
  assert(deck10.cutPad() == Card(1));
  assert(deck40.cipherPad() == Card(12));
  assert(deck40.cutPad() == Card(1));

  TEST_RNG rng;

  int cards = 40;
  int prefixLen = (cards == 10) ? 4 : 10;
  int minSuffixLen = (cards == 10) ? 2 : 5;
  int modLen = (cards == 10) ? 4 : 10;

  std::string text = "Hello World! <in color!>";

  
  Messenger encrypter(rng, cards, prefixLen, minSuffixLen, modLen);
  Messenger decrypter(rng, cards, prefixLen, minSuffixLen, modLen);
  Deck key = (cards == 10) ? deck10 : deck40;

  encrypter.reset();
  encrypter.key(key);
  std::cout << "en key: " << encrypter.key() << std::endl;
  encrypter.text(text);
  std::cout << "en text: " << encrypter.text() << std::endl;
  encrypter.addPrefix();
  std::cout << "en prefix: " << encrypter.plaincards() << std::endl;  
  encrypter.encode();
  std::cout << "en prefix/plaincards: " << encrypter.plaincards() << std::endl;
  encrypter.addSuffix();
  std::cout << "en prefix/plaincards/suffix: " << encrypter.plaincards() << std::endl;  
  encrypter.addModPrefix();
  std::cout << "en prefix/mod[plaincards/suffix]: " << encrypter.plaincards() << std::endl;
  encrypter.encrypt();
  std::cout << "en ciphercards: " << encrypter.ciphercards() << std::endl;

  std::cout << "for '" << encrypter.text() << "\', send " << encrypter.ciphercards() << std::endl;

  decrypter.reset();
  decrypter.key(key);
  std::cout << "de key: " << decrypter.key() << std::endl;  
  decrypter.ciphercards(encrypter.ciphercards());
  std::cout << "de ciphercards: " << decrypter.ciphercards() << std::endl;
  decrypter.decrypt();
  std::cout << "de prefix/mod[plaincards/suffix]: " << decrypter.plaincards() << std::endl;
  decrypter.subModPrefix();
  std::cout << "de prefix/plaincards/suffix: " << decrypter.plaincards() << std::endl;

  bool ok = decrypter.removeSuffix();
  std::cout << "de prefix/plaincards: " << decrypter.plaincards() << " (ok: " << ok << ")" << std::endl;
  decrypter.decode();
  std::cout << "de text: " << decrypter.text() << " (ok: " << ok << ")" << std::endl;

  return 0;  

}
