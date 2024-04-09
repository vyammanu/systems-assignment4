/* Header file for the simple circular queue */
#ifndef QUEUE_H
#define QUEUE_H

#include <time.h> // Necessary for time-related operations
#include <sys/types.h> // For pid_t

#define MAX_CMD_LEN 256

// Job structure to encapsulate job details
typedef struct {
    int jobid; // Unique identifier for the job
    char command[MAX_CMD_LEN]; // Command to execute
    pid_t pid; // Process ID after fork
    int status; // Job status: 0 for waiting, 1 for running, 2 for completed
    char out_file[MAX_CMD_LEN]; // File path for stdout redirection
    char err_file[MAX_CMD_LEN]; // File path for stderr redirection
    time_t start_time; // Start timestamp
    time_t end_time; // End timestamp
    int exit_status; // Exit status of the job
} Job;

// Queue structure for managing jobs
typedef struct _queue {
    int size; // Maximum capacity of the queue
    Job **buffer; // Buffer for storing pointers to Job structures
    int start; // Index of the queue's front
    int end; // Index of the queue's rear
    int count; // Number of jobs currently in the queue
} queue;

// Function prototypes for queue operations
queue *queue_init(int n); // Initialize the queue
int queue_insert(queue *q, Job* item); // Insert a job into the queue
Job* queue_delete(queue *q); // Remove and return the front job from the queue
void queue_display(queue *q); // Display current jobs in the queue
void queue_destroy(queue *q); // Free queue resources
int queue_count(queue *q); // Get the count of jobs in the queue
Job *queue_peek(queue *q, int index); // Peek at a job in the queue by index
int queue_delete_by_index(queue *q, int index); // Remove a job by its index in the queue

#endif // QUEUE_H
