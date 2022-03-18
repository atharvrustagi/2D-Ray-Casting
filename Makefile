
all:
	g++ -Isrc/include -c main.cpp
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system

crun3d:
	g++ -Isrc/include -c 3D.cpp
	g++ 3D.o -o 3D -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
	del *.o
	3D.exe

crun2d:
	g++ -Isrc/include -c 2D.cpp
	g++ 2D.o -o 2D -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
	del *.o
	2D.exe

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
