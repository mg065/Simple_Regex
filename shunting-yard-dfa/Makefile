CC=g++
FLAGS=-g

all:
	$(CC) -c $(FLAGS) regex.cpp -o regex.o
	$(CC) -c $(FLAGS) state.cpp -o state.o
	$(CC) -c $(FLAGS) stream.cpp -o stream.o
	$(CC) -c $(FLAGS) tree.cpp -o tree.o
	$(CC) -c $(FLAGS) main.cpp -o main.o
	$(CC) $(FLAGS) regex.o stream.o tree.o state.o main.o -o main
clean:
	rm *.o main
