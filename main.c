#include "pgm_IO.h"
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void initialize_arrays(float *data, float *prev_data, float *new_data, float *boundary_data, int local_width, int local_height)
{
    // Initialize all arrays to 255.0:
    for (int i = 0; i < (local_width + 2) * (local_height + 2); ++i)
    {
        prev_data[i] = 255.0;
        new_data[i] = 255.0;
        boundary_data[i] = 255.0;
    }

    // Copy data into the boundary_data array, considering appropriate offsets:
    for (int i = 1; i <= local_width; ++i)
    {
        for (int j = 1; j <= local_height; ++j)
        {
            // Calculate index in boundary_data (handles padding/boundaries)
            int boundary_index = i * (local_height + 2) + j;

            // Calculate index in the original 'data' array
            int data_index = (i - 1) * local_height + (j - 1);

            boundary_data[boundary_index] = data[data_index];
        }
    }
}

void update_boundaries(float *prev_data, int local_width, int local_height, int process_rank, int num_processes, int com_tag)
{
    MPI_Sendrecv(&prev_data[(local_width + 1) * (local_height + 2)], local_height, MPI_FLOAT, (process_rank + 1) % num_processes, com_tag,
                 &prev_data[0], local_height, MPI_FLOAT, (process_rank - 1 + num_processes) % num_processes, com_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(&prev_data[local_height + 2], local_height, MPI_FLOAT, (process_rank - 1 + num_processes) % num_processes, com_tag,
                 &prev_data[(local_width + 2) * (local_height + 2)], local_height, MPI_FLOAT, (process_rank + 1) % num_processes, com_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void compute_new_data(float *new_data, float *prev_data, float *boundary_data, int local_width, int local_height)
{
    for (int i = 1; i <= local_width; ++i)
    {
        for (int j = 1; j <= local_height; ++j)
        {
            new_data[i * (local_height + 2) + j] = 0.25 * (boundary_data[i * (local_height + 2) + j] + prev_data[(i - 1) * (local_height + 2) + j] + prev_data[(i + 1) * (local_height + 2) + j] + prev_data[i * (local_height + 2) + j - 1] + prev_data[i * (local_height + 2) + j + 1]);
        }
    }
}

void swap_arrays(float **a, float **b)
{
    float *temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char **argv)
{
    int process_rank, num_processes;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    int com_tag = 333;

    if (argc != 3)
    {
        printf("Usage: %s <image.pgm> <nr_num_iterations>\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    int num_iterations = atoi(argv[2]);
    if (num_iterations <= 0)
    {
        printf("nr_num_iterations must be greater than 0\n");
        MPI_Finalize();
        return 0;
    }

    int image_width, image_height;
    pgm_size(argv[1], &image_width, &image_height);

    if (image_width % num_processes != 0)
    {
        printf("image_width must be divisible by num_processes\n");
        MPI_Finalize();
        return 0;
    }

    int local_width = image_width / num_processes, local_height = image_height;

    float *global_img_data = NULL;
    float *data = (float *)malloc(local_width * local_height * sizeof(float));
    float *boundary_data = (float *)malloc((local_width + 2) * (local_height + 2) * sizeof(float));
    float *prev_data = (float *)malloc((local_width + 2) * (local_height + 2) * sizeof(float));
    float *new_data = (float *)malloc((local_width + 2) * (local_height + 2) * sizeof(float));

    if (process_rank == 0)
    {
        global_img_data = (float *)malloc(image_width * image_height * sizeof(float));
        pgm_read(argv[1], global_img_data, image_width, image_height);
    }

    MPI_Scatter(global_img_data, local_width * local_height, MPI_FLOAT, data, local_width * local_height, MPI_FLOAT, 0, MPI_COMM_WORLD);

    initialize_arrays(data, prev_data, new_data, boundary_data, local_width, local_height);

    for (int k = 0; k < num_iterations; ++k)
    {
        update_boundaries(prev_data, local_width, local_height, process_rank, num_processes, com_tag);
        compute_new_data(new_data, prev_data, boundary_data, local_width, local_height);
        swap_arrays(&prev_data, &new_data);
    }

    for (int i = 1; i <= local_width; ++i)
    {
        for (int j = 1; j <= local_height; ++j)
        {
            data[(i - 1) * local_height + (j - 1)] = prev_data[i * (local_height + 2) + j];
        }
    }

    MPI_Gather(data, local_width * local_height, MPI_FLOAT, global_img_data, local_width * local_height, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if (process_rank == 0)
    {
        char output[20];
        sprintf(output, "output_%d.pgm", num_iterations);
        pgm_write(output, global_img_data, image_width, image_height);
        free(global_img_data);
    }

    free(data);
    free(boundary_data);
    free(prev_data);
    free(new_data);

    MPI_Finalize();

    return 0;
}
