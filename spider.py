#/usr/bin/env python3

# Note this is defined to be simple and correct, not fast or secure

import math
import secrets
from turtle import undobufferentries

#       0   1   2   3   4   5   6   7   8   9
FACES=['Q','A','2','3','4','5','6','7','8','9']

#        00's     10's     20's     30's
#       club     diamond  heart    spade
SUITS=["\u2663","\u2666","\u2665","\u2660"]

CARDS=len(FACES)*len(SUITS)

CUT_ZTH=0

TAG_ZTH=2
TAG_OFFSET=CARDS-1

PREFIX_LEN = 10

# permute q with p (this is how permutations are composed p circle q)
def permute(p,q):
    return [p[q[i]] for i in range(len(p))]

def testPermute():
# example from wikipedia with 0-base indexing
# https://en.wikipedia.org/wiki/Permutation_group
    p=[1,3,0,2,4]
    q=[4,3,2,1,0]
    qp=permute(q,p)
    assert qp==[3,1,4,2,0]

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

def randomPermutation(n=CARDS,rng=secrets.randbelow):
    p = identityPermutation(n)
    for i in range(n):
        j=i+rng(n-i)
        (p[i],p[j])=(p[j],p[i])
    return p

def testRandomPermutation():
    # counts[i][j] counts how often the random permutation r
    # has r[i]=j. This should be a binomial distribution.

    n = 100
    len = 4
    counts = [ [0]*len for i in range(len) ]
    for trial in range(n):
        r = randomPermutation(len)
        for i in range(len):
            counts[i][r[i]]=counts[i][r[i]]+1

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
    return permute(deck,CUT_PERMUTATIONS[location])

def testCutAt():
    deck = [ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,\
            19,18,17,16,15,14,13,12,11,10,\
            29,28,27,26,25,24,23,22,21,20,\
            39,38,37,36,35,34,33,32,31,30]

    at = 5
    result = cutAt(deck,at)

    expect = [ 4, 3, 2, 1, 0,19,18,17,16,15,\
              14,13,12,11,10,29,28,27,26,25,\
              24,23,22,21,20,39,38,37,36,35,\
              34,33,32,31,30, 9, 8, 7, 6, 5]

    assert expect == result

def uncutAt(deck,at):
    return cutAt(deck,(CARDS-at)%CARDS)

def testUncutAt():
    deck = [ 4, 3, 2, 1, 0,19,18,17,16,15,\
            14,13,12,11,10,29,28,27,26,25,\
            24,23,22,21,20,39,38,37,36,35,\
            34,33,32,31,30, 9, 8, 7, 6, 5]

    at = 5
    result = uncutAt(deck,at)

    expect = [ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,\
              19,18,17,16,15,14,13,12,11,10,\
              29,28,27,26,25,24,23,22,21,20,\
              39,38,37,36,35,34,33,32,31,30]

    assert expect == result


def cutOn(deck,card):
    return cutAt(deck,deck.index(card))

def testCutOn():
    deck = [ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,\
            19,18,17,16,15,14,13,12,11,10,\
            29,28,27,26,25,24,23,22,21,20,\
            39,38,37,36,35,34,33,32,31,30]

    card = 11
    result = cutOn(deck,card)

    expect = [11,10,29,28,27,26,25,24,23,22,\
              21,20,39,38,37,36,35,34,33,32,\
              31,30, 9, 8, 7, 6, 5, 4, 3, 2,\
               1, 0,19,18,17,16,15,14,13,12]

    assert expect == result

BACK_FRONT_SHUFFLE_PERMUTATION = backFrontShufflePermutation(CARDS)

def backFrontShuffle(deck):
    return permute(deck,BACK_FRONT_SHUFFLE_PERMUTATION)

def testBackFrontShuffle():
    deck = [ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,\
            19,18,17,16,15,14,13,12,11,10,\
            29,28,27,26,25,24,23,22,21,20,\
            39,38,37,36,35,34,33,32,31,30]

    result = backFrontShuffle(deck)

    expect = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
                9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]

    assert expect == result

UN_BACK_FRONT_SHUFFLE_PERMUTATION = inversePermutation(backFrontShufflePermutation(CARDS))
def unBackFrontShuffle(deck):
    return permute(deck,UN_BACK_FRONT_SHUFFLE_PERMUTATION)
def testUnBackFrontShuffle():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]

    result = unBackFrontShuffle(deck)

    expect=[ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,\
            19,18,17,16,15,14,13,12,11,10,\
            29,28,27,26,25,24,23,22,21,20,\
            39,38,37,36,35,34,33,32,31,30]

    assert expect == result

def cutCard(deck,plain):
    return (deck[CUT_ZTH]+plain) % CARDS

def testCutCard():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]
    plain = 5

    expect = 35
    result = cutCard(deck,plain)

    assert expect == expect

def tagCard(deck):
    return (deck[TAG_ZTH]+TAG_OFFSET) % CARDS
    
def testTagCard():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]

    expect = 33
    result = tagCard(deck)
    
    assert expect == result

def noiseCard(deck):
    tagCardLoc = deck.index(tagCard(deck))
    return deck[(tagCardLoc+1) % CARDS]

def testNoiseCard():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]

    expect = 31
    result = noiseCard(deck)
    assert expect == result

def encrypt(deck,plain):
    tag = tagCard(deck)
    noise = noiseCard(deck)

    scrambled = (plain + noise) % CARDS
        
    cut = cutCard(deck,plain)

    deck = cutOn(deck,tag)
    deck = backFrontShuffle(deck)
    deck = cutOn(deck,cut)
        
    return (deck,scrambled)
    
def testEncrypt():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]
    plain = 5

    expectDeck = [35,39,23,27,11,15,19, 3, 7, 8, 4, 0,16,12,28,24,20,36,32,31,\
                  33,30,34,38,22,26,10,14,18, 2, 6, 9, 5, 1,17,13,29,25,21,37]
    expectScramble = 36

    (resultDeck,resultScramble)=encrypt(deck,plain)
    assert expectDeck == resultDeck
    assert expectScramble == resultScramble
    
def decrypt(deck,scrambled):
    tag = tagCard(deck)
    noise = noiseCard(deck)

    plain = (scrambled + (CARDS-noise)) % CARDS
        
    cut = cutCard(deck,plain)

    deck = cutOn(deck,tag)
    deck = backFrontShuffle(deck)
    deck = cutOn(deck,cut)
        
    return (deck,plain)

def testDecrypt():
    deck = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]
    scramble = 36

    expectDeck = [35,39,23,27,11,15,19, 3, 7, 8, 4, 0,16,12,28,24,20,36,32,31,\
                  33,30,34,38,22,26,10,14,18, 2, 6, 9, 5, 1,17,13,29,25,21,37]
    expectPlain = 5

    (resultDeck,resultPlain)=decrypt(deck,scramble)
    assert expectDeck == resultDeck
    assert expectPlain == resultPlain

def undoEncryptSlow(deck,scrambled,plains):
    undos=[]
    for at1 in range(CARDS):
        for at0 in range(CARDS):
            undeck=uncutAt(unBackFrontShuffle(uncutAt(deck,at1)),at0)
            for plain in plains:
                if encrypt(undeck,plain) == (deck,scrambled):
                    undos.append((undeck,plain))
    return undos

def undoEncrypt(deck,scrambled,plains):
    undos=[]
    plain = plains[0] if len(plains) == 1 else None
    noise = (scrambled + (CARDS-plains[0])) % CARDS if len(plains) == 1 else None
    
    cut = deck[0]
    for at in range(CARDS):
        undeck0=unBackFrontShuffle(uncutAt(deck,at))
        
        tag0 = undeck0[0]
        noise0 = undeck0[1]
        if noise != None and noise != noise0: continue

        cardTagZth=(tag0+(CARDS-TAG_OFFSET)) % CARDS
        cardTagZthAt=undeck0.index(cardTagZth)
        delta = (cardTagZthAt+(CARDS-TAG_ZTH)) % CARDS

        undeck1 = cutAt(undeck0,delta)
        plain1 = (cut+(CARDS-undeck1[CUT_ZTH])) % CARDS
        if plain != None and plain1 != plain: continue
        cut1=cutCard(undeck1,plain1)
        assert cut == cutCard(undeck1,plain1)
        assert tag0 == tagCard(undeck1)
        if noise != None:
            assert noise == noiseCard(undeck1)
        assert encrypt(undeck1,plain1) == (deck,scrambled)
        undos.append((undeck1,plain1))

    return undos

def testUndoEncryptSlow():
    deck = [35,39,23,27,11,15,19, 3, 7, 8, 4, 0,16,12,28,24,20,36,32,31,\
                  33,30,34,38,22,26,10,14,18, 2, 6, 9, 5, 1,17,13,29,25,21,37]

    undo = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]
    plains = [5]
    scramble = 36

    undos=undoEncryptSlow(deck,scramble,plains)
    assert (undo,plains[0]) in undos

    undos=undoEncryptSlow(deck,scramble,range(CARDS))
    assert (undo,plains[0]) in undos

def testUndoEncrypt():
    deck = [35,39,23,27,11,15,19, 3, 7, 8, 4, 0,16,12,28,24,20,36,32,31,\
                  33,30,34,38,22,26,10,14,18, 2, 6, 9, 5, 1,17,13,29,25,21,37]

    undo = [ 30,32,34,36,38,20,22,24,26,28,10,12,14,16,18, 0, 2, 4, 6, 8,\
              9, 7, 5, 3, 1,19,17,15,13,11,29,27,25,23,21,39,37,35,33,31 ]
    plain = 5
    scramble = 36

    assert undoEncrypt(deck,scramble,[plain]) == undoEncryptSlow(deck,scramble,[plain])
    assert undoEncrypt(deck,scramble,[i for i in range(CARDS)]) == undoEncryptSlow(deck,scramble,[i for i in range(CARDS)])

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
    any = range(CARDS)
    for i in reversed(range(len(plainCards))):
        plains = any
        if i >= PREFIX_LEN and (i - PREFIX_LEN) % 2 == 0:
            plains = [plainCards[i]]
        undos=undoEncrypt(enDecks[i+1],scrambledCards[i],plains)
        
        assert (enDecks[i],plainCards[i]) in undos
    return (enDeck,scrambledCards)

def randomMessage(len,rng=secrets.randbelow):
    return [rng(CARDS) for i in range(len)]

def makePacket(message,rng=secrets.randbelow):
    packet = [0]*(PREFIX_LEN+2*len(message))
    for i in range(len(packet)):
        if (i >= PREFIX_LEN) and (i - PREFIX_LEN) % 2 == 0:
            packet[i]=message[(i-PREFIX_LEN) // 2]
        else:
            packet[i]=rng(CARDS)
    return packet

def randomDeck(rng=secrets.randbelow):
    return randomPermutation(CARDS,rng)

def skipTestRandomEncryptions():
    deck = [i for i in range(CARDS)]
    plainCards = [(CARDS-i) % CARDS for i in range(CARDS)]
    scramble = checkedEncrypt(deck,plainCards)
    print(scramble)
    
    for trial in range(100):
        deck = randomDeck()
        message = randomMessage(secrets.randbelow(100))
        plainCards = makePacket(message)
        scramble = checkedEncrypt(deck,plainCards)

