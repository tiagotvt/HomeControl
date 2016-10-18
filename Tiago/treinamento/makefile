CC=g++
CFLAGS=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

executable: gramateste.cpp
	$(CC) gramateste.cpp -o grama $(LIBS) $(CFLAGS)
