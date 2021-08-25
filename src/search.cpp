#include <math.h>
#include "search.h"


namespace spider {
  Search::Search(const Deck &_from, const Deck &_to) : from(_from), to(_to) {
    cards=from.cards.size();
    dist=0;
    maxDist = -1;
    fdist=0;
    rdist=0;
    duplicates=0;
    growth=0;
    all = false;
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

  bool Search::done() const {
    if (!all && paths.size() > 0) return true;
    if (maxDist >= 0 && dist >= maxDist) return true;
    return false;
  }

  void Search::find() {
    while (!done()) {
      grow();
    }
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
	  std::cout << "path reverse containing " << card << " (rdist=" << rdist << ") found." << std::endl;
	  paths.push_back(std::vector<Card>());
	  std::vector<Card> &path = paths[paths.size()-1];
	  if (newDeck != from) {
	    Search recSearch(from,newDeck);
	    recSearch.find();
	    std::vector<Card> &toHere = recSearch.paths[0];
	    path.insert(path.begin(),toHere.begin(),toHere.end());
	  }
	  path.push_back(card);
	  if (deck != to) {
	    Search recSearch(deck,to);
	    recSearch.find();
	    std::vector<Card> &fromHere = recSearch.paths[0];
	    path.insert(path.end(),fromHere.begin(),fromHere.end());
	  }
	  if (!all) {
	    return;
	  }
	} else if (reverse.find(newDeck) == reverse.end()) {
	  newBoundary.insert(newDeck);
	} else {
	  ++duplicates;
	}
      }
    }
    growth = double(newBoundary.size())/double(rboundary.size());
    rboundary.swap(newBoundary);
    ++rdist;
  }

  void Search::growForward() {
    std::set<Deck> newBoundary;
    forward.insert(fboundary.begin(), fboundary.end());
   
    for (auto deck : fboundary) {
      for (int order  = 0; order < cards; ++order) {
	Card card(order);
	Deck newDeck(deck);
	newDeck.mix(Card(card));
	if (rboundary.find(newDeck) != rboundary.end()) {
	  std::cout << "path forward containing " << card << " (fdist=" << fdist << ") found." << std::endl;
	  paths.push_back(std::vector<Card>());
	  std::vector<Card> &path=paths[paths.size()-1];
	  if (deck != from) {
	    Search recSearch(from,deck);
	    recSearch.find();
	    std::vector<Card> &toHere=recSearch.paths[0];
	    path.insert(path.begin(),toHere.begin(),toHere.end());
	  }
	  path.push_back(card);
	  if (newDeck != to) {
	    Search recSearch(newDeck,to);
	    recSearch.find();
	    std::vector<Card> &fromHere=recSearch.paths[0];
	    path.insert(path.end(),fromHere.begin(),fromHere.end());
	  }
	  if (!all) {
	    return;
	  }
	} else if (forward.find(newDeck) == forward.end()) {
	  newBoundary.insert(newDeck);
	} else {
	  ++duplicates;
	}
      }
    }
    growth = double(newBoundary.size())/double(fboundary.size());
    fboundary.swap(newBoundary);
    ++fdist;
  };
}

