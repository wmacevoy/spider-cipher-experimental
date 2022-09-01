#/usr/bin/env python3

# Note this is defined to be simple and correct, not fast or secure

import math
import secrets

#       0   1   2   3   4   5   6   7   8   9
FACES=['Q','A','2','3','4','5','6','7','8','9']

#        00's     10's     20's     30's
#       club     diamond  heart    spade
SUITS=["\u2663","\u2666","\u2665","\u2660"]

CARDS=len(FACES)*len(SUITS)

CUT_ZTH=0

TAG_ZTH=2
TAG_OFFSET=CARDS-1

PREFIX_LEN=10

def permute(p,a):
    return [a[p[i]] for i in range(len(p))]

def testPermute():
    p=[0,2,1]
    q=[2,0,1]
    pq=permute(p,q)
    assert pq==[2,1,0]

def inversePermutation(q):
    p = [0] * len(q)
    for i in range(len(q)):
        p[q[i]] = i
    return p

def testInversePermutation():
    p = [2,0,1,4,3]
    q = inversePermutation(p)

    assert permute(p,q) == [0,1,2,3,4]
    assert permute(q,p) == [0,1,2,3,4]

def identityPermutation(n=CARDS):
    return [i for i in range(n)]

def testIdentityPermutation():
    assert identityPermutation(3) == [0,1,2]

def randomPermutation(n=CARDS):
    p = identityPermutation(n)
    for i in range(n):
        j=i+secrets.randbelow(n-i)
        (p[i],p[j])=(p[j],p[i])
    return p

def testRandomPermutation():
    n = 100
    len = 4
    counts = [ [0]*len for i in range(len) ]
    for trial in range(n):
        r = randomPermutation(len)
        for i in range(len):
            counts[i][r[i]]=counts[i][r[i]]+1

    print(counts)
    p=1/len
    q=1-p
    mean=n*p
    stddev=math.sqrt(n*p*q)

    z = [ [(counts[i][j]-mean)/stddev for j in range(len) ] for i in range(len) ]

    for i in range(len):
        for j in range(len):
            assert abs(z[i][j]) < 6

def shiftPermutation(delta,n=CARDS):
    return [(delta+i) % n for i in range(n)]

def testShiftPermutation():
    assert shiftPermutation(0,3) == [0,1,2]
    assert shiftPermutation(1,3) == [1,2,0]
    assert shiftPermutation(2,3) == [2,0,1]

def backFrontShufflePermutation(n=CARDS):
    p=[]
    for i in range(n):
        if i % 2 == 0:
            p.append(i)
        else:
            p.insert(0,i)
    return p

def testBackFrontShufflePermutation():
    assert backFrontShufflePermutation(1)==[0]
    assert backFrontShufflePermutation(2)==[1,0]
    assert backFrontShufflePermutation(3)==[1,0,2]
    assert backFrontShufflePermutation(4)==[3,1,0,2]


CUT_PERMUTATIONS = [ shiftPermutation(at,CARDS) for at in range(CARDS) ]

def cutAt(deck,location):
    return permute(CUT_PERMUTATIONS[location],deck)

def cutOn(deck,card):
    return cutAt(deck,deck.index(card))

BACK_FRONT_SHUFFLE_PERMUTATION = backFrontShufflePermutation(CARDS)

def backFrontShuffle(deck):
    return permute(BACK_FRONT_SHUFFLE_PERMUTATION,deck)

def cutCard(deck,plainCard=0):
    return (deck[CUT_ZTH]+plainCard) % CARDS

def tagCard(deck):
    return (deck[TAG_ZTH]+TAG_OFFSET) % CARDS
    
def noiseCard(deck):
    tagCardLoc = deck.index(tagCard(deck))
    return deck[(tagCardLoc+1) % CARDS]

def encrypt(deck,plain):
    tag = tagCard(deck)
    noise = noiseCard(deck)

    scrambled = (plain + noise) % CARDS
        
    cut = cutCard(deck,plain)

    deck = cutOn(deck,tag)
    deck = backFrontShuffle(deck)
    deck = cutOn(deck,cut)
        
    return (deck,scrambled)
    
def decrypt(deck,scrambled):
    tag = tagCard(deck)
    noise = noiseCard(deck)

    plain = (scrambled + (CARDS-noise)) % CARDS
        
    cut = cutCard(deck,plain)

    deck = cutOn(deck,tag)
    deck = backFrontShuffle(deck)
    deck = cutOn(deck,cut)
        
    return (deck,plain)


UN_CUT_PERMUTATIONS = \
    [ inversePermutation(CUT_PERMUTATIONS[at]) for at in range(CARDS) ]

UN_BACK_FRONT_SHUFFLE_PERMUTATION = \
    inversePermutation(BACK_FRONT_SHUFFLE_PERMUTATION)

def undoEncrypt(deck,scrambled,plain=None):
    undos=[]
    noise = (scrambled + (CARDS-plain)) % CARDS if plain != None else None
    
    cut = deck[0]
    for cutAt in range(CARDS):
        undeck0=deck
        undeck0=permute(undeck0,UN_CUT_PERMUTATIONS[cutAt])
        undeck0=permute(undeck0,UN_BACK_FRONT_SHUFFLE_PERMUTATION)
        
        tag = deck[0]
        for tagAt in range(CARDS):
            undeck1=undeck0
            undeck1=permute(undeck1,UN_CUT_PERMUTATIONS[tagAt])

            if tagCard(undeck1) != tag: continue
            
            plain1 = (cut+(CARDS-undeck1[CUT_ZTH])) % CARDS
            cut1=cutCard(undeck1,plain1)
            noise1=noiseCard(undeck1)

            if plain != None:
                if cutCard(undeck1,plain) != cut: continue
                if noiseCard(undeck1) != noise: continue

            assert encrypt(undeck1,plain1) == scrambled

            undos.append((undeck1,plain1))

    return undos

def checkedEncrypt(deck,plainCards):
    enDeck=deck.copy()
    enDecks=[enDeck]
    scrambledCards=[]
    for plain in plainCards:
        (enDeck,scrambled)=encrypt(enDeck,plain)
        enDecks.append(enDeck)
        scrambledCards.append(scrambled)

    deDeck=deck.copy()
    deDecks=[deDeck]
    unscrambledCards=[]
    for scrambled in scrambledCards:
        (deDeck,unscrambled)=decrypt(deDeck,scrambled)
        deDecks.append(deDeck)
        unscrambledCards.append(unscrambled)

    assert enDecks == deDecks
    assert plainCards == unscrambledCards

    unDeck=enDeck
    for i in reversed(range(len(plainCards))):
        plain = None
        if i >= PREFIX_LEN and (i - PREFIX_LEN) % 2 == 0:
            plain = plainCards[i]
        undos=undoEncrypt(unDeck,plain)
        print(undos)
        print((enDecks[i],plainCards[i]))
        
        assert (enDecks[i],plainCards[i]) in undos
    return (enDeck,scrambledCards)

def randomMessage(len):
    return [secrets.randbelow(CARDS) for i in range(len)]

def makePacket(message):
    packet = range(PREFIX_LEN+2*len(message))
    for i in range(len(packet)):
        if (i >= PREFIX_LEN) and (i - PREFIX_LEN) % 2 == 0:
            packet[i]=message[(i-PREFIX_LEN) // 2]
        else:
            packet[i]=secrets.randbelow(CARDS)
    return packet

def randomDeck():
    return randomPermutation(CARDS)

def testRandomEncryptions():
    deck = [i for i in range(CARDS)]
    plainCards = [(CARDS-i) % CARDS for i in range(CARDS)]
    scramble = checkedEncrypt(deck,plainCards)
    print(scramble)
    
    for trial in range(100):
        deck = randomDeck()
        message = randomMessage(secrets.randbelow(100))
        plainCards = makePacket(message)
        scramble = checkedEncrypt(deck,plainCards)

def test():
    testPermute()
    testInversePermutation()
    testIdentityPermutation()        
    testRandomPermutation()
    testShiftPermutation()
    testBackFrontShufflePermutation()        

test()
