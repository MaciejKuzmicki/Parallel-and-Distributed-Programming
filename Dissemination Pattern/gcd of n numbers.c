#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int nwd(int a, int b)
{
    while(a!=b)
       if(a>b)
           a-=b; 
       else
           b-=a;
    return a;
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size != argc -1) {
        if(rank == 0) {
            printf("Number of numbers is not equal to the number of processes\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return EXIT_FAILURE;
    }

    int local_value = atoi(argv[rank+1]);
    printf("Process %d starts with value %d\n", rank, local_value);
    int step_distance, partner, received_value, receive_from;

    for (int i = 0; i < (int)log2(size); i++) {
        step_distance = pow(2,i);
        partner = (rank + step_distance) % size;
        receive_from = (rank-step_distance+size) % size;
        MPI_Send(&local_value, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
        MPI_Recv(&received_value, 1, MPI_INT, receive_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_value = nwd(local_value, received_value);
    }

    printf("Final nwd result from process %d is %d\n", rank, local_value);
    MPI_Finalize();
    return 0;
}
