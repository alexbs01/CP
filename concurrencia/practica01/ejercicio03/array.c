#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include "options.h"

#define DELAY_SCALE 1000

struct array {
    int size;
    int *arr;
};

typedef struct args {
    int id;
    int iterations;
    int delay;
    struct array *arr;
    mtx_t *mutex;
} argumentos;

struct thrd_return{
    thrd_t thr;
    argumentos *argumentos;
};

// Aplica un delay para que el incremento de los elementos no sea tan seguido
void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}

// Muestra el array después de hacer los incrementos
void print_array(struct array arr) {
    int total = 0;

    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}

// Selecciona una posición al azar y la incrementa en 1, pero con los lock y unlock de los mutex
int incremento(void *p){
    int pos, val;
    struct args *args = p;

    for(int i = 0; i < args->iterations; i++) {
        pos = rand() % args->arr->size;
        printf("Thread %d of increase\t increasing position %d\n", args->id, pos);

        // Inicio de zona crítica
        mtx_lock(&args->mutex[pos]);

        val = args->arr->arr[pos];
        apply_delay(args->delay);
        val++;

        apply_delay(args->delay);
        args->arr->arr[pos] = val;

        apply_delay(args->delay);

        // Fin de zona crítica
        mtx_unlock(&args->mutex[pos]);
    }


    return 0;
}

// Selecciona dos posiciones al azar, a la primera le resta 1 y a la segunda le suma 1
int intercambio(void *p){
    int pos1, pos2, val;
    struct args *args = p;

    for(int i = 0; i < args->iterations; i++) {
        pos1 = rand() % args->arr->size;
        do {
            pos2 = rand() % args->arr->size;
        } while(pos1 == pos2);

        printf("Thread %d of change\t changing position %d and %d\n", args->id, pos1, pos2);

        // Bloqueamos posiciones
        if(pos1 < pos2) {
            mtx_lock(&args->mutex[pos1]);
            mtx_lock(&args->mutex[pos2]);
        } else {
            mtx_lock(&args->mutex[pos2]);
            mtx_lock(&args->mutex[pos1]);
        }

        // Decremento del primer valor
        val = args->arr->arr[pos1];
        apply_delay(args->delay);
        val--;

        apply_delay(args->delay);
        args->arr->arr[pos1] = val;

        apply_delay(args->delay);

        // Incremento del segundo valor
        val = args->arr->arr[pos2];
        apply_delay(args->delay);
        val++;

        apply_delay(args->delay);
        args->arr->arr[pos2] = val;

        apply_delay(args->delay);

        mtx_unlock(&args->mutex[pos2]);
        mtx_unlock(&args->mutex[pos1]);
    }

    return 0;
}

// Función que crea un thread de incremento
struct thrd_return createThread(int id, struct options opt, struct array *arr, mtx_t *mutex, int (funThr)(void *p)) {
    thrd_t thr; // Creamos un identificador para el hilo

    // Creamos espacio para guardar las direcciones de lor parámetros
    argumentos *args = malloc(sizeof(argumentos));

    // Guardamos en el struct de argumentos, los parámetros para la creación del hilo
    args->id = id;
    args->iterations = opt.iterations;
    args->delay = opt.delay;
    args->arr = arr;
    args->mutex = mutex;

    struct thrd_return ret; // Se crea una variable para el retorno del hilo
    ret.argumentos = args; // Guardamos los agumentos

    // Si ocurre un error con la creación retorna un mensaje de error
    /* Aquí indicamos su identificador, la función que ejecutarán que será el de incrementar
       elementos del array, y sus argumentos */
    if(thrd_create(&thr, funThr, args) == thrd_error){
        perror("No se pudo crear el thread de incremento");
        ret.argumentos = NULL; // Y la dirección de los argumentos se pasa a NULL
    }

    ret.thr = thr; // Guardamos el identificador

    return ret;
}

int main (int argc, char **argv) {
    struct options opt;
    struct array arr;

    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 5;
    opt.size         = 10;
    opt.iterations   = 100;
    opt.delay        = 1000;

    // Se leen los parámetros, si los hay sobreescriben a los valores por defecto
    read_options(argc, argv, &opt);

    // Creamos tantos hilos como se indiquen por parámetros
    struct thrd_return thread[opt.num_threads * 2 - 1];

    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));
    memset(arr.arr, 0, arr.size * sizeof(int));

    mtx_t mutex[opt.size];

    // Inicializamos los mutex, uno por cada posición del array
    for(int i = 0; i < opt.size; i++) {
        if(mtx_init(&mutex[i], mtx_plain) == thrd_error){
            perror("No se pudo crear el mutex");
            return 0;
        }
    }

    // Creamos los hilos
    for(int i = 0; i < opt.num_threads; i++) {
        thread[i] = createThread(i, opt, &arr, mutex, incremento);
        thread[i + opt.num_threads] = createThread(i + opt.num_threads, opt, &arr, mutex, intercambio);
    }

    // Los ejecutamos
    for(int i = 0; i < opt.num_threads; i++){
        thrd_join(thread[i].thr, NULL);
        thrd_join(thread[i + opt.num_threads].thr, NULL);
        free(thread[i].argumentos);
        free(thread[i + opt.num_threads].argumentos);
    }

    // Y destruimos los mutex
    for(int i = 0; i < opt.size; i++) {
        mtx_destroy(&mutex[i]);
    }

    // Mostramos el array
    print_array(arr);

    // Liberamos memoria dinámica
    free(arr.arr);

    return 0;
}
