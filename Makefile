
all:
	make del
#	make isa_main
	make isa
#	make maketest
	make isa_test
	make clear

isa: isa.o
	g++ -o isa isa.o -L/usr/local/ssl/lib -lssl -lcrypto

isa.o: isa.cpp isa.h
	g++ -c isa.cpp

#testold:
#	./isa -f "ff.txt" -c ewewe -C ewewe -u

clear:
	rm -f *.o

del:
	rm -f *.so
	rm -f isa_test
	rm -f isa

#isa_test: isa_test.o isa.o
#	g++ -o isa_test isa_test.o isa.o -L/usr/local/ssl/lib -lssl -lcrypto

#isa_test.o: isa_test.cpp isa.h isa.cpp
#	g++ -c isa_test.cpp isa.h isa.cpp

#maketest: isa_test.cpp isa.h
#	g++ -c isa_test.cpp isa.h
#	g++ -o isa_test isa_test.o -L/usr/local/ssl/lib -lssl -lcrypto

test:
	make && clear ; LD_LIBRARY_PATH=. ./isa_test

#bruh: bruh.cpp
#	g++ -c bruh.cpp
#	g++ -o bruh bruh.o -lssl -lcrypto

libisa.so: isa.cpp isa.h
	g++ -o libisa.so -fpic -shared isa.cpp isa.h

isa_test: libisa.so isa_test.cpp
	g++ -c isa_test.cpp -o isa_test.o
	g++ -o isa_test isa_test.o -lisa -L. -L/usr/local/ssl/lib -lssl -lcrypto
