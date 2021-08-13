#include "search.h"

namespace spider {
  
  Search::Search(const Deck &_from, const Deck &_to) : from(_from), to(_to) {
    cards=from.cards.size();
    dist=0;
    found=0;
    unique = false;
    forward.clear();
    reverse.clear();
    fboundary.clear();
    rboundary.clear();
    fboundary.insert(from);
    rboundary.insert(to);
  }

  void Search::grow() {
    if (dist % 2 == 0) {
      growForward();
    } else {
      growReverse();
    }
    ++dist;
  }

  void Search::find() {
    do {
      std::cout << "searching dist = " << dist << "..." << std::endl;
      grow();
      std::cout << "searching dist = " << dist << " found " << found << "." << std::endl;
    } while (found == 0);
  }

  void Search::growReverse() {
    std::set<Deck> newBoundary;
    reverse.insert(rboundary.begin(), rboundary.end());

    for (auto deck : rboundary) {
      for (int order  = 0; order < cards; ++order) {
	Card card(order);
	Deck newDeck(deck);
	newDeck.unmix(card);
	if (fboundary.find(newDeck) != fboundary.end()) {
	  ++found;
	  if (newDeck != from) {
	    Search recSearch(from,newDeck);
	    recSearch.find();
	    path.insert(path.begin(),recSearch.path.begin(),recSearch.path.end());
	  }
	  path.push_back(Card(card));
	  if (deck != to) {
	    Search recSearch(deck,to);
	    recSearch.find();
	    path.insert(path.end(),recSearch.path.begin(),recSearch.path.end());
	  }
	  if (!unique) {
	    return;
	  }
	} else {
	  newBoundary.insert(newDeck);
	}
      }
    }
    rboundary.swap(newBoundary);
  }

  void Search::growForward() {
    std::set<Deck> newBoundary;
    forward.insert(fboundary.begin(), fboundary.end());
   
    for (auto deck : fboundary) {
      for (int card  = 0; card < cards; ++card) {
	Deck newDeck(deck);
	newDeck.mix(Card(card));
	if (rboundary.find(newDeck) != rboundary.end()) {
	  ++found;
	  if (deck != from) {
	    Search recSearch(from,deck);
	    recSearch.find();
	    path.insert(path.begin(),recSearch.path.begin(),recSearch.path.end());
	  }
	  path.push_back(Card(card));
	  if (newDeck != to) {
	    Search recSearch(newDeck,to);
	    recSearch.find();
	    path.insert(path.end(),recSearch.path.begin(),recSearch.path.end());
	  }
	  if (!unique) {
	    return;
	  }
	} else {
	  newBoundary.insert(newDeck);
	}
      }
    }
    fboundary.swap(newBoundary);
  };
}

