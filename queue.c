#include <stdio.h>
#include <stdlib.h>
#include "queue.h" // Ensure this header file defines the queue structure and Job (formerly Job) appropriately
#include <time.h> 

// Initialize the queue
queue *queue_init(int n) {
    queue *q = (queue *)malloc(sizeof(queue));
    q->size = n;
    q->buffer = malloc(sizeof(Job *) * n); // Allocate for Job pointers
    q->start = 0;
    q->end = 0;
    q->count = 0;

    return q;
}

// Insert an Job into the queue
int queue_insert(queue *q, Job *Job) {
    if ((q == NULL) || (q->count == q->size))
        return -1;

    q->buffer[q->end] = Job;
    q->end = (q->end + 1) % q->size;
    q->count++;

    return 0; // Success
}

// Delete an Job from the queue
Job *queue_delete(queue *q) {
    if ((q == NULL) || (q->count == 0))
        return NULL;

    Job *Job = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    q->count--;
    return Job; // Return the deleted Job
}

// Peek at an Job in the queue by index
Job *queue_peek(queue *q, int index) {
    if ((q == NULL) || (index < 0) || (index >= q->count))
        return NULL;

    int actualIndex = (q->start + index) % q->size;
    return q->buffer[actualIndex];
}

// Get the count of Jobs in the queue
int queue_count(queue *q) {
    return (q != NULL) ? q->count : 0;
}

// Display the contents of the queue
void queue_display(queue *q) {
    if (q != NULL && q->count > 0) {
        printf("Queue has %d elements, start = %d, end = %d\n", q->count, q->start, q->end);
        printf("Queue contents: ");
        for (int i = 0; i < q->count; i++)
            printf("%p ", (void *)q->buffer[(q->start + i) % q->size]); // Prints memory addresses as placeholders
        printf("\n");
    } else {
        printf("Queue is empty, nothing to display\n");
    }
}

// Free the allocated memory for the queue
void queue_destroy(queue *q) {
    if (q != NULL) {
        free(q->buffer);
        free(q);
    }
}

// Delete an Job from the queue by index
int queue_delete_by_index(queue *q, int index) {
    if (q == NULL || q->count == 0 || index < 0 || index >= q->count) {
        return -1;
    }

    // Shift all elements after the actualIndex down by one
    for (int i = index; i < q->count - 1; i++) {
        q->buffer[(q->start + i) % q->size] = q->buffer[(q->start + i + 1) % q->size];
    }
    q->end = (q->end - 1 + q->size) % q->size;
    q->count--;

    return 0; // Success
}
