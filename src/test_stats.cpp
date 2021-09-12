
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

  z2 = sqrt(z2)-sqrt(pow(counts.size(),2)-1.5);
  return z2;
}

std::vector<DeckConfig> top40Configs() {
  std::vector<DeckConfig> cfgs;
  { DeckConfig cfg; cfg.cipherZth = 5; cfg.cipherOffset = 35; cfg.cutZth = 1; cfg.cutOffset = 38; cfgs.push_back(cfg); }
  return cfgs;
}

std::vector<DeckConfig> configs() {
  std::vector<DeckConfig> cfgs;
  DeckConfig cfg;  
  for (auto cipherZth : {0,1,2,3,4,5}) {
    cfg.cipherZth = cipherZth;
    for (auto cipherOffset : {1,2,3,4,5,35,36,37,38,39}) {
      cfg.cipherOffset=cipherOffset;
      for (auto cutZth : {0,1,2,3,4,5}) {
	if (cutZth == cipherZth) continue;
	cfg.cutZth = cutZth;
	for (auto cutOffset : {-1,1,2,3,4,5,35,36,37,38,39}) {
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
  int n;
  RNG &rng;
  std::ostream &out;

  int zTrials;
  int tTrials;
  int messageLen;

  double trials;
  double trial;
  int cipher0;
  int cipher1;
  int cut0;
  int cut1;

  int id;
  Deck deck;
  DeckConfig cfg;
  
  Stats zCipherStats;
  Stats zCipher2Stats;      
  Stats zCutStats;
  Stats zCut2Stats;
  Stats zXyStats;

  bool progress;

  std::vector< int > ciphers;
  std::vector< std::vector < int > > ciphers2;
  std::vector< int > cuts;
  std::vector< std::vector < int > > cuts2;
  std::vector< std::vector < int > > xy;

  
  DeckStats(int _n = 40, RNG &_rng = RNG::DEFAULT, std::ostream &_out = std::cout) : n(_n), rng(_rng), out(_out), zTrials(10*10), tTrials(100*100), messageLen(-1), trials(double(zTrials)*double(tTrials)),trial(0.0),cipher0(0), cipher1(0), cut0(0), cut1(0), id(0), deck(n), cfg(Deck::config()), ciphers(n,0), ciphers2(n,std::vector(n,0)), cuts(n,0), cuts2(n,std::vector<int>(n,0)), xy(n,std::vector<int>(n,0)), progress(false) {}

  void outHeader() {
    out << "n,cipherZth,cipherOffset,cutZth,cutOffset,cipherMean,cipherSd,cipher2Mean,cipher2Sd,cutMean,cutSd,cut2Mean,cut2Sd,xyMean,xySd" <<  std::endl;
  }

  void outRow() {
    //    const DeckConfig &cfg = deck.config();
    out << n << "," << cfg.cipherZth << "," << cfg.cipherOffset << "," << cfg.cutZth << "," << cfg.cutOffset << "," << zCipherStats.mean() <<  "," << zCipherStats.sd() << "," << zCipher2Stats.mean() << "," << zCipher2Stats.sd() << "," << zCutStats.mean() << "," << zCutStats.sd() << "," << zCut2Stats.mean() << "," << zCut2Stats.sd() << "," << zXyStats.mean() << "," << zXyStats.sd() << std::endl;
  }

  void tTrial(int t) {
   
    if (t == 0) {
      ciphers = std::vector< int > ( n, 0 );
      ciphers2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
      cuts = std::vector< int > ( n, 0 );
      cuts2 = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );
      xy = std::vector< std::vector < int > > ( n, std::vector<int>(n,0) );

      deck = Deck(n);
      deck.shuffle(rng);

      cipher0=0;
      cipher1=0;
      cut0=0;
      cut1=0;
    }

    if (messageLen > 0 && t % messageLen == 0) {
      for (int i=0; i<10; ++i) {
	deck.mix(rng.next(0,deck.modulus()-1));
      }
    }

    static const std::string testMessage = "SPIDER SOLITAIRE ";
    int offset = (messageLen > 0) ? (t % messageLen) : t;
    deck.mix(rng.next(0,deck.modulus()-1));    
    deck.mix(Card(testMessage[offset % testMessage.length()]-'A'));

    cipher0=deck.cipherPad().order;
    cut0=deck.cutPad().order;
    ++ciphers.at(cipher0);
    ++cuts.at(cut0);
    ++xy.at(cut0).at(cipher0);
    if (t > 0 || (messageLen > 0 && t % messageLen > 0)) {
      ++ciphers2.at(cipher1).at(cipher0);
      ++cuts2.at(cut1).at(cut0);
    }
    cipher1=cipher0;
    cut1=cut0;
    ++trial;
  }

  void zTrial(int z) {
    for (int t=0; t<tTrials; ++t) {
      tTrial(t);
      if (progress) {
	if (floor(100*trial/trials) != floor(100*(trial-1)/trials)) {
	  out << trial << "/" << trials << "(" << floor(100*trial/trials) << "%)" << std::endl;
	}
      }
    }

    if (z == 0) {
      zCipherStats.reset();
      zCipher2Stats.reset();
      zCutStats.reset();
      zCut2Stats.reset();
      zXyStats.reset();
    }
    
    zCipherStats.add(z_luck(ciphers));
    zCipher2Stats.add(z2_luck(ciphers2));
    zCutStats.add(z_luck(cuts));
    zCut2Stats.add(z2_luck(cuts2));
    zXyStats.add(z2_luck(xy));


    if (progress) {
      if (floor(100*double(z)/double(zTrials)) != floor(100*double(z-1)/double(zTrials))) {
	out << "at z=" << z << " out of " << zTrials << ":" << std::endl;
	outHeader();
	outRow();
      }
    }
  }

  void row() {
    retain<const DeckConfig> as(&cfg);

    trials = double(zTrials)*double(tTrials);
    trial = 0.0;
    
    for (int z=0; z<zTrials; ++z) {
      zTrial(z);
    }

    if (id == 0) {
      outHeader();
    }
    outRow();
    ++id;
  }
};

TEST(Stats,Default) {
  DeckStats stats;
  stats.n = 40;
  stats.zTrials = 4*4;
  stats.tTrials = 1000*1000;
  stats.cfg = DeckConfig::DEFAULT;
  stats.progress = true;
  stats.row();
}

TEST(Stats,Opt) {
  std::vector < DeckConfig > cfgs = configs();
  std::vector < int > ns = {40};
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

TEST(Stats,OptTopMessage) {
  std::vector < DeckConfig > cfgs = top40Configs();
  std::vector < int > ns = {40};
  DeckStats stats;
  stats.zTrials = 100;
  stats.tTrials = 1000*1000;
  stats.messageLen = 1000;

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
