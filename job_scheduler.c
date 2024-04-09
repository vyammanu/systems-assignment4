#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h> // This is the line you need to add

#include "queue.h"
#include <time.h>

#define MAX_CMD_LEN 256


int jobCounter = 1;
int P; // Number of concurrent jobs
queue *jobQueue;

#define MAX_JOBS 1024 // Adjust this as needed for your use case

// Global array and count for completed jobs
Job *completed_jobs[MAX_JOBS];
int completed_jobs_count = 0;


void submitHistory() {
    printf("Job History:\n");
    printf("Total Completed Jobs: %d\n", completed_jobs_count);
    for (int i = 0; i < completed_jobs_count; ++i) {
        Job *job = completed_jobs[i];
        const char* status_str;

        switch (job->status) {
            case 0:
                status_str = "Waiting";
                break;
            case 1:
                status_str = "Running";
                break;
            case 2:
                status_str = "Completed Successfully";
                break;
            case 3:
                status_str = "Failed";
                break;
            default:
                status_str = "Unknown";
                break;
        }

        printf("Job ID: %d, Command: %s\n", job->jobid, job->command);
        printf("Status: %s\n", status_str);

        if (job->status == 2 || job->status == 3) {
            // Print start and end times only for completed or failed jobs
            printf("Start Time: %s", ctime(&job->start_time));
            printf("End Time: %s", ctime(&job->end_time));
            printf("Exit Status: %d\n", job->exit_status);
        }

        printf("PID: %d\n\n", job->pid);  // Added an extra newline for better readability
    }
}

void printJob(const Job *job) {
    if (job == NULL) {
        printf("Job is NULL\n");
        return;
    }
    
    // Assuming that status 0 is waiting, 1 is running, 2 is completed successfully, 3 is failed
    const char *status_str = "Unknown";
    switch (job->status) {
        case 0:
            status_str = "Waiting";
            break;
        case 1:
            status_str = "Running";
            break;
        case 2:
            status_str = "Completed Successfully";
            break;
        case 3:
            status_str = "Failed";
            break;
    }

    // Print job details
    printf("Job ID: %d\n", job->jobid);
    printf("Command: %s\n", job->command);
    if (job->status == 2 || job->status == 3) { // If job is completed or failed
        printf("Start Time: %s", ctime(&(job->start_time)));
        printf("End Time: %s", ctime(&(job->end_time)));
        printf("Exit Status: %d\n", job->exit_status);
    }
    printf("Status: %s\n", status_str);
}

void updateJobStatus() {
    int status;
    // Iterate through the jobs in the queue
    for (int i = 0; i < queue_count(jobQueue);) {
        Job *job = queue_peek(jobQueue, i);
        if (job && job->status == 1) { // Check if the job is running
            pid_t result = waitpid(job->pid, &status, WNOHANG);
            if (result > 0) { // Job has finished
                job->end_time = time(NULL); // Record end time
                if (WIFEXITED(status)) {
                    job->exit_status = WEXITSTATUS(status); // Exit status of the job
                } else if (WIFSIGNALED(status)) {
                    job->exit_status = WTERMSIG(status); // Signal that terminated the job
                    printf("Job ID %d terminated by signal %d\n", job->jobid, job->exit_status);
                } else {
                    job->exit_status = -1; // Abnormal termination
                }

                job->status = 2; // Mark as completed

                // Add job to the completed jobs list if space is available
                if (completed_jobs_count < MAX_JOBS) {
                    completed_jobs[completed_jobs_count++] = job;
                } else {
                    fprintf(stderr, "Error: Maximum number of completed jobs reached for Job ID %d.\n", job->jobid);
                }

                // Remove the completed job from the queue
                queue_delete_by_index(jobQueue, i); // Adjust index accordingly
            } else if (result == 0) {
                // Job is still running; move to the next job
                i++;
            } else {
                // Error handling for waitpid
                perror("waitpid error");
                i++;
            }
        } else {
            // If job is not running or NULL, proceed to the next job
            i++;
        }
    }
}

void executeJob(Job *job) {
    printf("Preparing to execute job ID %d with command: %s\n", job->jobid, job->command);

    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Attempt to open the output and error files, creating them if they do not exist
        int out_fd = open(job->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int err_fd = open(job->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (out_fd == -1 || err_fd == -1) {
            perror("Failed to open output or error file");
            exit(EXIT_FAILURE); // Ensure child process exits if file opening fails
        }

        // Redirect stdout and stderr to the respective files
        if (dup2(out_fd, STDOUT_FILENO) == -1 || dup2(err_fd, STDERR_FILENO) == -1) {
            perror("Failed to redirect stdout or stderr");
            exit(EXIT_FAILURE);
        }

        // Execute the command using /bin/sh
        char *argv[] = {"/bin/sh", "-c", job->command, NULL};
        if (execvp(argv[0], argv) == -1) {
            perror("Failed to execute command");
            exit(EXIT_FAILURE); // Ensure child process exits if exec fails
        }
    } else if (pid < 0) { // Failed to fork
        perror("Failed to fork");
        job->status = 3; // Update job status to indicate failure to start
    } else { // Parent process
        job->start_time = time(NULL); // Record the start time of the job
        job->pid = pid; // Store the PID of the child process
        job->status = 1; // Mark the job as running
    }
}


void printFileContents(const char *filename) {
    // Attempt to open the specified file in read mode
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open '%s' for reading.\n", filename);
        return; // Exit the function if the file cannot be opened
    }

    printf("Contents of '%s':\n", filename); // Optional: indicate which file's contents are being printed

    // Create a buffer to hold file content lines
    char buffer[1024];
    // Read each line from the file and print it
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        fputs(buffer, stdout); // Directly output the buffer to stdout
    }

    // Close the file to free system resources
    fclose(file);
}



void submitJob(const char *cmd) {
    // Allocate memory for a new job structure
    Job *newJob = malloc(sizeof(Job));
    if (!newJob) {
        perror("Failed to allocate memory for a new job");
        return;
    }

    // Initialize the new job with provided command and default values
    newJob->jobid = jobCounter++; // Increment jobCounter for a unique job ID
    snprintf(newJob->command, MAX_CMD_LEN, "%s", cmd); // Copy the command directly
    snprintf(newJob->out_file, MAX_CMD_LEN, "%d.out", newJob->jobid); // Name the output file
    snprintf(newJob->err_file, MAX_CMD_LEN, "%d.err", newJob->jobid); // Name the error file
    newJob->status = 0; // Mark as waiting
    newJob->start_time = time(NULL); // Record the start time

    // Attempt to insert the new job into the queue
    if (queue_insert(jobQueue, newJob) == 0) {
        printf("Job ID %d has been added to the queue.\n", newJob->jobid);
    } else {
        printf("Failed to insert Job ID %d into the queue. The queue might be full.\n", newJob->jobid);
        // Optional: Handle the job or free its resources if it can't be queued
    }

    // If the queue is not exceeding its limit, execute the job
    if (queue_count(jobQueue) <= P) {
        // Assuming executeJob is a function that starts the job if the queue isn't full
        executeJob(newJob);
    } else {
        printf("Job ID %d is waiting for execution. The queue is full.\n", newJob->jobid);
    }
}



void showJobs() {
    int jobCount = queue_count(jobQueue); // Get the total number of jobs currently in the queue
    if (jobCount == 0) {
        printf("No jobs are currently scheduled.\n");
        return;
    }

    // Iterate through each job in the queue and display its details
    for (int i = 0; i < jobCount; i++) {
        Job *job = queue_peek(jobQueue, i); // Retrieve job at position i without removing it from the queue
        if (job == NULL) {
            printf("Error retrieving job at position %d.\n", i);
            continue; // Proceed to the next job if the current one cannot be retrieved
        }

        // Determine the job's status as a string based on its status code
        const char* statusStr = "Unknown"; // Default status
        switch (job->status) {
            case 0: statusStr = "Waiting"; break;
            case 1: statusStr = "Running"; break;
            case 2: statusStr = "Completed"; break;
            default: break; // Keep "Unknown" for any undefined status codes
        }

        // Display the job's details
        printf("Job ID: %d, Command: %s, Status: %s\n", job->jobid, job->command, statusStr);
    }
}

int main(int argc, char *argv[]) {
    printf("Starting the job scheduler.\n");
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Number of Concurrent Jobs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    P = atoi(argv[1]); // Convert argument to integer for concurrent job limit
    jobQueue = queue_init(P); // Initialize the job queue to hold up to P jobs

    char input[MAX_CMD_LEN];
    while (1) {
        printf("scheduler> ");        
        if (!fgets(input, sizeof(input), stdin)) {
            if (feof(stdin)) { // End-of-file (EOF) detected, exit loop
                printf("\nExiting scheduler...\n");
                break;
            }
            continue; // Skip the rest of the loop on read error
        }
        
        // Remove the newline character, if present
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            continue; // Skip empty inputs
        }

        // Handle commands
        if (strcmp(input, "submithistory") == 0) {
            submitHistory(); // Display the history of submitted jobs
        } else if (strncmp(input, "submit ", 7) == 0) {
            submitJob(input + 7); // Submit a new job, skipping the command part
        } else if (strcmp(input, "showjobs") == 0) {
            showJobs(); // Show current jobs in the queue
        } else {
            fprintf(stderr, "Unknown command: %s\n", input); // Handle unrecognized input
        }

        usleep(1000000); // Sleep for a short period to throttle the loop (1 second)
        updateJobStatus(); // Periodically check and update the status of jobs
    }

    // Clean up before exiting
    queue_destroy(jobQueue);
    printf("Scheduler terminated.\n");
    return 0;
}
