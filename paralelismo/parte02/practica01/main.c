#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>

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

    if(argc != 3) {
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1);
    }

    int i, n, count=0;
    char L;
    char *cadena;
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        n = atoi(argv[1]);
        L = *argv[2];

        cadena = (char *) malloc(n * sizeof(char));
        inicializaCadena(cadena, n);

        for(i = 1; i < size; i++) {
            MPI_Send (&n,1, MPI_INT,i,0, MPI_COMM_WORLD); //Aquí hay que enviar a los otros rangos n, L y cadena
            MPI_Send (&L,1, MPI_CHAR,i,0, MPI_COMM_WORLD);
            MPI_Send (cadena,n, MPI_CHAR,i,0, MPI_COMM_WORLD);
        }

    } else {
        MPI_Recv (&n,1, MPI_INT, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv (&L,1, MPI_CHAR, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cadena = (char *) malloc(n * sizeof(char));

        MPI_Recv (cadena,n, MPI_CHAR, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for(i = rank; i < n; i += size) {
        if(cadena[i] == L) {
            count++;
        }
    }
    MPI_Send (&count,1, MPI_INT,0,0, MPI_COMM_WORLD);


    if(rank == 0) {
        int c = 0;

        for(i = 0; i < size; i++) {
            MPI_Recv(&count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            c += count; // Sumamos los count de todos los procesos
        }

        printf("El numero de apariciones de la letra %c es %d\n", L, c);
    }

    free(cadena);
    MPI_Finalize();
    exit(0);
}