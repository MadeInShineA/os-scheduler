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
  task_performance_t task_performance_array[MAX_TASK_SIZE];
} scheduler_performance_t;


uint8_t get_scheduler_type() {
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


task_t* get_tasks_from_file(char* filepath){
  FILE *file = fopen(filepath, "r");
  char line[256];

  task_t *tasks_head = NULL;
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

    if (tasks_head == NULL) {
      tasks_head = temp;
    } else {
      p_task = tasks_head;
      while (p_task->next_task != NULL) {
        p_task = p_task->next_task;
      }
      p_task->next_task = temp;
    }
  }
  fclose(file);

  return tasks_head;
}


void write_output(scheduler_performance_t *scheduler_performance) {
  FILE *execution_file = fopen("./execution-1000-tasks-srtf.csv", "w");
  for (int i = 0; i < MAX_TASK_SIZE; i++) {
    // Check for sentinel value
    if (scheduler_performance->task_performance_array[i].process_id == 0 && 
        i != 0) {
      break;  // Stop if we encounter the first zero after the first element
    }

    task_performance_t task_performance = scheduler_performance->task_performance_array[i];

    fprintf(execution_file, "%lu %lu %lu %lu\n", 
            task_performance.process_id,
            task_performance.turnaround_time, 
            task_performance.waiting_time,
            task_performance.pre_empted_number);
  }
  fclose(execution_file);

  FILE *performance_file = fopen("./performance-1000-tasks-srtf.csv", "w");

  fprintf(performance_file, "total_time: %ld\n",
          scheduler_performance->total_time);
  fprintf(performance_file, "total_nr_ctxt_switch: %ld\n",
          scheduler_performance->context_switches_total_number);
  fprintf(performance_file, "total_time_ctx_switch: %ld\n",
          scheduler_performance->context_switch_total_time);

  fclose(performance_file);
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

    // task_performance->process_id = p_task->process_id;
    p_task->task_performance.pre_empted_number = 0;

    if (seconds < p_task->arrival_time) {
      printf("Waiting on PID | %ld\n", p_task->process_id);
      p_task->task_performance.waiting_time = 0;
      seconds = p_task->arrival_time;
    } else {
      p_task->task_performance.waiting_time = seconds - p_task->arrival_time;
    }

    printf("Handeling PID | %ld\n", p_task->process_id);
    seconds += p_task->execution_time;

    printf("Current time : %d\n", seconds);

    p_task->task_performance.turnaround_time =
        p_task->execution_time + p_task->task_performance.waiting_time;
    res->task_performance_array[task_counter] = p_task->task_performance;
    task_counter += 1;
    p_task = p_task->next_task;
  }

  res->context_switches_total_number = 0;
  res->context_switch_total_time = 0;
  res->total_time = seconds;

  return res;
}


scheduler_performance_t *rr(task_t *head, uint8_t quantum_time, uint8_t context_switch_time) {
  scheduler_performance_t *res = (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  uint64_t finished_task_counter = 0;
  uint64_t seconds = 0;
  task_t *queue_head = head;
  task_t *queue_pointer = queue_head;
  uint64_t total_context_switches = 0;
  task_t *previous_task = NULL;  // Track the previous task to detect context switches
  uint64_t total_tasks = 0;
  task_t *iter = head;
  while (iter != NULL) {
    total_tasks++;
    iter = iter->next_task;
  }


  while (finished_task_counter < total_tasks) {
    bool task_executed = false;
    bool any_task_ready = false; // To track if at least one task is ready

    // Look for a task that is ready to execute (has arrived and not finished)
    task_t *iter = queue_head;
    while (iter != NULL) {
      if (iter->arrival_time <= seconds && iter->executed_time < iter->execution_time) {
        // We found a task that is ready to execute
        any_task_ready = true;
        task_executed = true;

        // If the task switched, count a context switch (only if there are other tasks to switch to)
        if (previous_task != iter && previous_task != NULL) {
          if(previous_task->executed_time < previous_task->execution_time){
            previous_task->task_performance.pre_empted_number ++;
          }
          seconds += context_switch_time;
          total_context_switches++;
        }
        previous_task = iter;

        printf("Executing task: %ld at time: %ld\n", iter->process_id, seconds);

        // Execute for a quantum or until the task finishes
        if (iter->executed_time + quantum_time < iter->execution_time) {
          iter->executed_time += quantum_time;
          seconds += quantum_time;
          printf("Quantum used. Task %ld executed for %u seconds. Total executed: %ld\n",
                 iter->process_id, quantum_time, iter->executed_time);
        } else {
          // Task will finish in less than a quantum
          uint64_t remaining_time = iter->execution_time - iter->executed_time;
          iter->executed_time = iter->execution_time;
          seconds += remaining_time;
          printf("Task %ld finished at time: %ld\n", iter->process_id, seconds);

          // Mark task as completed, store performance data
          iter->task_performance.turnaround_time = seconds - iter->arrival_time;
          iter->task_performance.waiting_time = iter->task_performance.turnaround_time - iter->execution_time;
          res->task_performance_array[finished_task_counter] = iter->task_performance;
          finished_task_counter++;
        }
      }
      iter = iter->next_task;
    }

    // If no task was executed, increment time or jump to next task arrival
    if (!task_executed) {
      uint64_t next_arrival_time = UINT64_MAX;
      task_t *iter = queue_head;

      while (iter != NULL) {
        if (iter->arrival_time > seconds && iter->arrival_time < next_arrival_time) {
          next_arrival_time = iter->arrival_time;
        }
        iter = iter->next_task;
      }
            
      // If we found a next task arrival in the future, jump to that time
      if (next_arrival_time != UINT64_MAX) {
        printf("Jumping to the next task arrival at: %ld\n", next_arrival_time);
        seconds = next_arrival_time;
      } else {
        // Increment time by 1 second otherwise
        seconds++;
      }
    }

    // Move the queue pointer forward
    queue_pointer = queue_pointer->next_task;
    if (queue_pointer == NULL) {
      queue_pointer = queue_head;  // Wrap around to the start of the queue
    }

    // Prevent unnecessary context switches when only one task is available to compute
    if (!any_task_ready && previous_task != NULL && previous_task->executed_time < previous_task->execution_time) {
      queue_pointer = previous_task;
    }
  }

  // Final performance data
  res->context_switches_total_number = total_context_switches;
  res->context_switch_total_time = total_context_switches * context_switch_time;
  res->total_time = seconds;

  return res;
}


scheduler_performance_t *pr(task_t *head, uint8_t context_switch_time) {
  scheduler_performance_t *res = (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int total_context_switches = 0;
  task_t *p_first_task_not_execute = head;
  task_t *p_current_task = NULL;

  while (p_first_task_not_execute != NULL) {
    task_t *p_tmp = p_first_task_not_execute;
    task_t *highest_priority_task = NULL;

    while (p_tmp != NULL && p_tmp->arrival_time <= seconds) {
      if (p_tmp->executed_time < p_tmp->execution_time) {
        if (highest_priority_task == NULL || p_tmp->priority < highest_priority_task->priority) {
          highest_priority_task = p_tmp;
        }
      }
      p_tmp = p_tmp->next_task;
    }

    if (highest_priority_task == NULL) {
      seconds++;
      continue;
    }

    if (p_current_task != highest_priority_task) {
      if (p_current_task != NULL && p_current_task->executed_time < p_current_task->execution_time) {
        p_current_task->task_performance.pre_empted_number++;
        total_context_switches++;
        res->context_switch_total_time += context_switch_time;
        seconds += context_switch_time;
      }
      p_current_task = highest_priority_task;
      printf("Switched to task %ld at time %d\n", p_current_task->process_id, seconds);
    }

    p_current_task->executed_time++;
    seconds++;

    if (p_current_task->executed_time >= p_current_task->execution_time) {
      p_current_task->task_performance.turnaround_time = seconds - p_current_task->arrival_time;
      p_current_task->task_performance.waiting_time = p_current_task->task_performance.turnaround_time - p_current_task->execution_time;
      res->task_performance_array[task_counter] = p_current_task->task_performance;
      task_counter++;

      printf("Task %ld finished at time %d\n", p_current_task->process_id, seconds);

      p_first_task_not_execute = head;
      while (p_first_task_not_execute != NULL && p_first_task_not_execute->executed_time >= p_first_task_not_execute->execution_time) {
        p_first_task_not_execute = p_first_task_not_execute->next_task;
      }
    }
  }

  res->total_time = seconds;
  res->context_switches_total_number = total_context_switches;
  res->context_switch_total_time = total_context_switches * context_switch_time;
  return res;
}


scheduler_performance_t *srtf(task_t *head, uint8_t context_switch_time) {
  scheduler_performance_t *res = (scheduler_performance_t *)malloc(sizeof(scheduler_performance_t));
  int task_counter = 0;
  int seconds = 0;
  int total_context_switches = 0;
  task_t *p_first_task_not_execute = head;
  task_t *p_current_task = NULL;

  while (p_first_task_not_execute != NULL) {
    task_t *p_tmp = p_first_task_not_execute;
    task_t *highest_priority_task = NULL;

    while (p_tmp != NULL && p_tmp->arrival_time <= seconds) {
      if (p_tmp->executed_time < p_tmp->execution_time) {
        if (highest_priority_task == NULL || p_tmp->priority < highest_priority_task->priority) {
          highest_priority_task = p_tmp;
        }
      }
      p_tmp = p_tmp->next_task;
    }

    if (highest_priority_task == NULL) {
      seconds++;
      continue;
    }

    if (p_current_task != highest_priority_task) {
      if (p_current_task != NULL && p_current_task->executed_time < p_current_task->execution_time) {

        p_current_task->task_performance.pre_empted_number++;
        total_context_switches++;
        res->context_switch_total_time += context_switch_time;
        seconds += context_switch_time;
      }
      p_current_task = highest_priority_task;
      printf("Switched to task %ld at time %d\n", p_current_task->process_id, seconds);
    }

    p_current_task->executed_time++;
    seconds++;

    if (p_current_task->executed_time >= p_current_task->execution_time) {
      p_current_task->task_performance.turnaround_time = seconds - p_current_task->arrival_time;
      p_current_task->task_performance.waiting_time = p_current_task->task_performance.turnaround_time - p_current_task->execution_time;
      res->task_performance_array[task_counter] = p_current_task->task_performance;
      task_counter++;

      printf("Task %ld finished at time %d\n", p_current_task->process_id, seconds);

      p_first_task_not_execute = head;
      while (p_first_task_not_execute != NULL && p_first_task_not_execute->executed_time >= p_first_task_not_execute->execution_time) {
        p_first_task_not_execute = p_first_task_not_execute->next_task;
      }
    }
  }

  res->total_time = seconds;
  res->context_switches_total_number = total_context_switches;
  res->context_switch_total_time = total_context_switches * context_switch_time;
  return res;
}


void free_tasks(task_t* tasks_head){
  task_t *current_task = tasks_head;
  task_t *next_task;

  while (current_task != NULL) {
    next_task = current_task->next_task;
    free(current_task);
    current_task = next_task;
  }
}


void free_data(task_t* tasks_head, scheduler_performance_t* res){
  free_tasks(tasks_head);
  free(res);
}


int main() {
  char filepath[] = "./data-1000-tasks.csv";
  uint8_t scheduler_type = get_scheduler_type();
  task_t* tasks_head = get_tasks_from_file(filepath);
  scheduler_performance_t *res = NULL;

  switch (scheduler_type)
  {
  case 1:
    res = fcfs(tasks_head);
    break;
  case 2:
    res = rr(tasks_head, RR_QUANTUM, CNTXT_SWITCH);
    break;
  case 3:
    res = pr(tasks_head, CNTXT_SWITCH);
    break;
  case 4:
    res = srtf(tasks_head, CNTXT_SWITCH);
    break;
  default:
    printf("Unknown scheduler_type, exiting\n");
    // Exit (I'm a teapot)
    exit(418);
    break;
  }
  write_output(res);
  free_data(tasks_head, res);

  return 0;
}
