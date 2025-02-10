# MPI_Image_Reconstruction

## Table of Contents
- [Introduction](#introduction)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Implementation Details](#implementation-details)
- [How to Run](#how-to-run)
- [Output](#output)
- [Acknowledgments](#acknowledgments)

## Introduction
This project implements a parallel image reconstruction algorithm using MPI (Message Passing Interface). The goal is to reconstruct a blurred image using edge detection data provided in a PGM file.

The image reconstruction algorithm applies the following formula iteratively to compute pixel values:

$$ p_{new}[i][j] = 0.25 \times (p_{old}[i-1][j] + p_{old}[i+1][j] + p_{old}[i][j-1] + p_{old}[i][j+1] - p_{lim}[i][j]) $$

Each pixel's neighboring pixels are communicated between processes using MPI functions such as `MPI_Sendrecv()`.

## Project Structure
```
|-- main.c            # Main source code for MPI image processing
|-- pgm_IO.c          # Helper functions to read, write, and get dimensions of PGM files
|-- pgm_IO.h          # Header file for PGM utility functions
|-- image.pgm         # Example input image in PGM format
|-- output_xxx.pgm    # Output reconstructed images for various iteration counts
```

## Requirements
- C compiler with MPI support (e.g., `mpicc`)
- MPI library (e.g., OpenMPI)
- Linux environment for PGM image processing tools

## Implementation Details
### Parallelization Strategy
1. **Domain Decomposition**:
   The image is divided into horizontal slices, and each slice is assigned to a different process.
   
2. **Data Communication**:
   Each process exchanges pixel data with its neighboring processes using `MPI_Sendrecv()` to update the boundary regions (halo regions).

3. **Reconstruction Algorithm**:
   The reconstruction is performed iteratively. The number of iterations is specified by the user as a command-line argument.

4. **PGM File Handling**:
   - `pgm_size()`: Reads the dimensions of a PGM image.
   - `pgm_read()`: Loads a PGM image into a buffer.
   - `pgm_write()`: Writes a buffer to a PGM image file.

### Key MPI Functions Used:
- `MPI_Init()`
- `MPI_Comm_size()`
- `MPI_Comm_rank()`
- `MPI_Scatter()`
- `MPI_Sendrecv()`
- `MPI_Gather()`
- `MPI_Finalize()`

## How to Run
1. **Compile the Code**:
   ```sh
   mpicc -o mpi_image_processor main.c pgm_IO.c -lm
   ```

2. **Execute the Program**:
   ```sh
   mpirun -np <number_of_processes> ./mpi_image_processor <image.pgm> <num_iterations>
   ```
   Example:
   ```sh
   mpirun -np 4 ./mpi_image_processor image.pgm 1000
   ```

3. **Input Image**:
   Ensure that `image.pgm` is present in the same directory as the executable.

## Output
The output is a reconstructed image written to a file named `output_xxx.pgm`, where `xxx` is the number of iterations.

Example output:
```
output_1000.pgm
```

You can visualize the output using any PGM viewer or with the `display` command in a Linux terminal:
```sh
display output_1000.pgm
```

