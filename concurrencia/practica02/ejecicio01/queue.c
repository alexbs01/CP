#include <stdlib.h>
#include <threads.h>
#include <stdio.h>

// circular array
typedef struct _queue {
    int size;
    int used;
    int first;
    void **data;
    int threadsTerminated;
    mtx_t *buffer_lock;
    cnd_t *buffer_empty;
    cnd_t *buffer_full;
    int noHayMasElementos;
} _queue;

#include "queue.h"

queue q_create(int size) {
    queue q = malloc(sizeof(_queue));

    q->size  = size;
    q->used  = 0;
    q->first = 0;
    q->noHayMasElementos = 0;
    q->threadsTerminated = 0;
    q->data  = malloc(size * sizeof(void *));
    q->buffer_lock = malloc(sizeof(mtx_t));
    mtx_init(q->buffer_lock, mtx_plain);
    q->buffer_empty = malloc(sizeof(cnd_t));
    q->buffer_full = malloc(sizeof(cnd_t));
    cnd_init(q->buffer_empty);
    cnd_init(q->buffer_full);

    return q;
}

int q_elements(queue q) {
    return q->used;
}

void q_threadTerminated(queue q) {
    mtx_lock(q->buffer_lock);
    q->threadsTerminated--;
    mtx_unlock(q->buffer_lock);
}

int q_getThreadTerminated(queue q) {
    return q->threadsTerminated;
}

void q_liberar(queue q) {
    mtx_lock(q->buffer_lock);
    q->noHayMasElementos = 1;
    cnd_broadcast(q->buffer_full);
    cnd_broadcast(q->buffer_empty);
    mtx_unlock(q->buffer_lock);
}

int q_getLiberar(queue q) {
    return q->noHayMasElementos;
}

int q_insert(queue q, void *elem) {
    mtx_lock(q->buffer_lock);
    while(q->used == q->size) {
        cnd_wait(q->buffer_full, q->buffer_lock);
    }
    // Esto inserta el elemento en la lista
    if(q->noHayMasElementos) {
        mtx_unlock(q->buffer_lock);
        return -1;
    }
    q->data[(q->first + q->used) % q->size] = elem;
    q->used++;
    //
    cnd_broadcast(q->buffer_empty);
    mtx_unlock(q->buffer_lock);

    return 0;
}

void *q_remove(queue q) {
    mtx_lock(q->buffer_lock); // Bloqueamos el buffer

    // Si la cola está vacía y hay más elementos que se insertarán en la cola
    while(q->used == 0 && !q->noHayMasElementos && q->threadsTerminated == 0) {
        // printf("R cnd_wait %lu\n", thrd_current());
        // Desbloquea buffer_lock y espera a que buffer tenga elementos
        cnd_wait(q->buffer_empty, q->buffer_lock);
    }

    // ############
    // Si no hay más elementos y la cola está vacía, desbloquea y retorna NULL
    if(q->noHayMasElementos && q->used == 0) {
        mtx_unlock(q->buffer_lock);
        return NULL;
    }

    void *res = q->data[q->first]; // Selecciona el elemento que se elimina

    q->first = (q->first + 1) % q->size; // Establece el nuevo primer elemento
    q->used--; // Establece cuantos elementos hay en la cola
    // ############

    // Manda señal a buffer_full y desbloquea el mutex
    cnd_broadcast(q->buffer_full);
    mtx_unlock(q->buffer_lock);

    return res;
}

void q_destroy(queue q) {
    cnd_destroy(q->buffer_empty);
    cnd_destroy(q->buffer_full);
    mtx_destroy(q->buffer_lock);
    free(q->data);
    free(q);
}
