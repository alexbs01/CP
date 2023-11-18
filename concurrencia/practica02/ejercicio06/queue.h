#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct _queue *queue;

queue q_create(int size);            // Create a new queue
int   q_elements(queue q);             // number of elements in a queue

void q_threadTerminated(queue q);
int q_getThreadTerminated(queue q);
void q_setThreadTerminated(queue q, int a);

void q_liberar(queue q);
int q_getLiberar(queue q);

int   q_insert(queue q, void *elem); // insert an element into a queue
void *q_remove(queue q);             // remove an element from the queue

void  q_destroy(queue q);             // destroy a queue.
#endif
