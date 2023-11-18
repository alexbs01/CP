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

void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}



int increment(void *p) {
    int pos, val;
    argumentos *args = p;

    mtx_lock(args->mutex);

    for(int i = 0; i < args->iterations; i++) {
        pos = rand() % args->arr->size;

        printf("Thread %d increasing position %d\n", args->id, pos);

        val = args->arr->arr[pos];
        apply_delay(args->delay);

        val ++;
        apply_delay(args->delay);

        args->arr->arr[pos] = val;
        apply_delay(args->delay);
    }

    mtx_unlock(args->mutex);

    return 0;
}

void print_array(struct array arr) {
    int total = 0;

    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}

// Función que crea un thread
struct thrd_return create_thread(int id, struct options opt, struct array *arr, mtx_t *mutex) {
    thrd_t thr; // Creamos un identificador para el hilo

    // Creamos espacio para guardar las direcciones de lor parámetros
    argumentos *args = malloc(sizeof(struct args));

    // Guardamos en el struct de argumentos, los parámetros para la creación del hilo
    args->id = id;
    args->iterations = opt.iterations;
    args->delay = opt.delay;
    args->arr = arr;
    args->mutex = mutex;

    struct thrd_return ret; // Se crea una variable para el retorno del hilo
    ret.argumentos = args; // Guardamos los agumentos

    // Si ocurre un error con la creación retorna un mensaje de error
    /* Aquí indicamos su identificador, la función que ejecutarán que s
       erá el de incrementar elementos del array, y sus argumentos */
    if(thrd_create(&thr, increment, args) == thrd_error){
        perror("No se pudo crear el thread");
        ret.argumentos = NULL; // Y la dirección de los argumentos se pasa a NULL
    }

    ret.thr = thr; // Guardamos el identificador

    return ret;
}

int main (int argc, char **argv)
{
    struct options opt;
    struct array arr;

    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 5;
    opt.size         = 10;
    opt.iterations   = 100;
    opt.delay        = 1000;

    read_options(argc, argv, &opt);

    struct thrd_return thread[opt.num_threads - 1];

    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));

    memset(arr.arr, 0, arr.size * sizeof(int));

    mtx_t mutex;

    if(mtx_init(&mutex, mtx_plain) == thrd_error){
        perror("No se pudo crear el mutex");
        return 0;
    }

    // Creamos los hilos
    for(int i = 0; i < opt.num_threads; i++) {
        thread[i] = create_thread(i, opt, &arr, &mutex);
    }

    for(int i = 0; i < opt.num_threads; i++) {

        thrd_join(thread[i].thr, NULL);
        free(thread[i].argumentos);
    }

    mtx_destroy(&mutex);


    print_array(arr);

    free(arr.arr);

    return 0;
}
