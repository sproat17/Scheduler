/*  program to simulate different scheduling algrothims and print out the
   running process to approriate files every 10 units of time */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// only holds 1000 processes. need to change processes to be dynamically allocated
#define MAX_SIZE 1000

FILE *input, *fcfs, *rr_10, *rr_40, *spn, *srt, *hrrn, *feedback;

// process struct
typedef struct {
  char processID[MAX_SIZE];
  int arrivalTime;
  int serviceTime;
  int remainingTime;
  int priority;
} Process;

// quicksort helper
int partition(Process *processes, int low, int high) {
  Process pivot = processes[high];
  int i = low - 1;

  for (int j = low; j < high; j++) {
    if (processes[j].arrivalTime <= pivot.arrivalTime) {
      i++;
      Process temp = processes[i];
      processes[i] = processes[j];
      processes[j] = temp;
    }
  }

  Process temp = processes[i + 1];
  processes[i + 1] = processes[high];
  processes[high] = temp;

  return i + 1;
}

// main quicksort function
void quicksort(Process *processes, int low, int high) {
  if (low < high) {
    int pi = partition(processes, low, high);
    quicksort(processes, low, pi - 1);
    quicksort(processes, pi + 1, high);
  }
}

// define queue
typedef struct Queue {
  Process *queue;
  int front;
  int rear;
  int size;
} Queue;

Queue create_queue(int size) {
  Queue q;
  q.queue = (Process *)malloc(size * sizeof(Process));
  q.front = 0;
  q.rear = 0;
  q.size = size;
  return q;
}

void enqueue(Queue *q, Process *process) {
  if ((q->rear + 1) % q->size == q->front) {
    printf("queue is full\n");
    return;
  }
  q->queue[q->rear] = *process;
  q->rear = (q->rear + 1) % q->size;
}

Process *dequeue(Queue *q) {
  if (q->front == q->rear) {
    printf("queue is empty\n");
    return NULL;
  }
  Process *process = &q->queue[q->front];
  q->front = (q->front + 1) % q->size;
  return process;
}

int is_queue_empty(Queue *q) { return q->front == q->rear; }

// first come first serve
void fcfs_algorithm(int n, Process *processes) {

  fcfs = fopen("fcfs.out", "w");

  int time = 0;
  int i;

  // sort processes by arrival time using quicksort
  quicksort(processes, 0, n - 1);

  for (i = 0; i < n; i++) {
    int current_time = time;
    while (processes[i].remainingTime > 0) {
      time++;
      processes[i].remainingTime--;
      if (time % 10 == 0) {
        fprintf(fcfs, "%s\n", processes[i].processID);
      }
    }
    time = current_time + processes[i].serviceTime;
  }
  fclose(fcfs);
}

// round robin algorithm for either 10 or 40 qunatlum
void rr_algorithm(int n, Process *processes, int quantum) {

  int time = 0;
  int i, j;
  Queue ready_queue = create_queue(n + 1);

  if (quantum == 10) {
    rr_10 = fopen("rr_10.out", "w");

  } else {
    rr_40 = fopen("rr_40.out", "w");
  }

  // add processes that have already arrived
  for (i = 0; i < n; i++) {
    if (processes[i].arrivalTime == 0) {
      enqueue(&ready_queue, &processes[i]);
    }
  }

  while (!is_queue_empty(&ready_queue)) {
    Process *current_process = dequeue(&ready_queue);
    int slice = quantum;

    while (current_process->remainingTime > 0 && slice > 0) {
      if (time % 10 == 0) {
        // write to either the rr_10 or rr_40
        if (quantum == 10) {
          fprintf(rr_10, "%s\n", current_process->processID);
        } else {
          fprintf(rr_40, "%s\n", current_process->processID);
        }
      }
      time++;
      current_process->remainingTime--;
      slice--;

      // check for arrivals and add them to queue
      for (j = 0; j < n; j++) {
        if (processes[j].arrivalTime == time &&
            processes[j].remainingTime > 0) {
          enqueue(&ready_queue, &processes[j]);
        }
      }
    }

    if (current_process->remainingTime > 0) {
      enqueue(&ready_queue, current_process);
    }
  }
  if (quantum == 10) {
    fclose(rr_10);

  } else {
    fclose(rr_40);
  }
}

// shortest process next
void spn_algorithm(int n, Process *processes) {
  spn = fopen("spn.out", "w");
  int time = 0; // current time
  int i;

  while (1) {
    int shortest_index = -1;
    int shortest_time = MAX_SIZE;
    for (i = 0; i < n; i++) {
      if (processes[i].arrivalTime <= time && processes[i].remainingTime > 0) {
        if (processes[i].remainingTime < shortest_time) {
          shortest_time = processes[i].remainingTime;
          shortest_index = i;
        }
      }
    }

    if (shortest_index == -1) {
      time++;
      continue;
    }

    // execute the process for the remaining time
    while (processes[shortest_index].remainingTime > 0) {
      time++;
      processes[shortest_index].remainingTime--;
      if (time % 10 == 0) {
        fprintf(spn, "%s\n", processes[shortest_index].processID);
      }
    }

    // check if all processes have finished executing
    int completed = 1;
    for (i = 0; i < n; i++) {
      if (processes[i].remainingTime > 0) {
        completed = 0;
        break;
      }
    }
    if (completed) {
      break;
    }
  }
  fclose(spn);
}

// shortest remaining time
void srt_algorithm(int n, Process *processes) {
  srt = fopen("srt.out", "w");
  int time = 0; // current time
  int i;

  while (1) {
    int shortest_remaining_time = MAX_SIZE;
    int shortest_index = -1;

    // find the process with the shortest remaining time
    for (i = 0; i < n; i++) {
      if (processes[i].arrivalTime <= time && processes[i].remainingTime > 0 &&
          processes[i].remainingTime < shortest_remaining_time) {
        shortest_remaining_time = processes[i].remainingTime;
        shortest_index = i;
      }
    }

    // if no process has arrived yet, move time forward
    if (shortest_index == -1) {
      time++;
      continue;
    }

    // print the process ID every 10 units of time
    if (time % 10 == 0) {
      fprintf(srt, "%s\n", processes[shortest_index].processID);
    }

    processes[shortest_index].remainingTime--;
    time++;

    // check if current process has finished executing
    if (processes[shortest_index].remainingTime == 0) {
      // check if all processes have finished executing
      int completed = 1;
      for (i = 0; i < n; i++) {
        if (processes[i].remainingTime > 0) {
          completed = 0;
          break;
        }
      }
      if (completed) {
        break;
      }
    }
  }
  fclose(srt);
}

// response ratio = (waiting time + service time) / service time
// highest response ratio next
void hrrn_algorithm(int n, Process *processes) {
  hrrn = fopen("hrrn.out", "w");
  int time = 0;
  int i;

  while (1) {
    double highest_response_ratio = -1;
    int highest_index = -1;

    // find the process with the highest response ratio
    for (i = 0; i < n; i++) {
      if (processes[i].arrivalTime <= time && processes[i].remainingTime > 0) {

        double waiting_time =
            time - processes[i].arrivalTime - processes[i].serviceTime;
        if (processes[i].remainingTime <
            processes[highest_index].remainingTime) {
          waiting_time += processes[highest_index].remainingTime -
                          processes[i].remainingTime;
        }

        double response_ratio = (waiting_time + processes[i].serviceTime) /
                                (double)processes[i].serviceTime;
        if (response_ratio > highest_response_ratio) {
          highest_response_ratio = response_ratio;
          highest_index = i;
        }
      }
    }

    // if no process has arrived move time forward
    if (highest_index == -1) {
      time++;
      continue;
    }

    // print the processID every 10 units of time
    if (time % 10 == 0) {
      fprintf(hrrn, "%s\n", processes[highest_index].processID);
    }

    processes[highest_index].remainingTime--;
    time++;

    // check if finished
    if (processes[highest_index].remainingTime == 0) {
      // check if all processes have finished
      int completed = 1;
      for (i = 0; i < n; i++) {
        if (processes[i].remainingTime > 0) {
          completed = 0;
          break;
        }
      }
      if (completed) {
        break;
      }
    }
  }
  fclose(hrrn);
}

// feedback algorithm
// assumption is made that the quantum remains constant and equal to 10
void feedback_algorithm(int n, Process *processes) {
  feedback = fopen("feedback.out", "w");
  int time = 0;
  int i, j;

  // create queues for each level
  Queue q0_queue = create_queue(n + 1); // highest priority
  Queue q1_queue = create_queue(n + 1);
  Queue q2_queue = create_queue(n + 1);
  Queue q3_queue = create_queue(n + 1); // lowest priority

  // add processes that arrive at time 0 to the highest priority queue
  for (i = 0; i < n; i++) {
    if (processes[i].arrivalTime == 0) {
      enqueue(&q0_queue, &processes[i]);
      processes[i].priority = 0;
    }
  }

  // run the algorithm until all queues are empty
  while (!is_queue_empty(&q0_queue) || !is_queue_empty(&q1_queue) ||
         !is_queue_empty(&q2_queue) || !is_queue_empty(&q3_queue)) {

    Process *current_process;

    // check queues in order of priority
    if (!is_queue_empty(&q0_queue)) {
      current_process = dequeue(&q0_queue);
    } else if (!is_queue_empty(&q1_queue)) {
      current_process = dequeue(&q1_queue);
    } else if (!is_queue_empty(&q2_queue)) {
      current_process = dequeue(&q2_queue);
    } else {
      current_process = dequeue(&q3_queue);
    }

    int slice = 10;

    while (slice > 0) {
      if (current_process->remainingTime > 0) {
        if (time % 10 == 0) {
          fprintf(feedback, "%s\n", current_process->processID);
        }
        time++;
        current_process->remainingTime--;
        slice--;

        // check for new arrivals and add them to the highest priority queue
        for (j = 0; j < n; j++) {
          if (processes[j].arrivalTime == time &&
              processes[j].remainingTime > 0) {
            enqueue(&q0_queue, &processes[j]);
            processes[j].priority = 0;
          }
        }
      } else {
        printf("something bad\n");
        break;
      }
    }

    // demote the process to a lower priority queue if it still has time left
    if (current_process->remainingTime > 0) {
      current_process->priority++;

      if (current_process->priority == 1) {
        enqueue(&q1_queue, current_process);
      } else if (current_process->priority == 2) {
        enqueue(&q2_queue, current_process);
      } else if (current_process->priority == 3) {
        enqueue(&q3_queue, current_process);
      } else {
        enqueue(&q3_queue, current_process);
      }
    }
  }
  fclose(feedback);
}

int main(int argc, char **argv) {
  input = fopen(argv[1], "r");

  if (input == NULL) {
    printf("could not open file\n");
    return 1;
  }

  Process processes[MAX_SIZE];
  int n = 0;

  // read data from file
  while (fscanf(input, "%s %d %d", processes[n].processID,
                &processes[n].arrivalTime, &processes[n].serviceTime) != EOF) {
    processes[n].remainingTime = processes[n].serviceTime;
    n++;
  }

  // run algorithms
  Process processes_fcfs[MAX_SIZE];
  memcpy(processes_fcfs, processes, sizeof(Process) * n);
  fcfs_algorithm(n, processes_fcfs);

  Process processes_srt[MAX_SIZE];
  memcpy(processes_srt, processes, sizeof(Process) * n);
  srt_algorithm(n, processes_srt);

  Process processes_spn[MAX_SIZE];
  memcpy(processes_spn, processes, sizeof(Process) * n);
  spn_algorithm(n, processes_spn);

  Process processes_hrrn[MAX_SIZE];
  memcpy(processes_hrrn, processes, sizeof(Process) * n);
  hrrn_algorithm(n, processes_hrrn);

  Process processes_rr_10[MAX_SIZE];
  memcpy(processes_rr_10, processes, sizeof(Process) * n);
  rr_algorithm(n, processes_rr_10, 10);

  Process processes_rr_40[MAX_SIZE];
  memcpy(processes_rr_40, processes, sizeof(Process) * n);
  rr_algorithm(n, processes_rr_40, 40);

  Process processes_feedback[MAX_SIZE];
  memcpy(processes_feedback, processes, sizeof(Process) * n);
  feedback_algorithm(n, processes_feedback);

  fclose(input);
  return 0;
}
