# os-scheduler

## Requirements

Install latest version of [C](https://installc.org/).

Install latest version of [Python](https://www.python.org/downloads/).

## Installation

Create a virtual environment for Python : 
```
python -m venv /path/to/new/virtual/environment
```

Activate it following the [documentation](https://docs.python.org/3/library/venv.html).

Install the python libraries : 
```
pip install -r requirements.txt
```

## Tasks pipelining

### Generate tasks
Write the following command to generate data:
```
python generator.py --number_tasks {required} --poisson_number {default=5} --exp_scale {default=20} --display_graphics {default=False}
```

### Simulate the scheduler on the tasks


Compile the c file with the following command :
```bash
gcc simulator.c -o simulator
```

Launch the created simulator file : 
```bash
./simulator
```

Choose beetween methods :
- 1 : FCFS
- 2 : RR
- 3 : Pr
- 4 : SRTF

The simulator programm will now handle the generated data with the corresponding algorithm

./execution.csv is created and contains for each task their respective 
- process id
- turnaround time
- waiting time
- the number of times it was pre-empted

./performance.cvs is created and acts as a summary of the chosen algorithm with it's 
- total time
- total number of context switches 
- total time spent on context switching

### Analyse the scheduler performances

For this step, several schedule simulations are required 
- 10 tasks
- 50 tasks
- 100 tasks
- 500 tasks
- 1000 tasks

One simulation need to be ran for each available algorithm.

Their execution.csv and performance.csv files need to be saved as follow 

```./outputs/execution/{algorithm}/execution-{number-of-tasks}-tasks-{algorithm}.csv```

and

```./outputs/performance/{algorithm}/performance-{number-of-tasks}-tasks-{algorithm}.csv```

#### As example

```./outputs/execution/fcfs/execution-10-tasks-fcfs.csv```

and 

```./outputs/performance/fcfs/performance-10-tasks-fcfs.csv```

Once that's done, run the command ```python stats.py``` to generate the graphs!

## Code overview 

### [generator.py](https://github.com/MadeInShineA/os-scheduler/blob/master/generator.py)

#### Code sample

```python
@click.command()
@click.option("--number_tasks", required=True, type=int)
@click.option("--poisson_number", default=5, type=int)
@click.option("--exp_scale", default=20, type=int)
@click.option("--display_graphics", default=False, type=bool)
def generate_file(number_tasks, poisson_number, exp_scale, display_graphics):
    res_filepath = pathlib.Path(__file__).parent.resolve() / f"./tasks.csv"
    priorities_weights = [0.05, 0.25, 0.7]
    priorities = [1, 2, 3]

    poisson_distribution = np.random.poisson(poisson_number, number_tasks)
    exp_distribution = stats.expon.rvs(scale=exp_scale, size=number_tasks) + 10
    exp_distribution = np.ndarray.round(exp_distribution / 10) * 10

    # TODO Ajust filepath
    with open(res_filepath, "w") as file:
        for i in range(number_tasks):
            id = str(i + 1)
            arrival_time = str(sum(poisson_distribution[0 : i + 1]))
            execution_time = int(exp_distribution[i])
            priority = str(random.choices(priorities, priorities_weights)[0])

            file.write(f"{id} {arrival_time} {execution_time} {priority}\n")
```


#### Explanation

This script generates a task set for the scheduler based on user-provided parameters and writes the tasks to a `tasks.csv` file. Here's an overview of the key options:
- `number_tasks`: Specifies the number of tasks to generate (required).
- `poisson_number`: Controls the Poisson distribution, which simulates task arrival rates. The default value is `5`.
- `exp_scale`: Defines the exponential distribution's scale parameter, used to determine task execution times. The default value is `20`.
- `display_graphics`: If `True`, generates visualizations for the task data. The default is `False`.

The generator uses a Poisson distribution for the tasks' arrival times to simulate randomness in their initiation. The execution times follow an exponential distribution, which ensures that some tasks will take longer than others. Additionally, tasks are assigned a priority from 1 to 3, using predefined weights to favor lower-priority tasks.



### [simulator.c](https://github.com/MadeInShineA/os-scheduler/blob/master/simulator.c)

#### Code sample

```C
int main() {
  char filepath[] = "./tasks.csv";
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
```


#### Explanation

This file contains the main program that simulates different CPU scheduling algorithms. It reads the tasks from a file and executes one of the following scheduling methods, based on user input:
1.  **FCFS (First Come First Serve)**: Tasks are executed in the order they arrive.
2. **RR (Round Robin)**: Tasks are executed in a circular queue with a fixed time quantum.
3. **Pr (Priority)**: Tasks are scheduled based on their priority, with preemption if a higher-priority task arrives.
4. **SRTF (Shortest Remaining Time First)**: Tasks with the shortest remaining execution time are given priority.

The program reads the `tasks.csv` file using `get_tasks_from_file`, simulates the chosen scheduling algorithm, and writes performance metrics (e.g., total time, context switches) to the output files. The program also ensures memory cleanup using `free_data`.


### [stats.py](https://github.com/MadeInShineA/os-scheduler/blob/master/stats.py)

#### Code sample

```Python
class ScheduleAlgorithm:
  def __init__(self, name, data_numbers, metrics) -> None:
    self.name = name
    self.turnaround_times = {key:{'data': [], 'metrics': {}} for key in data_numbers}
    for number in data_numbers:
      for metric in metrics:
        self.turnaround_times[number]["metrics"][metric] = None
    

    self.waiting_times = {key:{'data': [], 'metrics': {}} for key in data_numbers}
    
    for number in data_numbers:
      for metric in metrics:
        self.waiting_times[number]["metrics"][metric] = None
    
    
    self.pre_empted_numbers = {key:[] for key in data_numbers}

    self.total_time_ctx_switch = {key:[] for key in data_numbers}
    self.total_time = {key:[] for key in data_numbers}

  def __str__(self) -> str:
    return f'Algorithm : {self.name}\n \t turnaround_times : {self.turnaround_times}\n\t waiting_times : {self.waiting_times}\n\t total_time_ctx_switch : {self.total_time_ctx_switch}\n\t total_time : {self.total_time}'

    
if __name__ == "__main__":

  methods = ['fcfs', 'rr', 'pr' , 'srtf']
  data_numbers = [10, 50, 100, 500, 1000]
  metrics = ['mean', 'std', 'percentile_50', 'percentile_90']

  algorithms = create_algorithms(methods, data_numbers, metrics)

  plot_metrics(algorithms, data_numbers, metrics, 'Turnaround Time')
  plot_metrics(algorithms, data_numbers, metrics, 'Waiting Time')
  plot_total_times(algorithms, data_numbers)
```

#### Explanation

This script analyzes the results of multiple scheduler simulations by reading the output `.csv` files and generating comparative graphs for different scheduling algorithms. It evaluates key metrics like:
- **Turnaround Time**: The time from task arrival to its completion.
- **Waiting Time**: The time a task spends waiting in the queue before execution.
- **Preemptions**: The number of times a task was interrupted during execution.
- **Context Switch Time**: The total time spent switching between tasks.

The `ScheduleAlgorithm` class stores the data for each algorithm (FCFS, RR, Priority, SRTF) and performs calculations on multiple task sets (e.g., 10, 50, 100, 500, 1000 tasks). Metrics like mean, standard deviation, and percentiles are calculated for both turnaround and waiting times. The script then uses the `plot_metrics` and `plot_total_times` functions to visualize these metrics across different task loads and scheduling algorithms.

Finally, `stats.py` generates comparative graphs to assess the performance of each scheduling method under varying task loads.

## Additional files

A small comparison of the different scheduling algorithms can be found inside the [performance.pdf](https://github.com/MadeInShineA/os-scheduler/blob/master/performance.pdf)
