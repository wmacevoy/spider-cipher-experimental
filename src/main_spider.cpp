struct Card {
  const uint8_t  JOKER_SUITE = 9;
  const uint8_t  JOKER_FACE = 99;
  uint8_t face;
  uint8_t suite;

  Card() : face(JOKER_FACE) , suite(JOKER_SUITE) {}

  Card(uint8_t order) {
    if (order < 40) {
      suite = i / 10;
      face = i % 10;
    } else {
      suite = JOKER_SUITE;
      face = JOKER_FACE;
    }
  }

  Card(uint8_t _face, uint8_t _suite) : face(_face) , suite(_suite) {}

  bool joker() const {
    returen suite == JOKER_SUITE && face == JOKER_FACE;
  }

  
  Card operator+(const Card &to) const {
    if (joker() || to.joker()) {
      throw std::invalid_argument();
    }
    return Card((suite+to.suite) % 4, (face+to.face) % 10);
  }

  void operator++(int post) {
    if (joker() || to.joker()) {
      throw std::invalid_argument();
    }
    if (++face == 10) {
      face = 0;
      suite = (suite + 1) % 4;
    }
  }
};

struct Deck {
  std::vector<Card> cards;
  void order() {
    for (int i=0; i<size; ++i) {
      cards[i]=Card(i);
    }
  }

  Deck(int size) : cards(size) {
    order();
  }
  
  int find(const Card &card) const {
    auto loc = std::find(std::begin(cards), std::end(cards), card);
    return (loc != std::end(cards)) ? loc-std::begin(cards) : -1;
  }

  void shuffle(const Card &with) {
    std::vector temp(cards.size());

    // find cut card
    int loc = find(width);

    // copy top of deck (including cut card) to bottom of deck (move to temp)
    std::copy(std::begin(cards), std::begin(cards)+(loc+1),
	      std::end(temp)-(loc+1));
    std::copy(std::begin(cards)+(loc+1),std::end(cards),std::begin(temp));

    // shuffle top and bottom back together
    auto e=std::rend(temp);
    auto b=std::begin(temp);
    auto j=std::begin(cards);
    
    for (std::size_t i=0; i<temp.size(); ++i) {
      if (i % 2 == 0) {
	*j=*e;
	++e;
      } else {
	*j=*b;
	++b;
      }
    }
  }
};

class Permutation {
  


};

  

int main(int argc, char *argv[])
{
  P(i,j) = 

}
