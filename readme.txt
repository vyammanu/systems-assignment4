Job Scheduler
=============

Description:
------------
The Job Scheduler is a simple system for managing and executing background jobs. It allows users to submit jobs, view the status of running and waiting jobs, and provides a history of completed or failed jobs.

Requirements:
-------------
- GCC Compiler
- Standard C library

Compilation:
------------
To compile the Job Scheduler, a Makefile is provided. Navigate to the directory containing the source code and execute the following command:

  make

This will produce an executable named `job_scheduler`.

Usage:
------
To start the Job Scheduler, run the executable with a single argument specifying the number of concurrent jobs allowed:

  ./job_scheduler <Number of Concurrent Jobs>

Commands:
---------
- submit <command>: Submits a new job to the scheduler. Example:

    scheduler> submit /path/to/script.sh

- showjobs: Displays a list of currently running and waiting jobs.

    scheduler> showjobs

- submithistory: Displays a history of all jobs processed by the scheduler.

    scheduler> submithistory

Each job's output and errors are redirected to files named `<jobid>.out` and `<jobid>.err` respectively.

Cleaning Up:
------------
To clean up the object files and the executable, run:

  make clean

Additional Notes:
-----------------
- The Job Scheduler can handle a maximum of 1024 completed jobs for history tracking. Adjust the `MAX_JOBS` definition in the source code if necessary.
- Ensure that the command submitted does not include 'submit ', as the scheduler adds this internally.
- The scheduler checks the status of running jobs once per second. If a finer resolution is needed, adjust the `usleep` duration in the main function.

Exiting the Scheduler:
----------------------
To exit the scheduler, use the EOF signal (Ctrl+D on Unix-like systems) when prompted for input.
