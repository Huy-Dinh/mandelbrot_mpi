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
    int rank, size;
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if ( argc > 1 ) m = std::strtod( argv[1], 0 );
    if ( argc > 2 ) x0 = std::strtod( argv[2], 0 );
    if ( argc > 3 ) y0 = std::strtod( argv[3], 0 );

    int blockSize;
    unsigned char* image = (unsigned char*) calloc(width * height * 4, sizeof(unsigned char));

    if (rank == 0) { // root process
        std::cout << "Please enter the blocksize: ";
        std::cin >> blockSize;

        assert(height % blockSize == 0); // ensure that image height is divisible by the blockSize

        /**
         * TODO root process
         * 1. Broadcast the block size to all processes
         * 2. Wait for a worker process to ask for a work item
         * 3. Answer with the line where the worker should start. 
         *    Use line = -1 to indicate that no more blocks are available.
         */

    } else { // child processes 

        /**
         * TODO child process
         * 1. Get block size from root process.
         * 2. Ask root process for a new work item.
         * 3. Receive start from root process.
         * 4. Calculate blockSize-many lines beginning at start.
         * 5. Get a new offset. Exit if offset = -1.
         */

    }

    /**
     * TODO everyone
     * 1. Collect the final image with MPI_Reduce
     * 2. Encode the image in process 0
     */

    free(image);
    /* Shutdown MPI */
    MPI_Finalize();
    return 0;
}
