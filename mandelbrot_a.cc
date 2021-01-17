#include "complex.h"
#include "lodepng.h"
#include "mandelbrot.h"

#include <mpi.h>

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

int main( int argc, char ** argv )
{
    int width = 1024, height = 1024;
    double sx = 2./width;
    double sy = 2./height;
    double m = 1.;
    double x0 = -.5, y0 = .0;
    double max_abs = 2.;
    int max_iter = 350;

    /* Initialize MPI */
    int rank , size ;
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if ( argc > 1 ) m = std::strtod( argv[1], 0 );
    if ( argc > 2 ) x0 = std::strtod( argv[2], 0 );
    if ( argc > 3 ) y0 = std::strtod( argv[3], 0 );

    assert((width * height) % size == 0); // Number of pixels must be divisible by the number of processes

    /**
     * TODO parallelize the mandelbrot calculation across all MPI processes.
     * Each process should calculate an equal part of the image.
     * Process 1..N send their result via MPI_Send() to Process 0.
     * Process 0 collects the results via MPI_Recv() and encodes the resulting
     * image.
     * Note: you will need to change the code for rank 0.
     */
    if (rank == 0) {
        unsigned char* image = (unsigned char*) malloc(width * height * 4);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                complex c( x0 + sx * (x - width/2) / m, y0 + sy * (y - height/2) / m );
                int iter = mandelbrot( c, max_abs, max_iter );
                unsigned int col = make_color(max_iter, iter, rank, size);
                set_pixel(image, width * y + x, col);
            }
        }

        /* ... PNG encoding ... */
        unsigned error = lodepng_encode32_file("mandelbrot_a.png", image, width, height);
        if (error) {
            std::cout << "error " << error << " : " << lodepng_error_text(error) << std::endl;
        }

        free(image);
    }

    /* Shutdown MPI, must be the last MPI call */
    MPI_Finalize();
    return 0;
}
