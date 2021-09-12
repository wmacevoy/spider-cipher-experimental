#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include "messenger.h"

using namespace std;
using namespace spider;

bool beginsWith(const std::string &str, const std::string &prefix) {
  return str.compare(0, prefix.size(), prefix) == 0;
}

struct MessengerBuilder {
  std::shared_ptr<RNG> rng;
  Deck key;
  int prefixLen;
  int minSuffixLen;
  int mulLen;
};

int main(int argc, char *argv[])
{
  std::cout << "SHIFT_LOCK_DOWN: " << Card::SHIFT_LOCK_DOWN << std::endl;
  std::cout << "SHIFT_LOCK_UP: " << Card::SHIFT_LOCK_UP << std::endl;
  std::cout << "SHIFT_DOWN: " << Card::SHIFT_DOWN << std::endl;
  std::cout << "SHIFT_UP: " << Card::SHIFT_UP << std::endl;
  Deck deck10(10);
  std::cout << deck10 << std::endl;

  Deck deck40(40);
  std::cout << deck40 << std::endl;

  Deck deck52(52);
  std::cout << deck52 << std::endl;

  Deck deck54(54);
  std::cout << deck54 << std::endl;

  assert(deck10.cipherPad() == Card(2));
  assert(deck10.cutPad() == Card(1));
  assert(deck40.cipherPad() == Card(12));
  assert(deck40.cutPad() == Card(1));

  TEST_RNG rng;

  int cards = 40;
  int prefixLen = (cards == 10) ? 4 : 10;
  int minSuffixLen = (cards == 10) ? 2 : 5;
  int modLen = (cards == 10) ? 4 : 10;

  std::string text = "Hello World! <in color!>";

  
  Messenger encrypter(rng, cards, prefixLen, minSuffixLen, modLen);
  Messenger decrypter(rng, cards, prefixLen, minSuffixLen, modLen);
  Deck key = (cards == 10) ? deck10 : deck40;

  encrypter.reset();
  encrypter.key(key);
  std::cout << "en key: " << encrypter.key() << std::endl;
  encrypter.text(text);
  std::cout << "en text: " << encrypter.text() << std::endl;
  encrypter.addPrefix();
  std::cout << "en prefix: " << encrypter.plaincards() << std::endl;  
  encrypter.encode();
  std::cout << "en prefix/plaincards: " << encrypter.plaincards() << std::endl;
  encrypter.addSuffix();
  std::cout << "en prefix/plaincards/suffix: " << encrypter.plaincards() << std::endl;  
  encrypter.addModPrefix();
  std::cout << "en prefix/mod[plaincards/suffix]: " << encrypter.plaincards() << std::endl;
  encrypter.encrypt();
  std::cout << "en ciphercards: " << encrypter.ciphercards() << std::endl;

  std::cout << "for '" << encrypter.text() << "\', send " << encrypter.ciphercards() << std::endl;

  decrypter.reset();
  decrypter.key(key);
  std::cout << "de key: " << decrypter.key() << std::endl;  
  decrypter.ciphercards(encrypter.ciphercards());
  std::cout << "de ciphercards: " << decrypter.ciphercards() << std::endl;
  decrypter.decrypt();
  std::cout << "de prefix/mod[plaincards/suffix]: " << decrypter.plaincards() << std::endl;
  decrypter.subModPrefix();
  std::cout << "de prefix/plaincards/suffix: " << decrypter.plaincards() << std::endl;

  bool ok = decrypter.removeSuffix();
  std::cout << "de prefix/plaincards: " << decrypter.plaincards() << " (ok: " << ok << ")" << std::endl;
  decrypter.decode();
  std::cout << "de text: " << decrypter.text() << " (ok: " << ok << ")" << std::endl;

  return 0;  

}
