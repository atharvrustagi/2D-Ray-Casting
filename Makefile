
all:
	g++ -Isrc/include -c main.cpp
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system

compilerun:
	g++ -Isrc/include -c main.cpp
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
	del *.o
	main.exe

compile:
	g++ -Isrc/include -c *.cpp
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
	del *.o

link:
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system 

clean:
	del *.o 

run:
	main.exe 
