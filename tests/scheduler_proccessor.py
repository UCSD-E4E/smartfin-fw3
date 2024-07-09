import re
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
import os.path
import cv2
import json
from pathlib import Path

def comparelogs():
    actual_json, expected_json = parse_logs()

    for k in expected_json.keys():
        expected = expected_json[k]
        actual = actual_json[k]
        same = False
        if actual == expected:
            same = True
        dir = "outputs/" + str(k) + "/"
        if len(expected) > 0:
            Path(dir).mkdir(parents=True, exist_ok=True)
            
            if same:
                if len(expected) > 0:
                    plot_gantt(expected, "out", dir)
            else:
            
                max_end = 0
                for v in expected:
                    if v.end_time > max_end:
                        max_end = v.end_time
                for v in actual:
                    if v.end_time > max_end:
                        max_end = v.end_time
                tasks_len = max(len(actual), len(expected))
                
                
                
                plot_gantt(expected, "expected", dir, max_end, tasks_len)
                
                plot_gantt(actual, "actual", dir, max_end, tasks_len)
                
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
    output_path = os.path.join(dir, title + ".png")
    

    plt.savefig(output_path)
    plt.close()
    
   


def parse_logs(return_max=False):
    tasks = []
    
    with open('expected.log') as f:
        expected_json = json.load(f)
    with open('actual.log') as f:
        actual_json = json.load(f)

    for k in expected_json.keys():
        expected_raw = expected_json[k]
        actual_raw = actual_json[k]
        expected_json[k] = []
        actual_json[k] = []
        for v in expected_raw:
            l = v.split('|')
            label = l[0]
            start_time = int(l[1])
            end_time = int(l[2])
            duration = end_time - start_time
            expected_json[k].append({'label': label, 'start': start_time, \
                              'duration': duration})
        for v in actual_raw:
            l = v.split('|')
            label = l[0]
            start_time = int(l[1])
            end_time = int(l[2])
            duration = end_time - start_time
            actual_json[k].append({'label': label, 'start': start_time, \
                              'duration': duration})
        
    return actual_json, expected_json


            
        
        
        
comparelogs()



#plot_gantt(tasks)
    
