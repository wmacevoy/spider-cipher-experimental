#include <iostream>
#include <math.h>
#include "gtest/gtest.h"
#include "rng.h"

using namespace std;
using namespace spider;

TEST(TEST_RNG,Stats) {
  TEST_RNG rng;

  for (int i=1; i<100; ++i) {
    ASSERT_EQ(rng.next_u32(),i);
  }
}

TEST(OS_RNG,Stats) {
  int n = 1000000;
  int counts[10][10];
  double z[10][10];
  OS_RNG rng;
  for (int j=1; j<10; ++j) {
    for (int k=0; k<10; ++k) {
      counts[j][k]=0;
    }
  }

  for (int j=1; j<10; ++j) {
    for (int i=0; i<n; ++i) {
      ++counts[j][rng.next(0,j)];
    }
  }

  for (int j=1; j<10; ++j) {
    double p = 1.0/double(j+1);
    double q = (j)/double(j+1);
    
    for (int k=0; k<j; ++k) {
      z[j][k]=(counts[j][k]-n*p)/sqrt(n*p*q);
    }
  }

  for (int j=1; j<10; ++j) {
    for (int k=0; k<j; ++k) {
      ASSERT_LT(fabs(z[j][k]),4.0);
    }
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
