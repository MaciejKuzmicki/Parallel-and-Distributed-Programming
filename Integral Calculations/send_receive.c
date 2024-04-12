#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

double func(double x) {
    return sin(x) * x * x / 17 + x  + exp(x);
}

typedef struct {
    double begin;
    double end;
    int n;
} integralarg;

double calcdelta(integralarg a) {
    return (a.end - a.begin) / a.n;
}

double integral(integralarg a, double (*f)(double)) {
    double result = 0;
    double delta = calcdelta(a);
    double i;
    for (int j = 0; j < a.n; j++) {
        i = a.begin + j * delta;
        result += delta * (f(i) + f(i + delta)) / 2;
    }
    return result;
}

void calcranges(integralarg arg, integralarg* args, int proc) {
    int nPerProc = arg.n / proc; 
    int remainder = arg.n % proc; 
    double currentBegin = arg.begin;
    double delta = calcdelta(arg); 

    int pointsForThisProc;
    double rangeForThisProc;
    
    for (int i = 0; i < proc; i++) {
        pointsForThisProc = nPerProc + (i < remainder ? 1 : 0); 
        rangeForThisProc = pointsForThisProc * delta; 
        args[i].begin = currentBegin;
        args[i].end = currentBegin + rangeForThisProc;
        args[i].n = pointsForThisProc;
        currentBegin = args[i].end; 
    }
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);

    int rank, world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    double begin = argc > 1 ? atof(argv[1]) : 0;
    double end = argc > 2 ? atof(argv[2]) : 10;
    int n = argc > 3 ? atoi(argv[3]) : 1000;

    integralarg arg = {begin, end, n};
    integralarg myarg;

    MPI_Datatype mpi_integralarg_type;
    int blocklengths[3] = {1, 1, 1};
    MPI_Aint offsets[3];
    offsets[0] = offsetof(integralarg, begin);
    offsets[1] = offsetof(integralarg, end);
    offsets[2] = offsetof(integralarg, n);
    MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT};
    MPI_Type_create_struct(3, blocklengths, offsets, types, &mpi_integralarg_type);
    MPI_Type_commit(&mpi_integralarg_type);

    if (rank == 0) {
        integralarg* ranges = malloc(world * sizeof(integralarg));
        calcranges(arg, ranges, world);
        for (int i = 1; i < world; i++) {
            MPI_Send(&ranges[i], 1, mpi_integralarg_type, i, 0, MPI_COMM_WORLD);
        }
        myarg = ranges[0];
        free(ranges);
    } else {
        MPI_Recv(&myarg, 1, mpi_integralarg_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double local_result = integral(myarg, func);
    double total_result = 0;

    if (rank == 0) {
        total_result = local_result;
        double recv_result;
        for (int i = 1; i < world; i++) {
            MPI_Recv(&recv_result, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_result += recv_result;
        }
        printf("Total integral: %f\n", total_result);
    } else {
        MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Type_free(&mpi_integralarg_type);
    MPI_Finalize();

    return 0;
}