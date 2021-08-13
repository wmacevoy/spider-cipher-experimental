#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>

namespace spider {
  struct Card {
    const static int JOKER_SUITE = 4;
    static const char * const SUITES[];
    static const char * const FACES[];
    static const char * const JOKER_FACES[];  

    uint8_t order;
    Card();
    Card(const Card &card);
    Card(uint8_t _order);

    uint8_t suiteNumber() const;
    const char * suite() const;  
    uint8_t faceNumber() const;
    const char * face() const;

    std::string name() const;

    Card(uint8_t faceNumber, uint8_t suiteNumber);
    bool joker() const;

    bool operator<(const Card &to) const;
    bool operator<=(const Card &to) const;
    bool operator==(const Card &to) const;
    bool operator!=(const Card &to) const;

    bool operator>=(const Card &to) const;
    bool operator>(const Card &to) const;
    void operator=(const Card &card);
    static const Card SHIFT_LOCK_DOWN;
    static const Card SHIFT_LOCK_UP;
    static const Card SHIFT_DOWN;

    static const Card SHIFT_UP;
    static const Card NEWLINE;
    static const Card BACKSLASH;
  };

  std::ostream& operator<<(std::ostream &out, const Card &card);
  std::istream& operator>>(std::istream &in, Card &card);  
  std::ostream& operator<<(std::ostream &out, const std::vector<Card> &cards);
  Card addMod(const Card &a, const Card &b, int n);
  Card subMod(const Card &a, const Card &b, int n);
}
