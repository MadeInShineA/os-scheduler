import matplotlib.pyplot as plt
import numpy as np

import random

#TODO replace with read from console
number_tasks = 1000
tasks = []
priority = [1,2,3]
priority_weight = [0.05, 0.25, 0.7]

for i in range(number_tasks):
  task = {}
  task['id'] = i+1
  task['priority'] = random.choices(priority, priority_weight)[0]
  task['arrival_time'] = np.random.poisson()


  tasks.append(task)

print(tasks)
