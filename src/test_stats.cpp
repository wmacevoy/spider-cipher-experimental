#include <math.h>
#include "gtest/gtest.h"

#include "rng.h"
#include "card.h"
#include "deck.h"

using namespace std;
using namespace spider;

template <typename X>
std::ostream &operator<<(std::ostream &out, const std::vector<X> &xs)
{
  out << "[";
  bool first = true;
  for (auto x : xs) {
    if (first) {
      first = false;
    } else {
      out << ",";
    }
    out << x;
  }
  out << "]";

  return out;
}


double z_luck(const std::vector<int> &counts) {
  int n=0;
  for (int i=0; i<counts.size(); ++i) {
    n += counts[i];
  }
  double p = 1.0/double(counts.size());
  double q = 1-p;  
  double mu = n*p;
  double inv_sigma = 1/sqrt(n*p);
  double z=0.0;

  for (int i=0; i<counts.size(); ++i) {
    z += pow(inv_sigma*(counts[i]-mu),2);
  }

  z = sqrt(z)-sqrt((counts.size()-1)-0.5);

  return z;
}

double z2_luck(const std::vector< std::vector<int> > &counts) {
  int n2=0;
  for (int i=0; i<counts.size(); ++i) {
    for (int j=0; j<counts[i].size(); ++j) {
      n2 += counts[i][j];
    }
  }
  double p = 1.0/pow(double(counts.size()),2);
  double q = 1-p;
  double mu = n2*p;
  double inv_sigma = 1/sqrt(n2*p);
  double z2=0;

  for (int i=0; i<counts.size(); ++i) {
    for (int j=0; j<counts[i].size(); ++j) {    
      z2 += pow(inv_sigma*(counts[i][j]-mu),2);
    }
  }

  z2 = sqrt(z2)-sqrt(pow(counts.size()-1,2)-0.5);

  return z2;
}

std::vector<DeckConfig> top40Configs() {
  std::vector<DeckConfig> cfgs;
  { Cfg cfg; cfg.cipherZth = 0; cfg.cipherOffset = -1; cfg.cutZth = 4; cfg.cutOffset = 37; cfgs.push_back(cfg); }
  { Cfg cfg; cfg.cipherZth = 1; cfg.cipherOffset = -1; cfg.cutZth = 3; cfg.cutOffset =  4; cfgs.push_back(cfg); }  
  { Cfg cfg; cfg.cipherZth = 2; cfg.cipherOffset = 38; cfg.cutZth = 5; cfg.cutOffset = 39; cfgs.push_back(cfg); }
  { Cfg cfg; cfg.cipherZth = 4; cfg.cipherOffset = 37; cfg.cutZth = 0; cfg.cutOffset =  4; cfgs.push_back(cfg); }  
}

std::vector<DeckConfig> configs() {
  std::vector<DeckConfig> cfgs;
  DeckConfig cfg;  
  for (auto cipherZth : {0,1,2,3,4,5}) {
    cfg.cipherZth = cipherZth;
    for (auto cipherOffset : {-1,0,1,2,3,4,5,35,36,37,38,39}) {
      cfg.cipherOffset=cipherOffset;
      for (auto cutZth : {0,1,2,3,4,5}) {
	cfg.cutZth = cutZth;
	for (auto cutOffset : {-1,0,1,2,3,4,5,35,36,37,38,39}) {
	  cfg.cutOffset=cutOffset;
	  cfgs.push_back(cfg);
	}
      }
    }
  }
  return cfgs;
}

struct Stats {
  double total;
  double sum;
  double sum2;
  double min;
  double max;
  void reset() {
    total = 0;
    sum  = 0;
    sum2 = 0;
    min = 0;
    max = 0;
  }

  Stats() {
    reset();
  }
  
  void add(double x, double w=1.0) {
    if (total == 0 || x < min) {
      min = x;
    }
    if (total == 0 || max < x) {
      max = x;
    }
    total += w;
    sum += w*x;
    sum2 += w*x*x;
  }

  double mean() const  {
    return sum / total;
  }
  double sd() const {
    return sqrt(sum2/total-pow(sum/total,2));
  }

};

struct DeckStats {
  std::ostream &out;
  RNG &rng;

  int n;
  int zTrials;
  int tTrials;

  int cipher0;
  int cipher1;
  int cut0;
  int cut1;

  int id;
  Deck deck;
  Stats zCipherStats;
  Stats zCipher2Stats;      
  Stats zCutStats;
  Stats zCut2Stats;
  Stats zXyStats;

  std::vector< int > ciphers;
  std::vector< std::vector < int > > ciphers2;
  std::vector< int > cuts;
  std::vector< std::vector < int > > cuts2;
  std::vector< std::vector < int > > xy;
  

  void reset() {
    id = 0;
    deck = Deck(n);
    deck.shuffle(rng);
    zCipherStats.reset();
    zCipher2Stats.reset();
    zCutStats.reset();
    zCut2Stats.reset();
    zXyStats.reset();

    ciphers = std::vector< int > ( n, 0 );
    ciphers2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
    cuts = std::vector< int > ( n, 0 );
    cuts2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
    xy = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );

    cipher0=0;
    cipher1=0;
    cut0=0;
    cut1=0;
  }
  
  DeckStats(RNG &rng = RNG::DEFAULT, std::ostream &_out = std::cout) : out(_out)  { reset(); }

  void outHeader() {
    out << "n,cipherZth,cipherOffset,cutZth,cutOffset,cipherMean,cipherSd,cipher2Mean,cipher2Sd,cutMean,cutSd,cut2Mean,cut2Sd,xyMean,xySd" <<  std::endl;
  }

  void outRow() {
    const DeckConfig &cfg = deck.config();
    out << n << "," << cfg.cipherZth << "," << cfg.cipherOffset << "," << cfg.cutZth << "," << cfg.cutOffset << "," << zCipherStats.mean() <<  "," << zCipherStats.sd() << "," << zCipher2Stats.mean() << "," << zCipher2Stats.sd() << "," << zCutStats.mean() << "," << zCutStats.sd() << "," << zCut2Stats.mean() << "," << zCut2Stats.sd() << "," << zXyStats.mean() << "," << zXyStats.sd() << std::endl;
  }

  void tTrial() {
    cipher1=deck.cipherPad().order;
    cut1=deck.cutPad().order;
    ++ciphers.at(cipher1);
    ++cuts.at(cut1);
    ++xy.at(cut1).at(cipher1);
    if (tTrial > 0) {
      ++ciphers2.at(cipher0).at(cipher1);
      ++cuts2.at(cut0).at(cut1);
    }
    cipher0=cipher1;
    cut0=cut1;
    
    deck.mix(deck.modulus()-1);
  }

  void zTrial() {
    ciphers = std::vector< int > ( n, 0 );
    ciphers2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
    cuts = std::vector< int > ( n, 0 );
    cuts2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
    xy = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );

    deck = Deck(n);
    deck.shuffle();
    
    cipher0=0;
    cipher1=0;
    cut0=0;
    cut1=0;
    for (int t=0; t<tTrials; ++t) {
      tTrial();
    }
    zCipherStats.add(z_luck(ciphers));
    zCipher2Stats.add(z2_luck(ciphers2));
    zCutStats.add(z_luck(cuts));
    zCut2Stats.add(z2_luck(cuts2));
    zXyStats.add(z2_luck(xy));
  }

  DeskConfig cfg;
  void row() {
    retain<const DeckConfig> as(&cfg);
    
    zCipherStats.reset();
    zCipher2Stats.reset();
    zCutStats.reset();
    zCut2Stats.reset();
    zXyStats.reset();
    
    for (int z=0; z<zTrials; ++z) {
      zTrial();
    }

    if (id == 0) {
      outHeader();
    }
    outRow();
    ++id;
  }
}

TEST(Stats,Opt) {
  std::vector < DeckConfig > cfgs = configs();
  std::vector < int > ns = {10, 40, 41, 52, 54};
  DeckStats stats;
  stats.zTrials = 24;
  stats.tTrials = 1000*1000;
  for (auto n : ns) {
    stats.n = n;
    for (auto cfg : cfgs) {
      stats.cfg = cfg;
      stats.row();
    }
  }
}

TEST(Stats,OptTop) {
  std::vector < DeckConfig > cfgs = top40Configs();
  std::vector < int > ns = {40};
  DeckStats stats;  
  int zTrials = 100;
  int tTrials = 1000*1000;
  for (auto n : ns) {
    stats.n = n;
    for (auto cfg : cfgs) {
      stats.cfg = cfg;
      stats.row();
    }
  }
}

TEST(Stats,OptTopMessage) {
  std::vector < DeckConfig > cfgs = top40Configs();
  std::vector < int > ns = {40};
  DeckStats stats;  
  int zTrials = 100;
  int tTrials = 1000*1000;
  for (auto n : ns) {
    stats.n = n;
    for (auto cfg : cfgs) {
      stats.cfg = cfg;
      stats.row();
    }
  }
}


TEST(Deck,Luck) {
  OS_RNG rng;

  int n = 100*1000*1000;

  int k = 40;

  std::vector < int > bins(k,0);
  std::vector < std::vector < int > > bins2(k, std::vector <int> (k, 0));

  for (int i=0; i<n; ++i) {
    int a = rng.next(0,k-1);
    int b = rng.next(0,k-1);
    int c = rng.next(0,k-1);    
    ++bins[a];
    ++bins2[a][b];
  }

  double z=z_luck(bins);

  double z2=z2_luck(bins2);

  std::cout << "z_luck(os_rand)=" << z << std::endl;
  std::cout << "z2_luck(os_rand)=" << z2 << std::endl;

  ASSERT_LT(fabs(z),8);
  ASSERT_LT(fabs(z2),8);  
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
