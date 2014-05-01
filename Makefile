all:
	swig -c++ -python bigInt.i
	g++ -fPIC -c bigInt.cpp
	g++ -fPIC -c bigInt_wrap.cxx -I/usr/include/python2.7
	g++ -shared bigInt.o bigInt_wrap.o -o _bigInt.so
	
windows32:
	swig -c++ -python bigInt.i
	g++ -c bigInt.cpp
	g++ -c bigInt_wrap.cxx -IC:\Python27\include
	g++ bigInt.o bigInt_wrap.o -Ic:\python27\include -Lc:\python27\libs -lpython27 -shared -o _bigInt.pyd
		
windows64:
	swig -c++ -python bigInt.i
	C:\TDM-GCC-64\bin\g++ -c bigInt.cpp
	C:\TDM-GCC-64\bin\g++ -c bigInt_wrap.cxx -IC:\Python27\include
	C:\TDM-GCC-64\bin\g++ bigInt.o bigInt_wrap.o -Ic:\python27\include -Lc:\python27\libs -lpython27 -shared -o _bigInt.pyd

clear:
	rm bigInt.o bigInt_wrap.cxx bigInt_wrap.o
	
clearWin:
	del bigInt.o bigInt_wrap.cxx bigInt_wrap.o