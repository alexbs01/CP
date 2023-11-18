#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include <math.h>

// mpicc main.c -o prueba -lm
// mpirun -n 4 --oversubscribe ./prueba 100 A

void inicializaCadena(char *cadena, int n) {
    int i;

    for (i = 0; i < n / 2; i++) {
        cadena[i] = 'A';
    }

    for (i = n / 2; i < 3 * n / 4; i++) {
        cadena[i] = 'C';
    }

    for (i = 3 * n / 4; i < 9 * n / 10; i++) {
        cadena[i] = 'G';
    }

    for (i = 9 * n / 10; i < n; i++) {
        cadena[i] = 'T';
    }
}


int MPI_BinomialBcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int numprocs, rank;
    int i, exit;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    for (i = 1; pow(2, i - 1) <= numprocs; i++) {

        if (rank < pow(2, i - 1) && rank + pow(2, i - 1) < numprocs) {

           exit = MPI_Send(buffer, count, datatype, rank + (int) pow(2, i - 1), 0, comm);

           if (exit != MPI_SUCCESS) {
               return exit;
           }
        }

        if(rank >= pow(2, i - 1) && rank < pow(2,i)) {
            MPI_Recv(buffer, count, datatype, rank - (int) pow(2, i - 1) , 0, comm, MPI_STATUS_IGNORE);
        }
    }

    return MPI_SUCCESS;
}


int MPI_FlattreeColectiva(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {

    int numprocs, rank, i, exit;

    exit = MPI_Send(sendbuf,count, datatype,0,0, comm);

    if (exit != MPI_SUCCESS) {
        return exit;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if(rank == 0) {
        int c = 0, j;

        for(i = 0; i < numprocs; i++) {
            MPI_Recv(&j, count, datatype, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
            c += j;
        }

        *(int *)recvbuf = c;
    }

    return MPI_SUCCESS;
}


int main(int argc, char *argv[]) {
    int i, c, count = 0;
    char *cadena;
    int n;
    char L;
    int numprocs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        n = atoi(argv[1]);
        L = *argv[2];

        cadena = (char *) malloc(n * sizeof(char));

        inicializaCadena(cadena, n);
        cadena[n] = '\0';
    }

    MPI_BinomialBcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        cadena = (char *) malloc(n * sizeof(char));
    }

    MPI_BinomialBcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&letra, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Bcast(cadena, n, MPI_CHAR, 0, MPI_COMM_WORLD);

    for (i = rank; i < n; i += numprocs) {
        if (cadena[i] == L) {
            count++;
        }
    }

    MPI_FlattreeColectiva(&count, &c, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Reduce(&count, &c, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("El numero de apariciones de la letra %c es %d\n", L, c);
    }

    free(cadena);
    MPI_Finalize();
    exit(0);
}