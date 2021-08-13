#include "rng.h"

namespace spider {
  
  uint32_t RNG::next_u32(uint32_t n) {
    uint32_t x, r;
    do {
      x = next_u32();
      r = x % n;
    } while (x - r > uint32_t(-n));
    return r;
  }
  
  int RNG::next(int a, int b) {
    return a + int((b>a) ? next_u32(b-a+1) : 0);
  }
  
  RNG::~RNG() {};

  OS_RNG::OS_RNG() {
    urand.open("/dev/urandom", std::ios::binary);
  }
  OS_RNG::~OS_RNG() {
  }

  uint32_t OS_RNG::next_u32() {
    uint32_t x;
    urand.read((char*) &x,sizeof(x));
    return x;
  }

  OS_RNG DEFAULT_OS_RNG;

  RNG& RNG::DEFAULT(DEFAULT_OS_RNG);

  TEST_RNG::TEST_RNG(uint32_t state) : m_state(state) { }
  uint32_t TEST_RNG::next_u32() {
    ++m_state;
    return m_state;
  }
  uint32_t TEST_RNG::next_u32(uint32_t n) {
    ++m_state;
    return m_state % n;
  }

}
