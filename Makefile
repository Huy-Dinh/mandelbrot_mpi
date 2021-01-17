# to execute the code, type:
# make
# mpirun -N 4 mandelbrot_a
# mpirun -N 4 mandelbrot_b
#

CC=mpicxx.mpich
CFLAGS= -c -g3 -Wall -O2 
LDFLAGS= -g3
RM=/bin/rm

SOURCES=lodepng.cc mandelbrot_a.cc mandelbrot_b.cc

EXECUTABLE=mandelbrot_a mandelbrot_b

OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(EXECUTABLE) complex.h

mandelbrot_a: lodepng.o mandelbrot_a.o
	$(CC) $(LDFLAGS) $^ -o $@

mandelbrot_b: lodepng.o mandelbrot_b.o
	$(CC) $(LDFLAGS) $^ -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf $(OBJECTS) $(EXECUTABLE) *.png *~ \#*
