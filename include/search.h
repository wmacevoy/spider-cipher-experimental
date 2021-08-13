#pragma once

#include <vector>
#include <set>

#include "card.h"
#include "deck.h"

namespace spider {
  
struct Search {
  int cards;
  int dist;
  int found;
  bool unique;
  std::set<Deck> forward,fboundary;
  std::set<Deck> reverse,rboundary;
  std::vector<Card> path;
  Deck from;
  Deck to;

  Search(const Deck &_from, const Deck &_to);

  void grow();

  void find();

  void growReverse();

  void growForward();
};

}
