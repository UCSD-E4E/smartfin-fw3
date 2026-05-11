"""Entry point: python tests/ble/run_ble.py [-v]"""
import matplotlib
matplotlib.use("Qt5Agg")

import argparse
import asyncio
import csv
from datetime import datetime
import logging
import sys
import threading
import time

if sys.platform == "win32":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

import numpy as np

from ble_receiver.config import LOG_DIR, SensorType
from ble_receiver.state import BLEState
from ble_receiver.processor import packet_processor
from ble_receiver.receiver import run_ble
from ble_receiver.plotter import setup_realtime_figures

try:
    import matplotlib.pyplot as plt
    _MPL_AVAILABLE = True
except Exception:
    _MPL_AVAILABLE = False


def _prune_old_devtty_logs() -> None:
    matches = [p for p in LOG_DIR.glob("devtty*.txt") if p.is_file()]
    if len(matches) <= 1:
        return
    newest = max(matches, key=lambda p: (p.stat().st_mtime, p.name))
    for p in matches:
        if p != newest:
            p.unlink()


class _FlushFileHandler(logging.FileHandler):
    def emit(self, record: logging.LogRecord) -> None:
        super().emit(record)
        self.flush()


class _FlushStreamHandler(logging.StreamHandler):
    def emit(self, record: logging.LogRecord) -> None:
        super().emit(record)
        self.flush()


def setup_logging(verbose: bool) -> logging.Logger:
    fmt = logging.Formatter(
        fmt="%(asctime)s:%(msecs)03d [%(levelname)s] %(message)s",
        datefmt="%H:%M:%S",
    )

    log = logging.getLogger("ble_receiver")
    log.setLevel(logging.DEBUG)
    fh = _FlushFileHandler(LOG_DIR / "ble_log.txt", mode="w")
    fh.setFormatter(fmt)
    log.addHandler(fh)
    if verbose:
        sh = _FlushStreamHandler()
        sh.setFormatter(fmt)
        log.addHandler(sh)

    bleak_log = logging.getLogger("bleak")
    bleak_log.setLevel(logging.DEBUG)
    bleak_fh = _FlushFileHandler(LOG_DIR / "bleak_debug.txt", mode="w")
    bleak_fh.setFormatter(fmt)
    bleak_log.addHandler(bleak_fh)
    if verbose:
        bleak_sh = _FlushStreamHandler()
        bleak_sh.setFormatter(fmt)
        bleak_log.addHandler(bleak_sh)

    return log


def _save_records(state: BLEState, log: logging.Logger) -> None:
    if not state.records:
        log.info("no records collected - nothing saved")
        return

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    npy_path = LOG_DIR / f"records_{ts}.npy"
    csv_path = LOG_DIR / f"records_{ts}.csv"

    arr = state.as_numpy()
    np.save(npy_path, arr)

    with csv_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time_ms", "sensor_type", "ensemble_type", "value"])
        for row in state.records:
            time_ms, sensor_int, ensemble_type, value = row
            try:
                sensor_name = SensorType(int(sensor_int)).name
            except ValueError:
                sensor_name = str(int(sensor_int))
            writer.writerow([f"{time_ms:.6f}", sensor_name, int(ensemble_type), f"{value:.8f}"])

    log.info("saved %d records -> %s  %s", len(state.records), npy_path, csv_path)


async def _ble_main(log: logging.Logger, state: BLEState) -> None:
    """BLE + packet processing only — no matplotlib calls."""
    queue: asyncio.Queue = asyncio.Queue()
    processor_task = asyncio.create_task(packet_processor(queue, state, log))
    try:
        await run_ble(state, queue, log)
    except asyncio.CancelledError:
        log.info("BLE task cancelled")
    finally:
        await queue.join()
        await queue.put(None)
        await processor_task


def _run_ble_thread(log: logging.Logger, state: BLEState, done: threading.Event) -> None:
    """Run the asyncio BLE loop in a background thread."""
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(_ble_main(log, state))
    except KeyboardInterrupt:
        pass
    finally:
        loop.close()
        done.set()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="also print logs to terminal")
    args = parser.parse_args()

    LOG_DIR.mkdir(parents=True, exist_ok=True)
    _prune_old_devtty_logs()

    log = setup_logging(args.verbose)
    state = BLEState()

    # Figures must be created on the main thread before asyncio takes over.
    if _MPL_AVAILABLE:
        plt.ion()
        figs, update_fn = setup_realtime_figures(state)
    else:
        figs, update_fn = [], lambda: None

    done = threading.Event()
    ble_thread = threading.Thread(target=_run_ble_thread, args=(log, state, done), daemon=True)
    ble_thread.start()

    try:
        # Main thread drives the GUI event loop via plt.pause().
        while not done.is_set():
            update_fn()
            for fig in figs:
                fig.canvas.draw_idle()
            if _MPL_AVAILABLE:
                for fig in figs:
                    fig.canvas.flush_events()
                time.sleep(0.5)
            else:
                done.wait(0.5)
    except KeyboardInterrupt:
        log.info("interrupted by user")
    finally:
        ble_thread.join(timeout=5)
        _save_records(state, log)

    if _MPL_AVAILABLE and plt.get_fignums():
        update_fn()
        plt.ioff()
        plt.show(block=True)
