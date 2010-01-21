from __future__ import division
from decimal import *

getcontext().prec = 100

def make_table(bits, num):
    table = []  

    for i in range(0,num):
        for j in range(0,100):
            if Decimal(2**j) / 10**i > 1:
                break
        else:
            raise RuntimeError, v
        j -= 1
        v = Decimal(2**j) / 10**i
        x = long(v * (2**bits))
        table.append((x,j))

    print "-------------------"
    for (x,j) in table:
        print (x >> j) / 2**bits

    print "-------------------"
    for (x,j) in table:
        print "%s, %d," % (hex(x),j)


make_table(32, 10)
make_table(64, 20)
