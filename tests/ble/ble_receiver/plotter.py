import numpy as np
from .config import SensorType
from .state import BLEState

try:
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation
    from matplotlib.artist import Artist
    _MPL_AVAILABLE = True
except Exception:
    _MPL_AVAILABLE = False


def _sensor(records: np.ndarray, sensor_type: SensorType) -> tuple[np.ndarray, np.ndarray]:
    """Return (time_ds, values) arrays for a given SensorType from the records array."""
    mask = records[:, 1] == sensor_type
    rows = records[mask]
    return rows[:, 0], rows[:, 3]


def show_realtime_plots(state: BLEState) -> None:
    """Open two live-updating figure windows and block until both are closed.

    Must be called from the main thread. BLE reception runs concurrently in a
    background thread and writes to state; FuncAnimation reads state every 500 ms.
    """
    if not _MPL_AVAILABLE:
        print("matplotlib not available - skipping plots")
        return

    # --- Sensor figure (temperature + IMU) ---
    fig_s, axes_s = plt.subplots(2, 2, figsize=(12, 7))
    if fig_s.canvas.manager is not None:
        fig_s.canvas.manager.set_window_title("Smartfin BLE - Live Sensor Data")
    ax_temp, ax_acc, ax_gyro, ax_mag = (
        axes_s[0][0], axes_s[0][1], axes_s[1][0], axes_s[1][1]
    )

    def _update_sensor(_: int) -> list[Artist]:
        ax_temp.clear(); ax_acc.clear(); ax_gyro.clear(); ax_mag.clear()

        if state.records:
            arr = np.array(state.records, dtype=np.float64)

            t_temp, v_temp = _sensor(arr, SensorType.TEMPERATURE)
            if len(t_temp):
                ax_temp.plot(t_temp, v_temp, lw=1.8, color="#CA472F")

            t_imu, _ = _sensor(arr, SensorType.ACCEL_X)
            if len(t_imu):
                ax_acc.plot(t_imu, _sensor(arr, SensorType.ACCEL_X)[1], lw=1.2, color="#0B84A5", label="ax")
                ax_acc.plot(t_imu, _sensor(arr, SensorType.ACCEL_Y)[1], lw=1.2, color="#F6C85F", label="ay")
                ax_acc.plot(t_imu, _sensor(arr, SensorType.ACCEL_Z)[1], lw=1.2, color="#6F4E7C", label="az")
                ax_gyro.plot(t_imu, _sensor(arr, SensorType.GYRO_X)[1],  lw=1.2, color="#9DD866", label="gx")
                ax_gyro.plot(t_imu, _sensor(arr, SensorType.GYRO_Y)[1],  lw=1.2, color="#FFA600", label="gy")
                ax_gyro.plot(t_imu, _sensor(arr, SensorType.GYRO_Z)[1],  lw=1.2, color="#BC5090", label="gz")
                ax_mag.plot(t_imu,  _sensor(arr, SensorType.MAG_X)[1],   lw=1.2, color="#003F5C", label="mx")
                ax_mag.plot(t_imu,  _sensor(arr, SensorType.MAG_Y)[1],   lw=1.2, color="#58508D", label="my")
                ax_mag.plot(t_imu,  _sensor(arr, SensorType.MAG_Z)[1],   lw=1.2, color="#FF6361", label="mz")
                ax_acc.legend(loc="upper right", fontsize=7)
                ax_gyro.legend(loc="upper right", fontsize=7)
                ax_mag.legend(loc="upper right", fontsize=7)

        ax_temp.set_title("Temperature"); ax_temp.set_xlabel("elapsed time (ds)"); ax_temp.set_ylabel("deg C")
        ax_acc.set_title("Acceleration"); ax_acc.set_xlabel("elapsed time (ds)"); ax_acc.set_ylabel("m/s^2")
        ax_gyro.set_title("Gyroscope"); ax_gyro.set_xlabel("elapsed time (ds)"); ax_gyro.set_ylabel("deg/s")
        ax_mag.set_title("Magnetometer"); ax_mag.set_xlabel("elapsed time (ds)"); ax_mag.set_ylabel("uT")
        fig_s.suptitle(
            f"Smartfin BLE sensor  ensembles={state.ensemble_count}  notifies={state.notify_count}"
        )
        fig_s.tight_layout()
        return []

    # --- BLE transport figure ---
    fig_b, axes_b = plt.subplots(2, 2, figsize=(12, 7))
    if fig_b.canvas.manager is not None:
        fig_b.canvas.manager.set_window_title("Smartfin BLE - Live Transport")
    ax_dt, ax_batch, ax_throughput, ax_payload = (
        axes_b[0][0], axes_b[0][1], axes_b[1][0], axes_b[1][1]
    )

    def _update_transport(_: int) -> list[Artist]:
        ax_dt.clear(); ax_batch.clear(); ax_throughput.clear(); ax_payload.clear()

        if not state.notify_index:
            return []

        n          = list(state.notify_index)
        elapsed_s  = list(state.notify_elapsed_s)
        dt_ms      = list(state.notify_dt_ms)
        batch      = list(state.notify_ensemble_batch)
        payload    = list(state.notify_payload_bytes)
        cumulative = list(state.cumulative_ensembles)

        ax_dt.plot(n, dt_ms, lw=1.2, color="#2F4B7C")
        if dt_ms:
            mean_dt = sum(dt_ms) / len(dt_ms)
            ax_dt.axhline(mean_dt, color="#D45087", lw=1.0,
                          linestyle="--", label=f"mean {mean_dt:.2f} ms")
            ax_dt.legend(loc="upper right", fontsize=7)
        ax_dt.set_title("Notify Interval Jitter")
        ax_dt.set_xlabel("notification index"); ax_dt.set_ylabel("ms")

        ax_batch.plot(n, batch, lw=1.2, color="#FFA600")
        ax_batch.set_title("Ensembles Per Notify")
        ax_batch.set_xlabel("notification index"); ax_batch.set_ylabel("ensembles")

        throughput = [c / t if t > 0 else 0.0 for t, c in zip(elapsed_s, cumulative)]
        ax_throughput.plot(elapsed_s, throughput, lw=1.4, color="#00A6ED", label="avg ensembles/s")
        ax_throughput.plot(elapsed_s, cumulative, lw=1.0, color="#7A5195", label="cumulative ensembles")
        ax_throughput.set_title("Throughput / Progress")
        ax_throughput.set_xlabel("elapsed time (s)"); ax_throughput.set_ylabel("ensembles")
        ax_throughput.legend(loc="upper left", fontsize=7)

        ax_payload.plot(n, payload, lw=1.2, color="#3CAEA3", label="payload bytes")
        if batch:
            efficiency = [p / b if b > 0 else 0 for p, b in zip(payload, batch)]
            ax_payload.plot(n, efficiency, lw=1.0, color="#ED553B", label="bytes/ensemble")
        ax_payload.set_title("Batching Efficiency")
        ax_payload.set_xlabel("notification index"); ax_payload.set_ylabel("bytes")
        ax_payload.legend(loc="upper right", fontsize=7)

        fig_b.suptitle(
            f"Smartfin BLE transport  ensembles={state.ensemble_count}  notifies={state.notify_count}"
        )
        fig_b.tight_layout()
        return []

    # Keep references alive - FuncAnimation will be GC'd if not stored
    animations = [
        FuncAnimation(fig_s, _update_sensor,    interval=500, cache_frame_data=False),
        FuncAnimation(fig_b, _update_transport, interval=500, cache_frame_data=False),
    ]
    _ = animations  # suppress unused-variable lint

    plt.show(block=True)
