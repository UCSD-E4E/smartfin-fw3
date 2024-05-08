import re
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


def parse_logs(filename):
    tasks = []
    with open(filename, 'r') as file:
        for line in file:
            if 'Next event to execute:' in line:
                match = re.search(r'(\w+) at time (\d+)\|(\d+)\|(\d+)', line)
                if match:
                    label = match.group(1)
                    start_time = int(match.group(3))
                    end_time = int(match.group(4))
                    duration = end_time - start_time
                    tasks.append({'label': label, 'start': start_time, 'duration': duration})
    return tasks


log_filename = 'log.txt'


tasks = parse_logs(log_filename)


plot_gantt(tasks, "Task Execution Gantt Chart")
