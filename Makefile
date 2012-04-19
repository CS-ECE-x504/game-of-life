CC = gcc
CFLAGS = -Wall -Werror -Wno-unused-result -O2
LDLIBS = -pthread
OBJECTS = gameoflife.o gol_serial.o gol_parallel.o 

all: gameoflife

gameoflife.o: gameoflife.c gameoflife.h

gol_serial.o: gol_serial.c gol_serial.h

gol_parallel.o: gol_parallel.c gol_parallel.h


gameoflife: $(OBJECTS) gol_cuda.cu gol_parallel.h
	nvcc -o gameoflife $(OBJECTS) gol_cuda.cu

clean:
	rm -f *.o *~ gameoflife
