# Define the compiler and the compiler flags
CC = gcc
CFLAGS = -Wall -g

# Define the target executable
TARGET = job_scheduler

# Define the object files
OBJS = job_scheduler.o queue.o

# Rule for making the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule for making the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Define the clean rule to remove intermediate files
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)

# Rule for making the queue.o file, depends on the queue.h header file
queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

# Rule for making the job_scheduler.o file, depends on the queue.h header file
job_scheduler.o: job_scheduler.c queue.h
	$(CC) $(CFLAGS) -c job_scheduler.c
