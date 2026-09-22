# Uno Q Port — Design Decisions

Reference doc for meetings on the Arduino Uno Q port (branch
`298-add-arduino-uno-q-port`). Every decision below, why it was made, what
else was on the table, and what could still change. Open items needing
team input are called out separately at the end.

## 1. Where the application runs

**Decision:** The entire firmware application (`src/`) runs as a single
process on the QRB2210 (Debian Linux). The STM32U585 is a peripheral
server only — it never links `src/` or `SF_HAL`, it just executes
GPIO/I2C operations and sensor sampling on request/schedule.

**Why:** Almost all of `src/` already talks only to `SF_HAL::*`/`IIMU`,
not to any vendor SDK directly, so the split was free — no `src/`
application code changes were required by this decision. The one known
exception (the IMU driver's direct Particle `TwoWire` dependency) was
already deferred work, so it didn't block this call either.

**Alternatives considered:** None seriously — this fell directly out of
confirming that `src/` is already HAL-clean.

**Directory consequence:**
- `src/platform/unoq/` — real `SF_HAL` implementation for Linux (this
  directory). Belongs under `src/platform/` because it implements
  `hal.hpp` and compiles into the same binary as `src/`.
- `unoq_mcu/` (repo root) — STM32U585 firmware. Not under `src/platform/`
  because it never implements `SF_HAL` — same category as `pc_hal/`
  already occupies at the root (own `main()`, own build), one step
  further removed since it doesn't even compile `src/`.
- `ipc/hal_rpc_protocol.h` (repo root) — the wire contract between the
  two binaries; not part of either one's HAL.

## 2. Transport: SPI + MessagePack-RPC

**Decision:** The QRB2210 and STM32U585 talk over **SPI**, using
**MessagePack-RPC** framing.

**Why:** The board wires two links between the processors, LPUART1 and
SPI3. Arduino's stock Bridge firmware uses the UART one, at 115200 baud
(roughly 11.5 kB/s). We take SPI3 instead, for bandwidth: batched sensor
samples plus BLE-bound ride data do not fit comfortably in 11.5 kB/s.

**Correction (2026-09-17):** an earlier version of this entry claimed the
SPI choice was "verified against Arduino's own Uno Q documentation" as
the wire scheme Bridge itself uses, and called it a confirmed fact rather
than a design choice. That was wrong. Bridge ships over UART on this
board; SPI3 is wired but unused by the stock firmware. Choosing SPI is a
deliberate divergence from what Arduino ships, not a reuse of it. The
hardware path is real (Arduino documents Bridge as transport-agnostic
across USB CDC, UART and SPI, and a community proof-of-concept drives
Bridge over SPI3), it is simply not the default.

**Alternatives considered:** LPUART1 at 115200, which is what Arduino
ships and is the lower-risk path, rejected on throughput. Earlier guesses
at UART or RPMsg predate any checking, and a bespoke binary opcode/frame
format was sketched before settling on msgpack-rpc (superseded).

**Still to verify:** that Bridge's framing is in fact MessagePack-RPC.
That claim predates this correction and has not been independently
checked. Nothing depends on it beyond the "reusing something documented"
justification, since `ipc/hal_rpc_protocol.h` is msgpack-rpc either way.

**Note:** We are *not* using Arduino's official Bridge library itself —
it's Python (Linux side) / Arduino-core C (MCU side), neither of which
fits our C++ app or bare-metal MCU firmware. We reuse the wire *format*
(msgpack-rpc) with our own encode/decode, not the library.

**Instance and pins:** SPI3. From Zephyr's Uno Q board devicetree the
pins are SCK PG9, MISO PG10, MOSI PB5, NSS PG12. Treat these as strong
but unconfirmed until checked against Arduino's official schematic;
nothing is wired to them in CubeMX yet.

**We replace Arduino's MCU firmware entirely.** The STM32 ships running
Arduino Core on Zephyr with Bridge on top. Our bare-metal peripheral
server takes the whole chip, so App Lab, the Arduino sketch flow and
Arduino's Linux-side Bridge tooling all stop applying to this board once
we flash it. That is implied by the port but was never written down.
It is reversible: `arduino-cli burn-bootloader` restores the stock image.

**Status:** `ipc/hal_rpc_protocol.h` defines the method names and message
shape. `src/platform/unoq/rpc_client.hpp/.cpp` implements real envelope
framing using `msgpack-c` (installed, linked). The one unfinished piece
is actual SPI byte I/O (`transport_send_raw`/`transport_recv_raw` in
`rpc_client.cpp`), still a stub. SPI mode (CPOL/CPHA, bit order) and the
Linux-side spidev device path remain unconfirmed.

## 3. Sensor sampling: MCU-timed, buffered/polled

**Decision:** The STM32U585 owns sample-rate timing itself via a hardware
timer, independent of RPC traffic. Linux drains a buffer on its own
schedule rather than triggering each sample.

**Why:** Sensor read scheduling is timing-critical. Having Linux trigger
each sample via an RPC round-trip would put capture timing at the mercy
of Linux's scheduler plus transport latency — unacceptable for the
precision needed.

**Alternatives considered:**
- *Linux-triggered, per-sample RPC* — rejected (timing risk above).
- *MCU streams/pushes samples as captured (async)* — not chosen; more
  complex on bare STM32Cube HAL with no RTOS (needs an async push path
  on the transport). Buffered/polled only needs synchronous request/response.

**Status:** Design only — the sampling task and its buffer are not yet
implemented in `unoq_mcu/`.

## 4. IMU goes on SPI; stub covers the gap until it does

**Decision:** The IMU is on SPI, not I2C. Temp sensor and water (wet/dry)
sensor remain I2C on the STM32U585.

**History:** this entry originally read "IMU stays on I2C/stub for now,
SPI move deferred", with the move scheduled only after the base GPIO/I2C
RPC port worked end-to-end. The reasoning was that nothing else on the
critical path needed SPI-for-sensors, and that the IMU driver
(`src/imu/newIMU.hpp`) already needed an unrelated invasive rework
(decoupling from Particle's `TwoWire`, see #5), so bundling the SPI move
into that same pass would avoid opening the file twice. The entry also
flagged the condition that would force it earlier: the board wiring the
IMU over SPI. That is now the case, so the deferral no longer holds and
the `newIMU.hpp` rework has to happen sooner than planned.

**Still unconfirmed:** which processor the IMU hangs off. That decides
how many SPI instances the STM32 needs (see open questions).

**Until it lands:** The IMU stays on `IMUStub` automatically.
`system.cpp`'s existing `#if SF_PLATFORM == SF_PLATFORM_PARTICLE ...
#else` branch already falls back to the stub for any non-Particle
platform, including `SF_PLATFORM_UNOQ`. Zero `src/` changes were needed
for this.

Note the protocol has no route for IMU data today. The method list in
`ipc/hal_rpc_protocol.h` covers GPIO, I2C, reset and version only.
Per section 3 the MCU samples on its own timer and Linux drains a buffer,
so what is missing is a sample-buffer read method, not generic SPI
passthrough.

## 5. Pre-existing HAL debt: `TwoWire` / `i2c_get_wire()`

**Decision:** Leave `hal.hpp`'s `class TwoWire;` forward declaration and
`i2c_get_wire()` alone. Every backend (this port included) carries a
placeholder `TwoWire` stub to satisfy it.

**Why it exists:** It's Particle/Arduino-specific and technically
violates `hal.hpp`'s own "no Particle SDK symbols" contract — but it only
exists to let the not-yet-decoupled IMU driver reach the raw `Wire`
object (tracked by the existing `TODO(290-hal-clean)` comment). Fixing
it means finishing the IMU rework (see #4), which is explicitly out of
scope for this port.

**Bug this caused:** The placeholder `TwoWire` in
`src/platform/unoq/i2c.cpp` was first written inside an anonymous
namespace, making it a distinct, incompatible type from the `::TwoWire`
forward-declared at global scope in `hal.hpp`. Failed to compile; fixed
by moving the definition to true global scope. Caught by actually
building, not by static analysis (no `compile_commands.json` existed
yet at that point).

**Resolves when:** The IMU rework in #4 lands — `i2c_get_wire()` and
`class TwoWire;` both get deleted from `hal.hpp` at that point, and every
backend's placeholder goes with them.

## 6. Phase-0 hardware/product decisions

Resolved directly by the team, with reasoning where given:

| Decision | Why / detail |
|---|---|
| No cellular modem on Uno Q or the peripheral board | `cellular_is_on()`/`cellular_is_ready()` are permanent `false` stubs — final behavior, not deferred. |
| BLE radio is the WCBN3536A, handled entirely by Linux (BlueZ) | No MCU involvement in `ble.cpp`. Implemented for real via BlueZ's D-Bus GATT API (`GattManager1`/`LEAdvertisingManager1`), not raw HCI — `bluetoothd` owns the HCI channel exclusively. |
| `system_reset()` resets both processors | Sent as an `SF_RPC_METHOD_RESET` **notification**, not a request — a request-and-await-response could race the MCU actually resetting mid-wait. |
| NVM must survive power loss | Implemented for real as an fsync'd file on Linux (`/data/.nvm`), not tmpfs/RAM-backed. The MCU has no NVM RPC verb — it's a dumb GPIO/I2C server only. |
| Reset diagnostics combine both processors' causes "where possible" | Added `SF_RPC_METHOD_GET_RESET_REASON` so Linux can query the MCU's last reset cause alongside its own boot reason. |
| `system_os_version()` returns Debian + kernel + MCU firmware version | The MCU firmware version needs a new `SF_RPC_METHOD_GET_FW_VERSION` query. |
| `mfgTest` needs an RPC-link self-test step | Not yet implemented. |
| Battery/fuel gauge bus placement | **Not resolved** — still open, see below. |

## 7. Build/tooling decisions

- **`unoq_hal/`** — a native (host-compiled, not cross-compiled) CMake
  target, reusing `pc_hal/`'s app source list but linking
  `src/platform/unoq/*.cpp`. Exists purely for IDE tooling and
  compile-sanity checking ahead of real cross-compilation — not the
  QRB2210 artifact itself. Reuses `pc_hal`'s `imu_stub.hpp` directly
  (via include path) rather than duplicating it.
- **`CMAKE_EXPORT_COMPILE_COMMANDS ON`** added at the root — this repo
  had no `compile_commands.json` anywhere before this port; this is what
  makes clangd/cpptools work for `pc_hal`, `googletests`, and `unoq_hal`
  alike.
- **No local ARM cross-toolchain requirement (for compiling).** Matches
  this repo's existing pattern: the real Particle firmware build uses
  Particle's cloud compile action, not a local toolchain, and CI installs
  build deps directly via `apt-get`. CI installs `gcc-arm-none-eabi`
  plus `ninja-build` and cross-compiles `unoq_mcu/` in the `test` job
  (`cmake --preset Debug`), so the MCU firmware is built on every push
  without anyone needing a local toolchain. A local install remains a
  separate, narrower need for physical flashing and debugging, and even
  that depends on the still-open deployment-mechanism question below.
- **STM32Cube HAL/CMSIS sourcing: STM32CubeMX**, generating a project for
  the exact part (**STM32U585AII6TR** — Cortex-M33 @ 160MHz, 2MB flash,
  786KB SRAM), over git-submoduling the full STM32CubeU5 repo or
  hand-vendoring minimal files. Chosen to avoid hand-rolling the
  clock-tree/pin config. Generation settings are pinned in section 8
  below. Until the generated files land, `unoq_mcu/` has no real
  `CMakeLists.txt`.
- **MessagePack library:** `msgpack-c` (already available, no new install
  needed) on the Linux side. MCU-side library still undecided (CMP was
  the earlier candidate, given no dynamic allocation on bare-metal).

## 8. STM32CubeMX generation settings

These are the settings the generated `unoq_mcu/` project is produced
with. The `.ioc` is committed alongside the generated sources so the
config is reproducible and CubeMX can be re-run when the schematic
answers the open pin questions.

**Decision:** Standalone STM32CubeMX (not STM32CubeIDE), project started
from the MCU selector on the exact orderable part `STM32U585AII6TR`,
**TrustZone disabled**, toolchain set to **CMake**, no RTOS middleware.

**Why, item by item:**

- Standalone CubeMX over STM32CubeIDE: we want generated sources that
  drop into our own CMake build, not an Eclipse project. Consistent with
  the no-local-toolchain-for-compiling decision above; CubeMX is a
  code generator here, not a development environment.
- MCU selector over board selector: the target is an Arduino Uno Q, so no
  ST board entry matches. The near-miss (B-U585I-IOT02A Discovery, same
  STM32U585AI die) would preload that board's pin assignments, clock
  source and peripherals, all of which would have to be unpicked.
- Exact orderable part rather than the generic `STM32U585AI`: pins the
  variant (2MB flash, UFBGA169) so the pinout matches the board.
- TrustZone disabled: the MCU firmware is a peripheral server. It holds
  no keys and attests nothing, so there is no secure/non-secure boundary
  worth drawing. Enabling it would split generation into paired secure
  and non-secure projects with a SAU partition to design, doubling the
  `CMakeLists.txt` problem, and pull TF-M into scope. It also requires
  setting the `TZEN` option byte, which needs an RDP regression and full
  chip erase to back out. Disabled is the reversible direction if the
  security requirements ever change.
- CMake toolchain output: this is what makes the generated tree
  consumable by `unoq_mcu/CMakeLists.txt`.
- "Copy only the necessary library files" plus per-peripheral `.c`/`.h`
  init file generation: keeps the vendored drop small and reviewable
  instead of pulling in all of STM32CubeU5.
- No RTOS middleware, matching the bare STM32Cube HAL decision.

**What could still change:** the SPI instance, pins and mode are left
unset until the schematic is confirmed (see open questions), so CubeMX
will need re-running.

**Peripheral set: one I2C bus, not four.** The four I2C devices share a
single bus at different addresses, so only one I2C peripheral is
enabled. This matches
what the HAL already assumes: `i2c_begin()` takes no bus argument and
`i2c_read()`/`i2c_write()` select a device by 7-bit address
(`src/platform/hal.hpp`), and the wire protocol agrees, with
`i2c_read` params `[address, length, repeated]` and `i2c_write` params
`[address, repeated, data]` (`ipc/hal_rpc_protocol.h`). Enabling I2C1
through I2C4 as independent buses was considered and rejected: it would
cost 8 pins instead of 2, and nothing could reach the extra buses without
adding a bus id to both the HAL signatures and the RPC params. Four
separate buses would only be justified by address collisions between the
devices, per-device speed isolation, or a need for electrical separation,
none of which apply. Consequence: `i2c_lock()`/`i2c_unlock()` matter more
than they would with separate buses, since all four devices contend for
the one peripheral.

**Which I2C instance: I2C4, not I2C1.** The generated project currently
has I2C1 on PB6/PB9, which are CubeMX's defaults and match nothing on
this board. The Uno Q routes I2C4 to the Qwiic connector (PD12 SCL,
PD13 SDA), I2C3 to the A4/A5 header pins (PC0/PC1), and I2C2 to
PB10/PB11. For four external sensors the Qwiic bus is the right one, so
I2C4 it is. PB9 is an active hazard besides: the board's pin map uses it
as SPI2 NSS. This needs fixing in CubeMX and regenerating.

Standalone GPIO is not configured in CubeMX yet. The pins that need it
are the seven `SF_HAL::PinId` members that `unoq_mcu/pins.hpp`
`resolve_pin()` maps, and which physical pins those are is schematic
dependent, the same blocker as SPI. Peripheral pins (I2C, SPI) are
assigned by CubeMX itself and are not configured as GPIO by hand.

**Deployment: SWD-flashed, so `STM32U585xx_FLASH.ld` is correct.**
Resolved against Arduino's and Zephyr's documentation: the STM32 is
programmed over SWD (STM32CubeProgrammer, OpenOCD or JLink runners, and
`arduino-cli burn-bootloader` to restore the stock image). The
RAM-loaded, remoteproc-style option we had been holding open is not how
this board works, so the `STM32U585xx_RAM.ld` CubeMX also generated goes
unused. Local flashing tools are therefore a real need for whoever does
hardware bring-up, though still not for compiling.

## 9. Open questions — needs team discussion

- **IMU SPI placement.** Wired to the STM32U585 or to the QRB2210
  directly? If the STM32, it needs two SPI instances with opposite roles
  (slave for the QRB2210 transport on SPI3, master for the IMU, most
  likely SPI2 which the board routes to the JSPI header as MISO PC2,
  MOSI PC3, SCK PD1). If the QRB2210, one instance is enough and the IMU
  is driven from Linux via spidev, outside the MCU-timed sampling model
  in section 3.
- **Confirm SPI3 against Arduino's official schematic.** Section 2 names
  SPI3 with pins from Zephyr's board devicetree, corroborated by a
  community SPI3 Bridge proof-of-concept, but we have not read the
  schematic ourselves. **SPI mode (CPOL/CPHA, bit order)** and the
  Linux-side **spidev device path** are genuinely unknown and block
  `rpc_client.cpp` and `transport.cpp` either way.
- **MCU-side MessagePack library.** Still undecided (CMP was the earlier
  candidate, given no dynamic allocation on bare metal). This is the only
  thing blocking `peripheral_server.cpp`'s dispatcher, and it needs no
  hardware information.
- **Sensor sample rate**, which sets the TIM2 configuration. No hardware
  dependency, just an unmade product decision.
- **Mapping the seven `SF_HAL::PinId` members to Uno Q header pins.** The
  datasheet publishes the full JDIGITAL/JANALOG/JMISC pin maps, so this
  is now a design choice rather than a blocker, and it is what fills in
  `unoq_mcu/pins.hpp` `resolve_pin()`.
- **Battery/fuel gauge bus placement** — unconfirmed against the schematic.
- **Sleep/power coordination** between Linux suspend and MCU sleep —
  `system_sleep()`/`system_sleep_gpio_wake()` have no design yet.
