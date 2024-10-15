import matplotlib.pyplot as plt
import numpy as np

import itertools
import sys

methods = ['fcfs', 'rr', 'pr' , 'srtf']
data_numbers = [10, 50, 100, 500, 1000]
metrics = ['mean', 'std', 'percentile_50', 'percentile_90']

filenames = [f'{method}/execution-{number}-tasks-{method}.csv' for (method, number) in itertools.product(methods, data_numbers)]


class ScheduleAlgorithm:
  def __init__(self, name, data_numbers) -> None:
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

def calculate_stats(times):
  avg = np.mean(times)
  stddev = np.std(times)
  percentile_50 = np.percentile(times, 50)
  percentile_90 = np.percentile(times, 90)
  return avg, stddev, percentile_50, percentile_90

def get_task_data_from_line(task_line):
  task_data = task_line.split(' ')
  turnaround_time = int(task_data[1])
  waiting_time = int(task_data[2])
  pre_empted_numbers = int(task_data[3].strip())

  return turnaround_time, waiting_time, pre_empted_numbers


algorithms = []

for method in methods:
  algorithm = ScheduleAlgorithm(method, data_numbers)
  for number in data_numbers:
    execution_filename = f'./outputs/execution/{method}/execution-{number}-tasks-{method}.csv'
    with open(execution_filename, 'r') as execution_file:
      for task_line in execution_file:
        turnaround_time, waiting_time, pre_empted_numbers = get_task_data_from_line(task_line)
        algorithm.turnaround_times[number]['data'].append(turnaround_time)
        algorithm.waiting_times[number]['data'].append(waiting_time)
        algorithm.pre_empted_numbers[number].append(pre_empted_numbers)

    current_metric_waiting = calculate_stats(algorithm.waiting_times[number]['data'])
    current_metric_turnaround = calculate_stats(algorithm.turnaround_times[number]['data'])

    for i, metric in enumerate(metrics):
      algorithm.waiting_times[number]['metrics'][metric] = current_metric_waiting[i]
      algorithm.turnaround_times[number]['metrics'][metric] = current_metric_turnaround[i]
    
    performance_filename = f'./outputs/performance/{method}/performance-{number}-tasks-{method}.csv'
    with open(performance_filename, 'r') as performance_file:
      for i, line in enumerate(performance_file):
        if i == 0:
          algorithm.total_time[number] = line.strip().split(' ')[1]
        if i == 2:
          algorithm.total_time_ctx_switch[number] = line.strip().split(' ')[1]

  algorithms.append(algorithm)


def plot_metrics(algorithms, data_numbers, metrics, metric_name):
    num_methods = len(algorithms)
    num_tasks = len(data_numbers)
    num_metrics = len(metrics)
    
    # Couleurs pour les graphiques
    colors = plt.get_cmap('tab10')

    fig, axes = plt.subplots(1, num_tasks, figsize=(15, 6), sharey=True)
    fig.suptitle(f'{metric_name} comparison for each scheduling algorithm', fontsize=16)

    for idx, number in enumerate(data_numbers):
        ax = axes[idx]
        metric_values = {metric: [] for metric in metrics}

        # Récupération des données pour chaque algorithme
        for algorithm in algorithms:
            for metric in metrics:
                if metric_name == 'Turnaround Time':
                    metric_values[metric].append(algorithm.turnaround_times[number]['metrics'][metric])
                elif metric_name == 'Waiting Time':
                    metric_values[metric].append(algorithm.waiting_times[number]['metrics'][metric])

        x = np.arange(num_methods)  # positions des algorithmes sur l'axe x

        # Tracé des barres pour chaque métrique
        width = 0.15  # Largeur des barres
        for i, metric in enumerate(metrics):
            ax.bar(x + i * width, metric_values[metric], width=width, label=metric, color=colors(i))

        # Configuration de l'axe
        ax.set_title(f'{number} Tasks')
        ax.set_xticks(x + width * (num_metrics - 1) / 2)
        ax.set_xticklabels([algorithm.name.upper() for algorithm in algorithms])
        ax.set_xlabel('Scheduling Algorithm')
        ax.set_ylabel(metric_name)
        ax.legend(title='Metrics')

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()

def plot_total_times(algorithms, data_numbers):
    num_methods = len(algorithms)
    num_tasks = len(data_numbers)

    # Couleurs pour les graphiques
    colors = plt.get_cmap('Set2')

    fig, axes = plt.subplots(1, num_tasks, figsize=(15, 6), sharey=True)
    fig.suptitle('Total Time and Context Switching Time for each Scheduling Algorithm', fontsize=16)

    for idx, number in enumerate(data_numbers):
        ax = axes[idx]
        total_times = []
        ctx_switch_times = []

        # Récupération des données pour chaque algorithme
        for algorithm in algorithms:
            total_times.append(float(algorithm.total_time[number]))
            ctx_switch_times.append(float(algorithm.total_time_ctx_switch[number]))

        x = np.arange(num_methods)  # positions des algorithmes sur l'axe x

        # Tracé des barres
        width = 0.35  # Largeur des barres
        ax.bar(x - width / 2, total_times, width=width, label='Total Time', color=colors(0))
        ax.bar(x + width / 2, ctx_switch_times, width=width, label='Context Switch Time', color=colors(1))

        # Configuration de l'axe
        ax.set_title(f'{number} Tasks')
        ax.set_xticks(x)
        ax.set_xticklabels([algorithm.name.upper() for algorithm in algorithms])
        ax.set_xlabel('Scheduling Algorithm')
        ax.set_ylabel('Time (ms)')
        ax.legend()

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()


# Appels de la fonction pour tracer les graphiques

plot_metrics(algorithms, data_numbers, metrics, 'Turnaround Time')
plot_metrics(algorithms, data_numbers, metrics, 'Waiting Time')
plot_total_times(algorithms, data_numbers)
