#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>

// mpicc main.c -o prueba
// pirun -n 4 --oversubscribe ./prueba 100 A

void inicializaCadena(char *cadena, int n) {
    int i;

    for(i=0; i < n/2; i++){
        cadena[i] = 'A';
    }

    for(i=n/2; i < 3*n/4; i++){
        cadena[i] = 'C';
    }

    for(i=3*n/4; i < 9*n/10; i++){
        cadena[i] = 'G';
    }

    for(i=9*n/10; i < n; i++){
        cadena[i] = 'T';
    }
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

    if(rank == 0) {
        n = atoi(argv[1]);
        L = *argv[2];

        cadena = (char *) malloc(n * sizeof(char));

        inicializaCadena(cadena, n);
        cadena[n] = '\0';
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank != 0) {
        cadena = (char *) malloc(n * sizeof(char));
    }

    MPI_Bcast(&L,1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(cadena, n, MPI_CHAR,0, MPI_COMM_WORLD);

    for(i = rank; i < n; i += numprocs) {
        if(cadena[i] == L) {
            count++;
        }
    }

    MPI_Reduce(&count, &c, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        printf("El numero de apariciones de la letra %c es %d\n", L, c);
    }

    free(cadena);
    MPI_Finalize();
    exit(0);
}