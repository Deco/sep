

program: newParams.o main.o
	g++-4.8 -std=c++11 newParams.o main.o -o program 


newParams.o: newParams.cpp newParams.h threads.h
	g++-4.8 -std=c++11 -c newParams.cpp newParams.h threads.h

main.o: main.cpp newParams.h threads.h
	g++-4.8 -std=c++11 -c main.cpp newParams.h threads.h

clean:
	rm program *.o *.gch

