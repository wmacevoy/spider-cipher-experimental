#pragma once

#include <vector>
#include <set>

#include "card.h"
#include "deck.h"

namespace spider {
  struct SearchSetCmp {
    inline bool operator() (const Deck &a, const Deck &b) const {
      return equivalent(a,b) < 0;
    }
  };
  typedef std::set<Deck,SearchSetCmp> SearchSet;
  
  struct Search {
    int cards;
    int maxDist,dist,fdist,rdist;
    int duplicates;
    double growth;
    bool all;
    SearchSet forward,fboundary;
    SearchSet reverse,rboundary;
    std::vector < std::vector<Card> > paths;

    Deck from;
    Deck to;

    Search(const Deck &_from, const Deck &_to);
    bool done() const;
    bool found() const;
    void grow();
    void find();
    void growReverse();
    void growForward();
  };

  bool SearchLite(const Deck &from, int fromDist, 
		  const Deck &to, int toDist, 
		  std::vector<int> &path, bool cycle=false);

}
