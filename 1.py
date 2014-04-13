#!/usr/bin/python

import bigInt
a = bigInt.bigInt("1111111111111111111111111111111111111111")
b = bigInt.bigInt("11111111111111111111111111111111111111112")
c = a % b
print c.getString()
