#include <math.h>
#include "search.h"


namespace spider {

  Search::Search(const Deck &_from, const Deck &_to) : from(_from), to(_to) {
    cards=from.cards.size();
    dist=0;
    maxDist = -1;
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
    SearchSet newBoundary;
    reverse.insert(rboundary.begin(), rboundary.end());

    for (auto deck : rboundary) {
      for (int order  = 0; order < cards; ++order) {
	Card card(order);
	Deck newDeck(deck);
	newDeck.unmix(card);
	if (fboundary.find(newDeck) != fboundary.end()) {
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
    ++dist;
  }

  void Search::growForward() {
    SearchSet newBoundary;
    forward.insert(fboundary.begin(), fboundary.end());
   
    for (auto deck : fboundary) {
      for (int order  = 0; order < cards; ++order) {
	Card card(order);
	Deck newDeck(deck);
	newDeck.mix(Card(card));
	if (rboundary.find(newDeck) != rboundary.end()) {
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
    ++dist;
  };

  bool Search::found() const {
    return paths.size() > 0;
  }

  bool SearchLite(const Deck &from, int fromDist, 
		  const Deck &to, int toDist, 
		  std::vector<int> &path, bool cycle) {

    if (from == to && !cycle) {
      return true;
    }

    int n=from.cards.size();

    if (fromDist > 0) {
      Deck step(from);
      Deck temp1(step);
      Deck temp2(step);      
      for (int i=0; i<n; ++i) {
	Deck::cut(step.cards,i,temp1.cards);
	Deck::backFrontShuffle(temp1.cards,temp2.cards);
	if (SearchLite(temp2, fromDist-1, to, toDist, path)) {
	  path.insert(path.begin(),i);
	  return true;
	}
      }
      return false;
    }
    
    if (toDist > 0) {
      Deck step(to);
      Deck temp1(step);
      Deck temp2(step);      
      for (int i=0; i<n; ++i) {
	Deck::backFrontUnshuffle(step.cards,temp1.cards);
	Deck::cut(temp1.cards,n-i,temp2.cards);
	if (SearchLite(from, fromDist, temp2, toDist-1, path)) {
	  path.push_back(i);
	  return true;
	}
      }
      return false;
    }

    return false;
  }
}


