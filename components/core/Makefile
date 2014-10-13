

program: newParams.o main.o
	g++ -std=c++11 newParams.o main.o -o program 


newParams.o: newParams.cpp newParams.h threads.h
	g++ -c -std=c++11 newParams.cpp newParams.h threads.h

main.o: main.cpp newParams.h threads.h
	g++ -c -std=c++11 main.cpp newParams.h threads.h

clean:
	rm program *.o *.gch

