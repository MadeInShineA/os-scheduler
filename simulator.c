#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RR_QUANTUM 2
#define CNTXT_SWITCH 1
#define MAX_TASK_SIZE 1024

typedef struct task_performance {
  uint64_t process_id;
  uint64_t turnaround_time;
  uint64_t waiting_time;
  uint64_t pre_empted_number;
} task_performance_t;

typedef struct task {
  uint64_t process_id;
  uint64_t arrival_time;
  uint64_t execution_time;
  uint64_t executed_time;
  uint8_t priority;
  struct task *next_task;
  task_performance_t task_performance;
} task_t;

typedef struct scheduler_performance {
  uint64_t total_time;
  uint64_t context_switches_total_number;
  uint64_t context_switch_total_time;
  // TODO need of * ?
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

char* getFilePath() {
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
}

scheduler_performance_t *fcfs(task_t *head) {
  scheduler_performance_t *res =
      (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int time_waited = 0;
  int time_handeling = 0;
  task_t *p_task = head;

  while (p_task != NULL) {
    task_performance_t *task_performance =
        (task_performance_t *)malloc(sizeof(task_performance_t));

    task_performance->process_id = p_task->process_id;
    task_performance->pre_empted_number = 0;

    if (seconds < p_task->arrival_time) {
      printf("Waiting on PID | %ld\n", p_task->process_id);
      task_performance->waiting_time = 0;
      seconds = p_task->arrival_time;
    } else {
      task_performance->waiting_time = seconds - p_task->arrival_time;
    }

    printf("Handeling PID | %ld\n", p_task->process_id);
    seconds += p_task->execution_time;

    printf("Current time : %d\n", seconds);

    task_performance->turnaround_time =
        p_task->execution_time + task_performance->waiting_time;
    res->task_performance_array[task_counter] = task_performance;
    task_counter += 1;
    p_task = p_task->next_task;
  }

  res->context_switches_total_number = 0;
  res->context_switch_total_time = 0;
  res->total_time = seconds;

  return res;
}

scheduler_performance_t *rr(task_t *head, uint8_t quantum_time) {
  scheduler_performance_t *res =
      (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  uint64_t finished_task_counter = 0;
  uint64_t task_counter = 0;
  uint64_t seconds = 0;
  uint64_t time_waited = 0;
  uint64_t time_handeling = 0;
  task_t *queue_head = head;
  task_t *queue_pointer = queue_head;
  uint64_t total_context_switches = 0;


  while (queue_pointer != NULL) {
    if (seconds < queue_pointer->next_task->arrival_time) {
      // the next packet hasn't arrived so we switch between packets in the queue

      while (queue_pointer != NULL && (queue_pointer->arrival_time > seconds || queue_pointer->executed_time >= queue_pointer->execution_time)) {
        queue_pointer = queue_pointer->next_task;
        if (queue_pointer == NULL) {
            queue_pointer = queue_head;
        }
      }

      if (queue_pointer == NULL) {
        printf("Break");
        break;  // No more tasks to process
      }
      
      task_t current_handled_task = *queue_pointer;
      if (seconds % quantum_time == 0) {
        printf("Switching task, current time: %ld\n", seconds);
        
        current_handled_task.task_performance.pre_empted_number += 1;

        total_context_switches += 1;

        // Check if we are at the end of the queue, therefore reseting to head
        if(seconds < queue_pointer->next_task->arrival_time){
          queue_pointer = queue_head;
        }else{
          queue_pointer = queue_pointer->next_task;
        }
        printf("Switched from task %ld to %ld\n", current_handled_task.process_id, queue_pointer->next_task->process_id);

      } else {
        current_handled_task.executed_time += 1;
        if(current_handled_task.executed_time >= current_handled_task.execution_time){
          // The task leaves the queue
          // current_handled_task->task_performance->turnaround_time is currently set at the arrival time
          current_handled_task.task_performance.turnaround_time = seconds - current_handled_task.task_performance.turnaround_time;
          current_handled_task.task_performance.waiting_time = current_handled_task.task_performance.pre_empted_number * quantum_time;
          res->task_performance_array[finished_task_counter] = &current_handled_task.task_performance;

          finished_task_counter ++;
          printf("Finished task: %ld\n", current_handled_task.process_id);
        }
      }
    } else {
      // Add the arriving packet inside the queue
      queue_pointer = queue_pointer->next_task;    
      printf("Added task : %ld to the queue\n", queue_pointer->process_id);
      queue_pointer->task_performance.turnaround_time = seconds;
    }
    seconds++;
  }

  res->total_time = seconds;
  res->context_switches_total_number = total_context_switches;
  res->context_switch_total_time = total_context_switches * CNTXT_SWITCH;
  return res;
}

scheduler_performance_t *pr(task_t *head) {
  scheduler_performance_t *res =
      (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int time_waited = 0;
  int time_handeling = 0;
  int current_priority = head->priority;
  task_t *p_first_task_not_execute = head;
  task_t *p_current_task = head;
  task_t *p_tmp = head;

  while (p_first_task_not_execute != NULL) {
    if(p_first_task_not_execute->execution_time == 0){
      while (p_first_task_not_execute != NULL && p_first_task_not_execute->execution_time == 0){
        p_first_task_not_execute = p_first_task_not_execute->next_task;
      }  
      
      if (p_first_task_not_execute == NULL) {
        break;
      }    
    }
    
    while(p_tmp != NULL && p_tmp->arrival_time <= seconds){
      if(p_tmp->priority > current_priority && p_tmp->execution_time != 0){
        current_priority = p_tmp->priority;
        p_current_task = p_tmp;

        if(p_current_task->priority == 3){
          break;
        }
      }
      p_tmp = p_tmp->next_task;
    }

    if(p_current_task->execution_time > p_current_task->executed_time){
      p_current_task->executed_time += 1;
    }

    p_tmp = p_first_task_not_execute;
    current_priority = p_first_task_not_execute->priority;
    seconds++;

    if(p_current_task->execution_time <= p_current_task->executed_time){
      p_current_task->task_performance.turnaround_time = seconds - p_current_task->arrival_time;
      p_current_task->task_performance.waiting_time = p_current_task->task_performance.turnaround_time - p_current_task->execution_time;
    }
  }

  res->context_switches_total_number = 0;
  res->context_switch_total_time = 0;
  res->total_time = seconds;

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
    temp->executed_time = 0;
    temp->priority = atoi(p_priority);
    temp->next_task = NULL;

    // Initialize task_performance
    temp->task_performance.process_id = temp->process_id;
    temp->task_performance.turnaround_time = 0;
    temp->task_performance.waiting_time = 0;
    temp->task_performance.pre_empted_number = 0;

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

  scheduler_performance_t *res = rr2(head_task, RR_QUANTUM);
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
