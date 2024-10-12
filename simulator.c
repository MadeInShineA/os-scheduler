#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RR_QUANTUM 2
#define CNTXT_SWITCH 1
#define MAX_TASK_SIZE 1024

typedef struct task {
  uint64_t process_id;
  uint64_t arrival_time;
  uint64_t execution_time;
  uint8_t priority;
  struct task *next_task;
} task_t;

typedef struct task_performance {
  uint64_t process_id;
  uint64_t turnaround_time;
  uint64_t waiting_time;
  uint64_t pre_empted_number;
} task_performance_t;

typedef struct scheduler_performance {
  uint64_t total_time;
  uint64_t context_switches_total_number;
  uint64_t context_switch_total_time;
  task_performance_t *task_performance_array[MAX_TASK_SIZE];
} scheduler_performance_t;

uint8_t getSchedulerType() {
  uint8_t scheduler_type = 0;

  while (scheduler_type == 0 || scheduler_type < 1 || scheduler_type > 4) {
    printf("Enter the scheduler type (1-4)\n");
    int input_status = scanf("%hhu", &scheduler_type);

    if (input_status != 1) {
      while (getchar() != '\n')
        ;
    }
  }
  return scheduler_type;
}

/*char* getFilePath() {
  char *filepath = (char *)malloc(256 * sizeof(char));
  if (filepath == NULL) {
      perror("Failed to allocate memory for file path");
      return NULL;
  }

  while (1) {
    printf("Enter a CSV file path: ");
    int input_status = scanf("%255s", filepath);

    if (input_status == 1) {
      if (fopen(filepath, "r")) {
      } else {
        printf("File does not exist. Please try again.\n");
      }
    } else {
      printf("Invalid input. Please try again.\n");
      while (getchar() != '\n');
    }
  }

  return filepath;
}*/

scheduler_performance_t *fcfs(task_t *head) {
  scheduler_performance_t *res =
      (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int time_waited = 0;
  int time_handeling = 0;
  task_t *task_pointer = head;

  while (task_pointer != NULL) {
    task_performance_t *task_performance =
        (task_performance_t *)malloc(sizeof(task_performance_t));

    task_performance->process_id = task_pointer->process_id;
    task_performance->pre_empted_number = 0;

    if (seconds < task_pointer->arrival_time) {
      printf("Waiting on PID | %ld\n", task_pointer->process_id);
      task_performance->waiting_time = 0;
      seconds = task_pointer->arrival_time;
    } else {
      task_performance->waiting_time = seconds - task_pointer->arrival_time;
    }

    printf("Handeling PID | %ld\n", task_pointer->process_id);
    seconds += task_pointer->execution_time;

    printf("Current time : %d\n", seconds);

    task_performance->turnaround_time =
        task_pointer->execution_time + task_performance->waiting_time;
    res->task_performance_array[task_counter] = task_performance;
    task_counter += 1;
    task_pointer = task_pointer->next_task;
  }

  res->context_switches_total_number = 0;
  res->context_switch_total_time = 0;
  res->total_time = seconds;

  return res;
}

scheduler_performance_t *rr(task_t *head) {
  scheduler_performance_t *res =
      (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int time_waited = 0;
  int time_handeling = 0;
  int doing_task = 1;
  task_t *task_pointer = head;

  while (task_pointer != NULL && !doing_task) {
    seconds++;
  }
  return res;
}

void write_output(scheduler_performance_t *scheduler_performance) {
  FILE *execution_file = fopen("./outputs/execution.csv", "w");
  int i = 0;
  while (scheduler_performance->task_performance_array[i] != NULL) {
    task_performance_t *task_performance =
        scheduler_performance->task_performance_array[i];

    fprintf(execution_file, "%lu %lu %lu %lu\n", task_performance->process_id,
            task_performance->turnaround_time, task_performance->waiting_time,
            task_performance->pre_empted_number);
    i++;
  }
  fclose(execution_file);

  FILE *performance_file = fopen("./outputs/performance.csv", "w");

  fprintf(performance_file, "total_time: %ld\n",
          scheduler_performance->total_time);
  fprintf(performance_file, "total_nr_ctxt_switch: %ld\n",
          scheduler_performance->context_switches_total_number);
  fprintf(performance_file, "total_time_ctx_switch: %ld\n",
          scheduler_performance->context_switch_total_time);

  fclose(performance_file);
}

int main() {
  uint8_t scheduler_type = getSchedulerType();
  char filepath[] = "./inputs/tasks.csv";
  FILE *file = fopen(filepath, "r");
  char line[256];

  task_t *head_task = NULL;
  task_t *temp = NULL;
  task_t *p_task = NULL;

  while (fgets(line, sizeof(line), file)) {
    char *p_process_id, *p_arrival_time, *p_execution_time, *p_priority;

    p_process_id = strtok(line, " ");
    p_arrival_time = strtok(NULL, " ");
    p_execution_time = strtok(NULL, " ");
    p_priority = strtok(NULL, " ");

    temp = (task_t *)malloc(sizeof(task_t));
    temp->process_id = atoi(p_process_id);
    temp->arrival_time = atoi(p_arrival_time);
    temp->execution_time = atoi(p_execution_time);
    temp->priority = atoi(p_priority);
    temp->next_task = NULL;

    if (head_task == NULL) {
      head_task = temp;
    } else {
      p_task = head_task;
      while (p_task->next_task != NULL) {
        p_task = p_task->next_task;
      }
      p_task->next_task = temp;
    }
  }
  fclose(file);

  /*   p_task = head_task;
    while(p_task != NULL){
      printf("PID : %li\n", p_task->process_id);
      p_task = p_task->next_task;
    } */

  scheduler_performance_t *res = fcfs(head_task);
  write_output(res);

  /* int index = 0;
  while (res->task_performance_array[index] != NULL){
    printf("Process id in sche")
  } */

  task_t *current_task = head_task;
  task_t *next_task;

  while (current_task != NULL) {
    next_task = current_task->next_task;
    free(current_task);
    current_task = next_task;
  }

  // TODO Free the scheduler_performance_t

  return 0;
}
