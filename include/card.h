#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>

// Playing cards, with an unusual order.
//
// The 10,A,2,..,9 face cards serve as single digits 0,..,9
// The suits serve as the "tens" place (alphabetical order)
//
//  Clubs => +0
//  Diamonds => +10
//  Hearts => +20
//  Spades => +30
//
//  0- 9: 10C AC 2C 3C 4C 5C 6C 7C 8C 9C
// 10-19: 10D AD 2D 3D 4D 5D 6D 7D 8D 9D
// 20-29: 10H AH 2H 3H 4H 5H 6H 7H 8H 9H
// 30-39: 10S AS 2S 3S 4S 5S 6S 7S 8S 9S
//
// The next groups the Jack, Queen and Kings in alphabetical order of suite.
//
// 40-42:  JC QC KC
// 43-45:  JD QD KD
// 46-48:  JH QH KH
// 49-51:  JS QS KS
//
// Finally the jokers, labeled "a" and "b" for uniqueness.
//
// 52-53:  "A" Joker, "B" Joker
//
//
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


    // 0 for Clubs, 1 for Diamonds, 2 Hearts, 3 Spades, 4 for Jokers
    uint8_t suiteNumber() const;

    // "C", "D", "H", "S", "J"
    const char * suite() const;

    // 0 for 10, 1 for A, 2, .. 9, for 2..9, J is 10, Q is 11, K is 12.
    // note the unusual order for the 10 and J, Q, K.  We ignore the
    // face cards.
    uint8_t faceNumber() const;

    // "10","A","2",...,"9","J","Q","K"
    const char * face() const;

    // "10C", etc.
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

    // Cards representing useful encodings.
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
  std::istream& operator>>(std::istream &in, std::vector<Card> &cards);  
  Card addMod(const Card &a, const Card &b, int n);
  Card subMod(const Card &a, const Card &b, int n);
}
