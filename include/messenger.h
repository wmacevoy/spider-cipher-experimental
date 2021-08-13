#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "rng.h"
#include "card.h"
#include "deck.h"

namespace spider {
struct Messenger {
  Deck m_key;
  RNG &m_rng;
  int m_prefixLen;
  int m_minSuffixLen;
  int m_mulLen;
  std::string m_text;
  std::vector<Card> m_plaincards;
  std::vector<Card> m_ciphercards;

  int modulus() const;
  Card addMod(const Card &a, const Card &b) const;
  Card subMod(const Card &a, const Card &b) const;
  
  Messenger(RNG &rng,
	    int keyLen,
	    int prefixLen,
	    int minSuffixLen,
	    int mulLen);

  const Deck& key() const;
  void key(const Deck &value);
  
  const std::string &text() const;
  void text(const std::string &value);

  const std::vector<Card>& plaincards() const;
  void plaincards(const std::vector<Card> &value);
  const std::vector<Card>& ciphercards() const;
  void ciphercards(const std::vector<Card> &value);
  
  void addPrefix();

  void addSuffix();

  bool removeSuffix();

  void addModPrefix();

  void subModPrefix();

  static const std::string UN;
  static const std::string DOWN;
  static const std::string UP;  

  void encode();

  void decode();

  void encrypt();
  
  void decrypt();

  void reset();
};
