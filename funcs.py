import bigInt
import argparse

def noArguments():
	print "Launched without parameters."
	print "Perform all kind of operations with entered numbers."
	
	print "Enter A:",
	a = bigInt.bigInt(raw_input())
	print "Enter B:",
	b = bigInt.bigInt(raw_input())

	print "A + B =", a, "+", b, "=", a + b
	print "A - B =", a, "-", b, "=", a - b
	print "A * B =", a, "*", b, "=", a * b

	if b != bigInt.bigInt("0"):
		print "A / B =", a, "/", b, "=", a / b
		print "A % B =", a, "%", b, "=", a % b
	else:
		print "Division by zero"
	
	print "Enter modulus for pow: ",
	mod = bigInt.bigInt(raw_input())
	print "A ^ B mod N =", a, "^", b, "mod", mod,  "=", bigInt.pow(a, b, mod)

	print "That's all"
	
def getArguments ():
	parser = argparse.ArgumentParser(prog = "TCHMK_3")
	parser.add_argument ('A_filename', help = "filename of file with 1-st big integer")
	parser.add_argument ('operation', choices=['+', '-', '*', '/', '%', '^'], help = "operations")
	parser.add_argument ('B_filename', help = "filename of file with 2-nd big integer")
	parser.add_argument ('result_filename', help = "filename of result file")
	parser.add_argument ('-b', action='store_const', const=True,  default=False, help = "perform binary files")
	parser.add_argument ('mod_filename', help = "filename of file with modulus", nargs='?')
 	return parser.parse_args()
 
def getFromFiles (fileA, fileB, fileMod, binary):
	A = bigInt.bigInt()
	B = bigInt.bigInt()
	modulus = bigInt.bigInt()
	if (binary):
		if not A.getFromBinFile(fileA):
			print "Can't get number from:", fileA 
			return False, A, B, modulus
		if not B.getFromBinFile(fileB):
			print "Can't get number from:", fileB 
			return False, A, B, modulus
		if fileMod:
			if not modulus.getFromBinFile(fileMod):
				print "Can't get number from:", fileMod 
				return False, A, B, modulus
	else:
		if not A.getFromTextFile(fileA):
			print "Can't get number from:", fileA 
			return False, A, B, modulus
		if not B.getFromTextFile(fileB):
			print "Can't get number from:", fileB 
			return False, A, B, modulus
		if fileMod:
			if not modulus.getFromTextFile(fileMod):
				print "Can't get number from:", fileMod 
				return False, A, B, modulus
	return True, A, B, modulus
	
def process(A, B, modulus, operation):
	zero = bigInt.bigInt("0")
	res = zero
	
	if (modulus < zero):
		print "Negative modulus!"
		return False, res
		
	if operation == '^':
		res = bigInt.pow(A, B, modulus)
		return True, res
			
	if (modulus > zero):
		A %= modulus
		B %= modulus
	
	if operation == '+':
		res = A + B

	elif operation == '-':
		res = A - B

	elif operation == '*':
		res = A * B

	elif operation == '/':
		if B == zero:
			print "Division by zero"
			return False, res
		res = A / B

	elif operation == '%':
		if (B == zero):
			print "Division by zero"
			return False, res
		res = A % B
	
	if (modulus > zero):
			res %= modulus
			while (res < zero):
				res += modulus
				
	return True, res
			
def saveRes(fileRes, binary, res):
	if binary:
		if not res.saveToBinFile(fileRes):
			print "Can't save result to ", fileRes 
			return False
	else:
		if not res.saveToTextFile(fileRes):
			print "Can't save result to ", fileRes 
			return False
	return True
