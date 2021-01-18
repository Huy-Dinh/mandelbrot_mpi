#include "complex.h"
#include "lodepng.h"
#include "mandelbrot.h"

#include <mpi.h>

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

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

void generateMandelbrotSubset(unsigned char *array, int const rank, int const size)
{
    int heightSlice = height / size;
    int yOffset = height * rank / size;
    for (int y = 0; y < heightSlice; y++)
    {
        int realY = y + yOffset;
        for (int x = 0; x < width; x++)
        {
            complex c(xZero + sx * (x - width / 2) / m, yZero + sy * (realY - height / 2) / m);
            int iter = mandelbrot(c, max_abs, max_iter);
            unsigned int col = make_color(max_iter, iter, rank, size);
            set_pixel(array, width * y + x, col);
        }
    }
}

int main(int argc, char **argv)
{
    /* Initialize MPI */
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc > 1)
        m = std::strtod(argv[1], 0);
    if (argc > 2)
        xZero = std::strtod(argv[2], 0);
    if (argc > 3)
        yZero = std::strtod(argv[3], 0);

    assert((width * height) % size == 0); // Number of pixels must be divisible by the number of processes

    /**
     * TODO parallelize the mandelbrot calculation across all MPI processes.
     * Each process should calculate an equal part of the image.
     * Process 1..N send their result via MPI_Send() to Process 0.
     * Process 0 collects the results via MPI_Recv() and encodes the resulting
     * image.
     * Note: you will need to change the code for rank 0.
     */
    int sliceSize = width * height * 4 / size;
    if (rank == 0)
    {
        generateMandelbrotSubset(image, rank, size);

        for (int i = 1; i < size; ++i)
        {
            MPI_Recv(&image[sliceSize * i], sliceSize, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        /* ... PNG encoding ... */
        unsigned error = lodepng_encode32_file("mandelbrot_a.png", image, width, height);
        if (error)
        {
            std::cout << "error " << error << " : " << lodepng_error_text(error) << std::endl;
        }
    }
    else
    {
        std::vector<unsigned char> subImage(width * height * 4 / size);
        generateMandelbrotSubset(&subImage[0], rank, size);
        MPI_Send(&subImage[0], sliceSize, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    /* Shutdown MPI, must be the last MPI call */
    MPI_Finalize();
    return 0;
}
