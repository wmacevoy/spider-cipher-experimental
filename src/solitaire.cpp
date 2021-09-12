#include "solitaire.h"

#include <algorithm>
#include <cassert>

namespace spider {

  const int Solitaire::N = 54;
  const int Solitaire::JOKER_A = 52;
  const int Solitaire::JOKER_B = 53;    
  
  Solitaire::Solitaire() : cards(N) {
    for (int i=0; i<N; ++i) { cards[i]=i; }
  }
  
  void Solitaire::swap(int i, int j) {
    int tmp=cards[i];
    cards[i]=cards[j];
    cards[j]=tmp;
  }
  
  int Solitaire::find(int card) const {
    auto i = std::find(std::begin(cards),std::end(cards),card);
    assert(i != std::end(cards));
    return i-std::begin(cards);
  }
  
  void Solitaire::shuffle(RNG &rng) {
    for (int i=0; i<N; ++i) {
      int j=rng.next(i,N-1);
      swap(i,j);
    }
  }

  void Solitaire::down1(int card) {
    int n = find(card);
    if (n <  N-1) {
      swap(n,n+1);
    } else {
      cards.erase(std::begin(cards)+n);
      cards.insert(std::begin(cards)+1,card);
    }
  }

  int Solitaire::pad() const {
    int c = cards[0]+1;
    if (c > N-1) {
      c = N-1;
    }
    c=cards[c];
    if (c > N-2) {
      c = N-2;
    }
    return c;
  }
  
  bool Solitaire::valid() const {
    return pad() < JOKER_A;
  }

  void Solitaire::tripleCut() {
    int ja=find(JOKER_A);
    int jb=find(JOKER_B);
    int jmin=(ja < jb) ? ja : jb;
    int jmax=(ja < jb) ? jb : ja;
    std::vector<int> tmp(cards);

    int i=0;
    
    // copy end (after jmax)
    int e=jmax+1;
    while (e<N) {
      cards[i]=tmp[e];
      ++i, ++e;
    }
      
    // copy middle next (jmin <= m <= jmax)
    int m=jmin;
    while (m <= jmax) {
      cards[i]=tmp[m];
      ++i, ++m;
    }
      
    // copy begin (until jmin)
    int b=0;
    while (b < jmin) {
      cards[i]=tmp[b];
      ++i, ++b;
    }
  }

  void Solitaire::countCut(int count) {
    std::vector<int> tmp(cards);
    int c = (count < N-1) ? count : N-1;
    int j=N-1;
    for (int i=c-1; i>=0; --i) {
      cards[--j]=tmp[i];
    }
    for (int i=N-2; i>=c; --i) {
      cards[--j]=tmp[i];
    }
  }
  
  void Solitaire::next() {
    down1(JOKER_A);
    down1(JOKER_B);
    down1(JOKER_B);    
    tripleCut();
    countCut(cards[N-1]+1);    
  }
}
