import matplotlib.pyplot as plt
import numpy as np


def plot_gantt(tasks, title):
    fig, ax = plt.subplots(figsize=(10, 3))
    task_dict = {}

   
    for task in tasks:
        if task['label'] not in task_dict:
            task_dict[task['label']] = []
        task_dict[task['label']].append((task['start'], task['duration']))

    
    y_pos = range(len(task_dict))
    colors = ['skyblue', 'lightgreen', 'salmon', 'grey']  
    color_idx = 0

    for i, (label, segments) in enumerate(task_dict.items()):
        for start, duration in segments:
            ax.barh(i, duration, left=start, color=colors[color_idx % len(colors)])
            ax.text(start + duration / 2, i, f"{label} ({start} - {start+duration})",
                    va='center', ha='center', color='black')
        color_idx += 1

    ax.set_yticks(y_pos)
    ax.set_yticklabels(task_dict.keys())
    ax.set_xlabel('Time (seconds)')
    ax.set_title(title)
    ax.grid(True)

    plt.show()
    #plt.save(str(title) + ".jpg")

tasks_intended = [
    {'label': 'A', 'start': 0, 'duration': 40},
    {'label': 'B', 'start': 50, 'duration': 20},
    {'label': 'C', 'start': 80, 'duration': 70},
    {'label': 'A', 'start': 200, 'duration': 40},
    {'label': 'B', 'start': 250, 'duration': 30},
    {'label': 'C', 'start': 280, 'duration': 70}
]
#plot_gantt(tasks_intended, "Intended")

tasks_delay1 = [
    {'label': 'A', 'start': 0, 'duration': 60},
    {'label': 'B', 'start': 60, 'duration': 20},
    {'label': 'C', 'start': 80, 'duration': 70},
    {'label': 'A', 'start': 200, 'duration': 40},
    {'label': 'B', 'start': 250, 'duration': 30},
    {'label': 'C', 'start': 280, 'duration': 70}
]
#plot_gantt(tasks_delay1, "Delay but no overlap")

tasks_delay2 = [
    {'label': 'A', 'start': 0, 'duration': 60},
    {'label': 'B', 'start': 60, 'duration': 20},
    {'label': 'C', 'start': 80, 'duration': 140},
    {'label': 'B', 'start': 250, 'duration': 30},
    {'label': 'C', 'start': 280, 'duration': 70}
    {'label': 'A', 'start': 350, 'duration': 40},
]
plot_gantt(tasks_delay2, "Delay but overlap 1")
