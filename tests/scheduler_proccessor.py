from pathlib import Path
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import cv2
import json
from typing import Tuple, Dict, List, Union

import glob

mpl.use('Agg')

def compare_logs() -> None:
    actual_json, expected_json = parse_logs()

    for k in expected_json.keys():
        expected = expected_json[k]
        actual = actual_json[k]
        same = False
        if actual == expected:
            same = True
        dir_path = Path("tests/outputs") / str(k)
        if expected:
            dir_path.mkdir(parents=True, exist_ok=True)
            
            if same:
                plot_gantt(expected, "out", dir_path)
            else:
                max_end = max(v['start'] + v['duration'] for v in expected + actual)
                tasks_len = max(len(actual), len(expected))

                plot_gantt(expected, "expected", dir_path, max_end, tasks_len)
                plot_gantt(actual, "actual", dir_path, max_end, tasks_len)

                img1 = cv2.imread(str(dir_path / "expected.jpg"))
                img2 = cv2.imread(str(dir_path / "actual.jpg"))
                im_v = cv2.vconcat([img1, img2])
                cv2.imwrite(str(dir_path / 'out.jpg'), im_v)

def plot_gantt(tasks: List[Dict[str, Union[str, int]]], title: str, dir_path: Path, max_duration: int = 0, tasks_len: int = 0) -> None:
    if tasks_len == 0:
        tasks_len = len(tasks)
    fig, ax = plt.subplots(figsize=(tasks_len, 3))
    task_dict = {}
    for task in tasks:
        task_dict.setdefault(task['label'], []).append((task['start'], task['duration']))

    y_pos = range(len(task_dict))
    colors = ['skyblue', 'lightgreen', 'salmon', 'grey']
    color_idx = 0

    for i, (label, segments) in enumerate(task_dict.items()):
        last_end = 0
        for start, duration in segments:
            ax.barh(i, duration, left=start, color=colors[color_idx % len(colors)])
            ha = 'left' if start - last_end < 300 else 'center'
            va = 'top' if start - last_end < 300 else 'bottom'

            if len(task_dict.keys()) > 1:
                ax.text(start + duration / 2, i, f"{label} ({start} - {start + duration})",
                        verticalalignment=va, horizontalalignment=ha, color='black')
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
    output_path = dir_path / (title + ".jpg")

    plt.savefig(output_path)
    plt.close()

def parse_logs() -> Tuple[Dict[str, List[Dict[str, Union[str, int]]]], Dict[str, List[Dict[str, Union[str, int]]]]]:
    expected_json = {}
    actual_json = {}
    for filepath in glob.iglob('tests/outputs/actual*.log'):
        expected_file_path = Path(str(filepath).replace("actual", "expected"))
        actual_file_path = Path(filepath)

        with open(expected_file_path) as f:
            expected_json.update(json.loads(f.read()))

        with open(actual_file_path) as f:
            actual_json.update(json.loads(f.read()))

    for k in expected_json.keys():
        expected_raw = expected_json[k]
        actual_raw = actual_json[k]
        expected_json[k] = [{'label': l[0], 'start': int(l[1]), 'duration': int(l[2]) - int(l[1])} for v in expected_raw for l in [v.split('|')]]
        actual_json[k] = [{'label': l[0], 'start': int(l[1]), 'duration': int(l[2]) - int(l[1])} for v in actual_raw for l in [v.split('|')]]
    
    return actual_json, expected_json

def parse_examine_logs() -> Dict[str, List[Dict[str, Union[str, int]]]]:
    actual_json = {}
    for filepath in glob.iglob('no_check_outputs/actual*.log'):
        actual_file_path = Path(filepath)

        with open(actual_file_path) as f:
            actual_json.update(json.loads(f.read()))

    for k in actual_json:
        actual_raw = actual_json[k]
        actual_json[k] = [{'label': l[0], 'start': int(l[1]), 'duration': int(l[2]) - int(l[1])} for v in actual_raw for l in [v.split('|')]]

    return actual_json

def examine_logs() -> None:
    actual_json = parse_examine_logs()

    for k in actual_json:
        actual = actual_json[k]
        dir_path = Path("no_check_outputs")
        if actual:
            plot_examine_gantt(actual, str(k), dir_path, len(actual))

def plot_examine_gantt(tasks: List[Dict[str, Union[str, int]]], title: str, dir_path: Path, tasks_len: int = 0) -> None:
    if tasks_len == 0:
        tasks_len = len(tasks)
    fig, ax = plt.subplots(figsize=(tasks_len, 3))
    task_dict = {}
    
    max_duration = 0
    for task in tasks:
        task_dict.setdefault(task['label'], []).append((task['start'], task['duration']))
        max_duration = max(max_duration, task['start'] + task['duration'])
    
    y_pos = range(len(task_dict))
    colors = ['skyblue', 'lightgreen', 'salmon', 'grey']
    color_idx = 0

    for i, (label, segments) in enumerate(task_dict.items()):
        for start, duration in segments:
            ax.barh(i, duration, left=start, color=colors[color_idx % len(colors)])
            ax.text(start + duration / 2, i, f"{label} ({start} - {start + duration})",
                    verticalalignment='center', horizontalalignment='center', color='black')
        color_idx += 1
    
    if max_duration != 0:
        ax.set_xlim(0, max_duration)
        
    ax.set_yticks(list(y_pos))
    ax.set_yticklabels(task_dict.keys())
    ax.set_xlabel('Time (ms)')
    ax.set_title(title)
    ax.grid(True)

    plt.subplots_adjust(bottom=0.15)
    output_path = dir_path / (title + ".jpg")
    plt.savefig(output_path)
    plt.close()

if __name__ == "__main__":
    compare_logs()
    examine_logs()
    root = Path('tests/outputs/')
    folders = [f for f in root.glob('*/') if f.is_dir()]

    for folder in folders:
        if not any(folder.iterdir()):
            folder.rmdir()
