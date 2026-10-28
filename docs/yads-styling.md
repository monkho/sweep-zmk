# YADS styling experiment

Branch: `codex/yads-nothing-style`, based on `origin/dongle` at
`4a2c69bf143d1e12e9eb532f49609b2eeca8e86d`.

This remains YADS/LVGL. The existing YADS dependency from
https://github.com/janpfischer/zmk-dongle-screen, branch `upgrade-4.1`,
commit `a8cec2626b8ea94e9a047eac1e53792980369c93`, is vendored into
`boards/shields/dongle_screen` and `drivers/display` with its MIT license.
The remote west project is removed to avoid duplicate shields, status screens,
and ST7789V driver replacements. Root CMake and module metadata register the
same driver and shield locally. Hardware overlays, rotation, brightness,
keyboard bindings, build targets, and Studio options are retained from dongle.

## Layout

The existing XIAO overlay has a 240 x 280 panel with y-offset 20. Existing
horizontal rotation yields a 280 x 240 logical canvas. This layout is for that
horizontal configuration. `src/theme.h` defines the palette and container styles.
Output is at (12,12); layer is at (20,86); WPM is at (154,58);
batteries are at (12,174); modifiers are at (12,214).
The embedded UNSCII 8 font supplies small labels; existing Montserrat 40
supplies large values. No web fonts or browser assets are used.

## Live values and limits

- Output: existing selected endpoint, USB HID readiness, BLE profile and
  connection state. Red means connected; gray means unavailable/disconnected.
- Layer: existing highest-active-layer API and layer name, falling back to its
  numeric index. Long names are clipped with dots to keep the WPM panel clear.
- WPM: existing ZMK WPM event, formatted with three digits.
- Batteries: existing peripheral battery events and canvas bars. P1/P2 are
  pairing slots, not guaranteed left/right. Unreported values show `--` and
  gray bars. Existing YADS treats a reported zero as unavailable; distinguishing
  a genuine empty battery from disconnect needs a separate state change.
- Modifiers: existing HID report mask; CTRL/ALT/GUI/SHFT turn red while active.
  Polling stays at 100 ms but now uses an LVGL timer instead of performing LVGL
  calls in a Zephyr timer interrupt. Unchanged masks skip label updates.

This pass changes existing widgets only. No WPM history graph, duplicate top
battery readout, separate layer-number widget, or scanline animation is added.

## Validation and build

Static checks cover local includes, unchanged driver/overlay files and build
matrix, and removal of the duplicate remote module. Firmware has not been
compiled: west, CMake, Ninja, the Zephyr SDK, and a ZMK dependency workspace are
not installed here. Hardware rendering and boot behavior remain unverified.

The existing GitHub Actions workflow builds all targets on push or manual run.
Alternatively, in an initialized ZMK workspace with Zephyr SDK installed:

```sh
west update
west zephyr-export
west build -s zmk/app -b xiao_ble//zmk -d build/dongle -- -DZMK_CONFIG=/absolute/path/sweep-zmk/config -DZMK_EXTRA_MODULES=/absolute/path/sweep-zmk -DSHIELD="cradio_dongle dongle_screen" -DSNIPPET="zmk-usb-logging studio-rpc-usb-uart" -DCONFIG_LOG_PROCESS_THREAD_STARTUP_DELAY_MS=8000 -DCONFIG_ZMK_STUDIO=y
```

Use the unchanged `build.yaml` matrix for half and reset targets.
ZMK itself still tracks `main` as in the existing configuration, so full version
compatibility must be established by the firmware build before flashing.
