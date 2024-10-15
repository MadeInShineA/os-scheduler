import pathlib
import random
import click
import matplotlib.pyplot as plt
import numpy as np
from scipy import stats


@click.command()
@click.option("--number_tasks", required=True, type=int)
@click.option("--poisson_number", default=5, type=int)
@click.option("--exp_scale", default=20, type=int)
@click.option("--display_graphics", default=False, type=bool)
def generate_file(number_tasks, poisson_number, exp_scale, display_graphics):
    res_filepath = pathlib.Path(__file__).parent.resolve() / f"./data-{number_tasks}-tasks.csv"
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

    if display_graphics:
        plt.hist(
            poisson_distribution,
            bins=range(min(poisson_distribution), max(poisson_distribution) + 1),
            density=True,
            alpha=0.6,
            color="g",
        )
        plt.title(f"Poisson distribution (lambda={poisson_number})")
        plt.xlabel("Number of events (k)")
        plt.ylabel("Probability")
        plt.show()

        plt.hist(
            exp_distribution, bins=10, density=True, alpha=0.6, color="g"
        )  # 10 bins pour un affichage plus clair
        plt.title(f"Exponential distribution (scale={exp_scale})")
        plt.xlabel("execution time")
        plt.ylabel("probability density")
        plt.show()


if __name__ == "__main__":
    generate_file()
