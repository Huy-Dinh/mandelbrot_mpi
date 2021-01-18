#include "lodepng.h"
#include "mandelbrot.h"

#include <mpi.h>

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>


constexpr int blockSizeBroadcastTag = 100;
constexpr int startingLineTag = 101;
constexpr unsigned char startingLineRequestBody = 's';
constexpr int calculatedResultTag = 102;

// Since this is known during compiled time anyway, make them constepxr
// to avoid needing to do heap allocation.
constexpr int width = 1024;
constexpr int height = 1024;
unsigned char image[width * height * 4];

// This is probably going to be optimized,
// but no reason to not be constexpr
constexpr double sx = 2. / width;
constexpr double sy = 2. / height;
constexpr double max_abs = 2.;
constexpr int max_iter = 350;

double m = 1.;
double xZero = -.5;
double yZero = .0;

void generateMandelbrotSubset(unsigned char *array, int const rank, int const blockSize, int const startingLine, int const size)
{
    for (int y = startingLine; y < startingLine + blockSize; y++)
    {
        for (int x = 0; x < width; x++)
        {
            complex c(xZero + sx * (x - width / 2) / m, yZero + sy * (y- height / 2) / m);
            int iter = mandelbrot(c, max_abs, max_iter);
            unsigned int col = make_color(max_iter, iter, rank, size);
            set_pixel(array, width * y + x, col);
        }
    }
}

inline int getStartingLine()
{
    int startingLine = 0;
    unsigned char requestLineBuf = startingLineRequestBody;
    MPI_Send(&requestLineBuf, 1, MPI_UNSIGNED_CHAR, 0, startingLineTag, MPI_COMM_WORLD);
    MPI_Recv(&startingLine, 1, MPI_INT, 0, startingLineTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return startingLine;
}

int main( int argc, char ** argv )
{
    /* Initialize MPI */
    int rank, size;
    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if ( argc > 1 ) m = std::strtod( argv[1], 0 );
    if ( argc > 2 ) xZero = std::strtod( argv[2], 0 );
    if ( argc > 3 ) yZero = std::strtod( argv[3], 0 );

    int blockSize;

    std::vector<unsigned char> image(height * width * 4, 0);
    std::vector<unsigned char> subImage(height * width * 4, 0);

    if (rank == 0) { // root process
        std::cout << "Please enter the blocksize: ";
        std::cin >> blockSize;

        assert(height % blockSize == 0); // ensure that image height is divisible by the blockSize

        int nextStartingLine = 0;
        /**
         * TODO root process
         * 1. Broadcast the block size to all processes
         * 2. Wait for a worker process to ask for a work item
         * 3. Answer with the line where the worker should start. 
         *    Use line = -1 to indicate that no more blocks are available.
         */
        // Broadcast the block size to all processes
        MPI_Bcast(&blockSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int stoppedProcessCount = 0;
        MPI_Status recvStatus;
        unsigned char requestLineBuf;
        // As long as there are still child processes running
        while (stoppedProcessCount < size - 1) {
            MPI_Recv(&requestLineBuf, 1, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, startingLineTag, MPI_COMM_WORLD, &recvStatus);
            if (requestLineBuf == startingLineRequestBody) {
                // If there's no more work to be done
                if (nextStartingLine == -1) {
                    MPI_Send(&nextStartingLine, 1, MPI_INT, recvStatus.MPI_SOURCE, startingLineTag, MPI_COMM_WORLD);
                    ++stoppedProcessCount;
                } else {
                    MPI_Send(&nextStartingLine, 1, MPI_INT, recvStatus.MPI_SOURCE, startingLineTag, MPI_COMM_WORLD);
                    nextStartingLine += blockSize;
                    if (nextStartingLine >= height) nextStartingLine = -1;
                }
            }
        }

    } else { // child processes
        /**
         * TODO child process
         * 1. Get block size from root process.
         * 2. Ask root process for a new work item.
         * 3. Receive start from root process.
         * 4. Calculate blockSize-many lines beginning at start.
         * 5. Get a new offset. Exit if offset = -1.
         */
        int blockSize = 0;
        MPI_Bcast(&blockSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int startingLine = getStartingLine();
        while (startingLine != -1) {
            generateMandelbrotSubset(&subImage[0], rank, blockSize, startingLine, size);
            startingLine = getStartingLine();
        }
    }

    /**
     * TODO everyone
     * 1. Collect the final image with MPI_Reduce
     * 2. Encode the image in process 0
     */
    MPI_Reduce(&subImage[0], &image[0], width * height * 4, MPI_UNSIGNED_CHAR, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        unsigned error = lodepng_encode32_file("mandelbrot_b.png", &image[0], width, height);
        if (error)
        {
            std::cout << "error " << error << " : " << lodepng_error_text(error) << std::endl;
        }
    }

    /* Shutdown MPI */
    MPI_Finalize();
    return 0;
}
