CC=g++
CFLAGS=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

executable: grama.cpp
	$(CC) grama.cpp -o grama $(LIBS) $(CFLAGS)