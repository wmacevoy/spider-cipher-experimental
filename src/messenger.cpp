#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>
#include <math.h>

#include "config.h"
#include "messenger.h"

namespace spider {

  const std::string Messenger::UP  ("ABCDEFGHIJKLMNOPQRSTUVWXYZ[]=~\t;!`\\\n");
  const std::string Messenger::UN  ("abcdefghijklmnopqrstuvwxyz<>{} :.\"\\\n");
  const std::string Messenger::DOWN("0123456789ABCDEF+-~@#$%^&|()*/_,?\'\\\n");

  int Messenger::modulus() const { return m_key.modulus(); }
  Card Messenger::addMod(const Card &a, const Card &b) const { return m_key.addMod(a,b); }
  Card Messenger::subMod(const Card &a, const Card &b) const { return m_key.subMod(a,b); }  
  Messenger::Messenger(RNG &rng,
	    int keyLen,
	    int prefixLen,
	    int minSuffixLen,
	    int mulLen)
    : m_rng(rng),
      m_key(keyLen),
      m_prefixLen(prefixLen),
      m_minSuffixLen(minSuffixLen),
      m_mulLen(mulLen) {
  }

  const Deck& Messenger::key() const { return m_key;}
  void Messenger::key(const Deck &value) { assert(value.cards.size() == m_key.cards.size()); m_key = value; }
  
  const std::string &Messenger::text() const { return m_text; }
  void Messenger::text(const std::string &value) { m_text = value; }

  const std::vector<Card>& Messenger::plaincards() const { return m_plaincards; }
  void Messenger::plaincards(const std::vector<Card> &value) { m_plaincards = value; }  
  const std::vector<Card>& Messenger::ciphercards() const { return m_ciphercards; }
  void Messenger::ciphercards(const std::vector<Card> &value) { m_ciphercards = value; }
  
  void Messenger::addPrefix() {
    for (int i=0; i< m_prefixLen; ++i) {
      int order = m_rng.next(0,modulus()-1);
      Card card(order);
      m_plaincards.push_back(card);
    }
  }

  void Messenger::addPrefix(const std::vector < Card > &cards) {
    m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
  }

  void Messenger::addSuffix() {
    for (int i=0; i<m_minSuffixLen; ++i) {
      Card card((i==0 && modulus() == 10) ? 0 : modulus()-1);
      m_plaincards.push_back(card);
    }
    while (m_plaincards.size() % m_mulLen != 0) {
      m_plaincards.push_back(Card(modulus()-1));
    }
  }

  bool Messenger::removeSuffix() {
    int suffixLen = 0;
    while (m_plaincards.size() > 0 && m_plaincards[m_plaincards.size()-1].order == modulus()-1) {
      m_plaincards.pop_back();
      ++suffixLen;
    }
    if (modulus() == 10 && m_plaincards.size() > 0 && m_plaincards[m_plaincards.size()-1].order == 0) {
      m_plaincards.pop_back();
      ++suffixLen;
    }
    return (suffixLen >= m_minSuffixLen);
  }

  void Messenger::addModPrefix() {
    for (int i=m_prefixLen; i<m_plaincards.size(); ++i) {
      m_plaincards[i] = addMod(m_plaincards[i],m_plaincards[i % m_prefixLen]);
    }
  }

  void Messenger::subModPrefix() {
    for (int i=m_prefixLen; i<m_plaincards.size(); ++i) {
      m_plaincards[i] = subMod(m_plaincards[i],m_plaincards[i % m_prefixLen]);
    }
  }

  void Messenger::encode() {
    if (m_plaincards.size() == 0) {
      addPrefix();
    }
    int i=0;
    while (i < m_text.size()) {
      int upLen = 0;
      while (i + upLen < m_text.size() && UP.find(m_text[i+upLen]) != std::string::npos) ++upLen;
      int unLen = 0;
      while (i + unLen < m_text.size() && UN.find(m_text[i+unLen]) != std::string::npos) ++unLen;
      int downLen = 0;
      // encode all unfound codes as octets
      while (i + downLen < m_text.size() && (DOWN.find(m_text[i+downLen]) != std::string::npos || (UN.find(m_text[i+downLen]) == std::string::npos && UP.find(m_text[i+downLen]) == std::string::npos))) {
	++downLen;
      }

      if (DEBUG >= 100) {
	std::cout << "upLen=" << upLen << ",unLen=" << unLen << ",downLen=" << downLen << std::endl;
      }

      int maxLen = std::max(std::max(downLen,upLen),unLen);

      if (DEBUG >= 100) {      
	std::cout << "maxLen=" << maxLen << std::endl;
      }
      
      if (unLen == maxLen) {
	std::vector<Card> cards;
	for (int j=0; j<unLen; ++j) {
	  cards.push_back(Card(UN.find(m_text[i])));
	  ++i;
	}
	if (DEBUG >= 100) {
	  std::cout << "un cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
      } else if (upLen == maxLen) {
	std::vector<Card> cards;	
	for (int j=0; j<upLen; ++j) {
	  cards.push_back(Card(UP.find(m_text[i])));
	  ++i;
	}
	m_plaincards.push_back(cards.size() > 1 ? Card::SHIFT_LOCK_UP : Card::SHIFT_UP);
	if (DEBUG >= 100) {
	  std::cout << "up cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
	if (cards.size() > 1 && i < m_plaincards.size()) {
	  m_plaincards.push_back(Card::SHIFT_LOCK_DOWN);
	}
      } else {
	std::vector<Card> cards;
	for (int j=0; j<downLen; ++j) {
	  uint8_t ch = m_text[i];
	  int order = DOWN.find(ch);
	  if (order >= 0) {
	    cards.push_back(Card(order));
	  } else {
	    uint8_t o2 = (ch >> 6) & 0x3;
	    uint8_t o1 = (ch >> 3) & 0x7;
	    uint8_t o0 = (ch >> 0) & 0x7;
	    cards.push_back(Card::BACKSLASH);
	    cards.push_back(Card(o2));
	    cards.push_back(Card(o1));
	    cards.push_back(Card(o0));
	  }
	  ++i;
	}
	m_plaincards.push_back(cards.size() > 1 ? Card::SHIFT_LOCK_DOWN : Card::SHIFT_DOWN);
	if (DEBUG >= 100) {
	  std::cout << "down cards=" << cards << std::endl;
	}
	m_plaincards.insert(m_plaincards.end(),cards.begin(),cards.end());
	if (cards.size() > 1 && i < m_plaincards.size()) {
	  m_plaincards.push_back(Card::SHIFT_LOCK_UP);
	}
      }
    }
  }

  void Messenger::decode() {
    int shift = 0;
    bool lock = true;
    m_text.clear();
    for (int i = m_prefixLen; i < m_plaincards.size(); ++i) {
      Card &card=m_plaincards[i];
      if (card == Card::SHIFT_LOCK_UP || card == Card::SHIFT_UP) {
	++shift;
	if (shift > 1) shift = 1;
	lock = (shift == 0 || card == Card::SHIFT_LOCK_UP);
	continue;
      } else if (card == Card::SHIFT_LOCK_DOWN || card == Card::SHIFT_DOWN) {
	--shift;
	if (shift < -1) shift = -1;
	lock = (shift == 0 || card == Card::SHIFT_LOCK_DOWN);
	continue;
      }
      if (shift == 0) {
	if (card.order < UN.length()) {
	  m_text.push_back(UN[card.order]);
	}
      } else if (shift == 1) {
	if (card.order < UP.length()) {
	  m_text.push_back(UP[card.order]);
	}
	if (!lock) {
	  shift = 0;
	  lock = true;
	}
      } else if (shift == -1) {
	if (card == Card::BACKSLASH && lock && i + 4 < m_plaincards.size() &&
	    m_plaincards[i+1].order < 8 && m_plaincards[i+2].order < 8 && m_plaincards[i+3].order < 8) {
	  uint8_t o2 = m_plaincards[i+1].order;
	  uint8_t o1 = m_plaincards[i+2].order;
	  uint8_t o0 = m_plaincards[i+3].order;
	  uint8_t b = (o2 << 6) | (o1 << 3) | o0;
	  m_text.push_back(b);
	} else {
	  if (card.order < DOWN.length()) {
	    m_text.push_back(DOWN[card.order]);
	  }
	  if (!lock) {
	    shift = 0;
	    lock = true;
	  }
	}
      }
    }
  }

  void Messenger::encrypt() {
    Deck work(m_key);
    for (int i=0; i<m_plaincards.size(); ++i) {
      Card plainCard = m_plaincards[i];
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card cutCard = work.addMod(plainCard,cutPad);
      Card cipherCard = work.addMod(m_plaincards[i],cipherPad);
      if (DEBUG >= 100) {
	std::cout << "cipher pad = " << cipherPad << ", cut pad = " << cutPad << ", plain = " << plainCard << ", cipher=" << cipherCard << std::endl;
      }
      m_ciphercards.push_back(cipherCard);
      work.pseudoShuffle(cutCard);
    }
  }
  
  void Messenger::decrypt() {
    Deck work(m_key);
    m_plaincards.clear();
    for (int i=0; i<m_ciphercards.size(); ++i) {
      Card cipherCard = m_ciphercards[i];
      Card cipherPad = work.cipherPad();
      Card cutPad = work.cutPad();
      Card plainCard = work.subMod(cipherCard,cipherPad);
      Card cutCard = work.addMod(plainCard,cutPad);
      if (DEBUG >= 100) {
	std::cout << "cipher pad = " << cipherPad << ", cut pad = " << cutPad << ", plain = " << plainCard << ", cipher=" << cipherCard << std::endl;
      }
      m_plaincards.push_back(plainCard);
      work.pseudoShuffle(cutCard);
    }
  }

  void Messenger::reset() {
    m_key.reset();
    m_text.clear();
    m_plaincards.clear();
    m_ciphercards.clear();
  }
}
