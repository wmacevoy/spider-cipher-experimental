#pragma once

#include <vector>
#include <set>

#include "card.h"
#include "deck.h"

namespace spider {
  
  struct Search {
    int cards;
    int maxDist,dist,fdist,rdist;
    int duplicates;
    double growth;
    bool all;
    std::set<Deck> forward,fboundary;
    std::set<Deck> reverse,rboundary;
    std::vector < std::vector<Card> > paths;

    Deck from;
    Deck to;

    Search(const Deck &_from, const Deck &_to);
    bool done() const;
    void grow();
    void find();
    void growReverse();
    void growForward();
  };

}
