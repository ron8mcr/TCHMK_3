all:
	swig -c++ -python bigInt.i
	g++ -fPIC -c bigInt.cpp
	g++ -fPIC -c bigInt_wrap.cxx -I/usr/include/python2.7
	g++ -shared bigInt.o bigInt_wrap.o -o _bigInt.so
clear:
	rm bigInt.o bigInt_wrap.cxx bigInt_wrap.o
