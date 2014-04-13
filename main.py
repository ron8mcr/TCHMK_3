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
	
#print "A = ", A.getString()
#print "B = ", B.getString()
#print "M = ", modulus.getString()
#print "R = ", res.getString()

isWrite = funcs.saveRes(arguments.result_filename, arguments.b, res)
if not isWrite:
	sys.exit(-1)
