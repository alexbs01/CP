#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include <math.h>

// mpicc main.c -o prueba
// mpirun -n 4 --oversubscribe ./prueba 100 A

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

int MPI_BinomialBCast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for(int i = root; rank < pow(2, i - 1); rank += pow(2, i - 1)) {
        MPI_Send(buffer, count, datatype, i, 0, comm);
        i++;
    }



    for(int i = root; rank < pow(2, i - 1); rank += pow(2, i - 1)) {
        MPI_Recv(buffer, count, datatype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUSES_IGNORE);
        i++;
    }

    return 0;
}

int MPI_FlattreeColectiva(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int acc = 0;
    int buffer;
    int errorCode;

    errorCode = MPI_Send(sendbuf, count, datatype, 0, 0, comm) != 0;
    if(errorCode) {
        return errorCode;
    }

    for(int i = root; rank < pow(2, i - 1); rank += pow(2, i - 1)) {
        errorCode = MPI_Recv(&buffer, count, datatype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUSES_IGNORE) != 0;
        if(errorCode) {
            return errorCode;
        }
        acc += buffer;
        i++;
    }

    recvbuf = &acc;
    printf("c\n");

    return 0;

}

int main(int argc, char *argv[]) {
    int i, n, count = 0;
    char *cadena;
    char letra;
    int size, rank;
    int resultado;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        printf("Escribe una letra y un tamaño: \n"); scanf("%c %d", &letra, &n); // Pedimos la letra y el tamaño de la cadena

        cadena = (char *) malloc(n * sizeof(char)); // Reservamos memoria para la cadena

        inicializaCadena(cadena, n); // Inicializamos la cadena
        cadena[n] = '\0';  // Añadimos el caracter de fin de cadena
    }

    // Enviamos el tamaño de la cadena a todos los procesos
    // Recordar que las operaciones colectivas deben ejecutarlos todos los procesos
    MPI_BinomialBCast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank != 0) { // Reservamos memoria para la cadena en los procesos que no son el principal
        cadena = (char *) malloc(n * sizeof(char));
    }

    // Enviamos la letra y la cadena a todos los procesos
    MPI_BinomialBCast(&letra,1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_BinomialBCast(cadena, n, MPI_CHAR,0, MPI_COMM_WORLD);

    for(i = rank; i < n; i += size) {
        if(cadena[i] == letra) { // Sumamos 1 al contador si la letra coincide
            count++;
        }
    }
    printf("%s\n", cadena);
    // Sumamos los contadores de cada proceso y almacenamos el resultado en la variable resultado
    MPI_FlattreeColectiva(&count, &resultado, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("b\n");
    if(rank == 0) {
        printf("El numero de apariciones de la letra %c es %d\n", letra, resultado);
    }

    free(cadena);
    MPI_Finalize();
    exit(0);
}