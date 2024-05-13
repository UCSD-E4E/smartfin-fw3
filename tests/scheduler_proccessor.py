import re
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os
import sys

def plot_gantt(tasks, title):
    fig, ax = plt.subplots(figsize=(len(tasks), 3))
    task_dict = {}

   
    for task in tasks:
        if task['label'] not in task_dict:
            task_dict[task['label']] = []
        task_dict[task['label']].append((task['start'], task['duration']))

    
    y_pos = range(len(task_dict))
    colors = ['skyblue', 'lightgreen', 'salmon', 'grey']  
    color_idx = 0
    
    
    for i, (label, segments) in enumerate(task_dict.items()):
        last_end = 0
        for start, duration in segments:
            
            ax.barh(i, duration, left=start, color=colors[color_idx % len(colors)])
            ha = 'left' if start - last_end < 300 else 'center'  
            va = 'top' if start - last_end < 300 else 'bottom'
            
            ax.text(start + duration / 2, i, f"{label} ({start} - {start+duration})",
                    verticalalignment=va, horizontalalignment=ha, color='black')
            last_end = start + duration
        color_idx += 1

    ax.set_yticks(y_pos)
    ax.set_yticklabels(task_dict.keys())
    ax.set_xlabel('Time (seconds)')
    ax.set_title(title)
    ax.grid(True)
    
    plt.subplots_adjust(bottom=0.15)
    plt.savefig(str(os.path.dirname(os.path.abspath(__file__))) + '/outputs/' + title + ".png")


def parse_logs(filename):
    tasks = []
    deployment_start_time = 0
    with open(filename, 'r') as file:
        
        for line in file:
            first_line_match = re.search(r'Deployment started at (\d+)', line)
            if first_line_match:
                deployment_start_time = int(first_line_match.group(1))
            match = re.search(r'(\w+)\|(\d+)\|(\d+)', line)
            if match:
                label = match.group(1)
                start_time = int(match.group(2)) - deployment_start_time
                end_time = int(match.group(3)) - deployment_start_time
                
                duration = end_time - start_time
                tasks.append({'label': label, 'start': start_time, 'duration': duration})
    return tasks


log_filename = str(os.path.dirname(os.path.abspath(__file__))) + '/outputs/log.txt'





tasks = parse_logs(log_filename)
title = "test"
if len(sys.argv) > 1:
    title = sys.argv[1]
plot_gantt(tasks, title)
