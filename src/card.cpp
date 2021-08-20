#include <iostream>
#include <strstream>
#include <stdint.h>
#include <vector>
//#include <algorithm>

#include "card.h"

namespace spider {
  Card::Card() : order(0) {}
  Card::Card(const Card &card) : order(card.order) {}
  Card::Card(uint8_t _order) : order(_order) {}

  uint8_t Card::suiteNumber() const {
    if (order < 40) {
      return order / 10;
    } else if (order < 52) {
      return (order-40) / 3;
    } else {
      return JOKER_SUITE;
    }
  }

  const char * Card::suite() const {
    return SUITES[suiteNumber()];
  }
  
  uint8_t Card::faceNumber() const {
    if (order < 40) {
      return order % 10;
    } else if (order < 52) {
      return 10+((order-40) % 3);
    } else {
      return (order - 52);
    }
  }

  const char * Card::face() const {
    return joker() ? JOKER_FACES[faceNumber()] : FACES[faceNumber()];
  }

  Card::Card(uint8_t faceNumber, uint8_t suiteNumber) {
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

  std::string Card::name() const {
    std::string ans;
    ans += face();
    ans += suite();
    return ans;
  }

  bool Card::joker() const {
    return order >= 52;
  }

  bool Card::operator<(const Card &to) const {
    return order < to.order;
  }
  bool Card::operator<=(const Card &to) const {
    return order <= to.order;
  }
  bool Card::operator==(const Card &to) const {
    return order == to.order;
  }

  bool Card::operator!=(const Card &to) const {
    return order != to.order;
  }

  bool Card::operator>=(const Card &to) const {
    return order >= to.order;
  }
  bool Card::operator>(const Card &to) const {
    return order > to.order;
  }
  void Card::operator=(const Card &card) {
    order = card.order;
  }

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
    return out << card.face() << card.suite() << "(" << int(card.order) << ")";
  }

  std::istream& operator>>(std::istream &in, Card &card) {
    std::string name;
    int order;
    char ch;
    while (name.length() < 32 && in >> std::ws && in.peek() != '(' && in >> ch) {
      name.push_back(ch);
    }
    if (in >> std::ws && in.peek() == '(' && in >> ch >> order >> std::ws && in.peek() == ')' &&  in >> ch) {
      card.order = order;
      if (card.name() == name) {
	return in; // ok;
      }
    }
    in.setstate(std::ios::failbit);
    return in;
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

  std::istream& operator>>(std::istream &in, std::vector<Card> &cards) {
    Card card;
    char ch;
    cards.clear();
    if (in >> std::ws && in.peek() == '[' && in >> ch) {
      while (in >> std::ws && in.peek() != ']' && in >> card) {
	cards.push_back(card);
	if (in >> std::ws && in.peek() == ',') {
	  in >> ch;
	}
      }
      if (in >> std::ws && in.peek() == ']' && in >> ch) {
	return in; // ok
      }
    }
    in.setstate(std::ios::failbit);
    return in;
  }
}
