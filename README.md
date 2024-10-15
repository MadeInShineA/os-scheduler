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

## Launching project

### Generate datas

Write the following command to generate datas :
```
python generator.py --number_tasks {number of tasks} --poisson_number {poisson number (default=5)} --exp_scale {exponential scale (default=20)} --display_graphics {(default=False)}
```

### scheduler simulator

# TODO REGARDDER QUOI METTRE ICI !!!!

Compile the c file with the following command :
```
gcc simulator.c -o simulator
```

Launch the created simulator file : 
```
./simulator
```

Choose beetween methods :
- 1 : FCFS
- 2 : RR
- 3 : Pr
- 4 : SRTF

### Analyse datas
