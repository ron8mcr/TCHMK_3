#!/usr/bin/python

import bigInt
import funcs
import sys


if len(sys.argv) == 1 :
	funcs.noArguments()
	sys.exit(-1)
	
arguments = funcs.getArguments()

isRead, A, B, modulus = funcs.getFromFiles (arguments.A_filename, arguments.B_filename, arguments.mod_filename, arguments.b)
if not isRead:
	sys.exit(-1)
	
isProcess, res = funcs.process(A, B, modulus, arguments.operation)
if not isProcess:
	sys.exit(-1)
	
print "A = ", A
print "B = ", B
print "M = ", modulus
print "R = ", res

isWrite = funcs.saveRes(arguments.result_filename, arguments.b, res)
if not isWrite:
	sys.exit(-1)
