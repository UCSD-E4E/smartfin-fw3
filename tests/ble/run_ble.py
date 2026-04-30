"""Entry point: python tests/ble/run_ble.py [-v]"""
import argparse
import asyncio
from datetime import datetime
import logging
import threading

import numpy as np

from ble_receiver.config import LOG_DIR
from ble_receiver.state import BLEState
from ble_receiver.processor import packet_processor
from ble_receiver.receiver import run_ble
from ble_receiver.plotter import show_realtime_plots


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


def _run_ble_thread(state: BLEState, log: logging.Logger) -> None:
    """Run the full BLE session (scan -> connect -> receive -> decode) in a thread."""
    async def _async() -> None:
        queue: asyncio.Queue = asyncio.Queue()
        processor_task = asyncio.create_task(packet_processor(queue, state, log))
        try:
            await run_ble(state, queue, log)
        finally:
            await queue.join()
            await queue.put(None)
            await processor_task

    try:
        asyncio.run(_async())
    except Exception:
        log.exception("BLE thread exception")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="also print logs to terminal")
    args = parser.parse_args()

    _prune_old_devtty_logs()

    log = setup_logging(args.verbose)
    state = BLEState()

    ble_thread = threading.Thread(
        target=_run_ble_thread, args=(state, log), daemon=True
    )
    ble_thread.start()

    # matplotlib must run on the main thread; show_realtime_plots blocks here
    # until both plot windows are closed
    try:
        show_realtime_plots(state)
    except KeyboardInterrupt:
        log.info("interrupted by user")
    finally:
        arr = state.as_numpy()
        if arr.shape[0] > 0:
            ts = datetime.now().strftime("%Y%m%d_%H%M%S")
            npy_path = LOG_DIR / f"records_{ts}.npy"
            csv_path = LOG_DIR / f"records_{ts}.csv"
            np.save(npy_path, arr)
            np.savetxt(
                csv_path, arr,
                delimiter=",",
                header="time_ds,sensor_type,ensemble_type,value",
                comments="",
                fmt=["%.6f", "%d", "%d", "%.8f"],
            )
            log.info("saved %d records -> %s  %s", arr.shape[0], npy_path, csv_path)
        else:
            log.info("no records collected - nothing saved")

    ble_thread.join(timeout=5)
