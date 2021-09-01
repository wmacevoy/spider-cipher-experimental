#pragma once
#include <vector>
#include "rng.h"
#include "deck.h"

namespace spider {
  struct Solitaire {
    static const int N;
    static const int JOKER_A;
    static const int JOKER_B;    
    
    std::vector <int > cards;
    void swap(int i, int j);
    int find(int card) const;
    Solitaire();
    void shuffle(RNG &rng);
    int pad() const;
    bool valid() const;
    void down1(int card);
    void tripleCut();
    void countCut(int count);
    void next();
  };
}

