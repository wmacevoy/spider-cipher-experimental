#pragma once

#include <stdint.h>
#include <fstream>

namespace spider {

  struct RNG {
    static RNG &DEFAULT;
    virtual uint32_t next_u32() = 0;
  
    virtual uint32_t next_u32(uint32_t n);  
    int next(int a, int b);
    virtual ~RNG();
  };

  struct OS_RNG : RNG {
    std::ifstream urand;
    OS_RNG();
    ~OS_RNG();
    uint32_t next_u32();
  };

  struct TEST_RNG : RNG {
    uint32_t m_state;
    TEST_RNG(uint32_t state=0);
    virtual uint32_t next_u32();
    virtual uint32_t next_u32(uint32_t n);
  };
}
