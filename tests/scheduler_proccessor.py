import re
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
import os.path
import cv2


def comparelogs(dir):
    expected_file = open(str(dir) + "/expected.txt", "r")
    actual_file = open(str(dir) + "/actual.txt", "r")
    expected = expected_file.readlines()
    actual = actual_file.readlines()
    same = False
    if len(expected) == len(actual):
        same = True
        for i in range(len(expected)):
            if expected[i] != actual[i]:
                same = False
                break
    if same:
        tasks = parse_logs(str(dir) + "/expected.txt")
        plot_gantt(tasks, "out", dir)
    else:
        
        expected_tasks, max1 = parse_logs(str(dir) + "/expected.txt",True)
        
        actual_tasks, max2 = parse_logs(str(dir) + "/actual.txt",True)
        max_duration = max(max1, max2)
        tasks_len = max(len(actual_tasks),len(expected_tasks))
        print(max1,max2,max_duration)
        
        plot_gantt(expected_tasks, "expected", dir, max_duration,tasks_len)
        plot_gantt(actual_tasks, "actual", dir, max_duration,tasks_len)
        img1 = cv2.imread(str(dir) + "/expected.png")
        img2 = cv2.imread(str(dir) + "/actual.png")
        print(img1.shape[:2])
        print(img2.shape[:2])
        im_v = cv2.vconcat([img1, img2]) 
        cv2.imwrite(str(dir) + '/out.jpg', im_v)



def plot_gantt(tasks, title, dir='/outputs/', max_duration=0,tasks_len=0):
    if tasks_len == 0:
        tasks_len = len(tasks)
    fig, ax = plt.subplots(figsize=(tasks_len, 3))
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
            ax.barh(i, duration, left=start, \
                    color=colors[color_idx % len(colors)])
            ha = 'left' if start - last_end < 300 else 'center'
            va = 'top' if start - last_end < 300 else 'bottom'

            # Debugging print statements
            if len(task_dict.keys()) > 1:
                ax.text(start + duration / 2, i, \
                        f"{label} ({start} - {start + duration})", \
                            verticalalignment=va, horizontalalignment=ha, \
                                color='black')
            last_end = start + duration
        color_idx += 1
    if max_duration != 0:
        ax.set_xlim(0, max_duration)
    ax.set_yticks(y_pos)
    ax.set_yticklabels(task_dict.keys())
    ax.set_xlabel('Time (ms)')
    ax.set_title(title)
    ax.grid(True)

    plt.subplots_adjust(bottom=0.15)
    output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), \
                               dir, title + ".png")
    plt.savefig(output_path)
    print(f"Plot saved to {output_path}")


def parse_logs(filename,return_max=False):
    tasks = []
    deployment_start_time = 0
    max_end_time = 0
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
                tasks.append({'label': label, 'start': start_time, \
                              'duration': duration})
                if end_time > max_end_time:
                    max_end_time = end_time
    if return_max:
        return tasks, max_end_time
    return tasks


title = "test"
log_file = str(os.path.dirname(os.path.abspath(__file__))) + '/outputs/log.txt'
dir_arg = False
if len(sys.argv) > 1:
    if os.path.isdir(sys.argv[1]):
        dir_arg = True
        comparelogs(sys.argv[1])
    else:
        log_file = str(os.path.dirname(os.path.abspath(__file__))) + \
            '/' + sys.argv[1]
        title = os.path.splitext(os.path.basename(log_file))[0]
if not dir_arg:
    print(log_file)
    print(title)
    tasks = parse_logs(log_file)
    plot_gantt(tasks, title)
