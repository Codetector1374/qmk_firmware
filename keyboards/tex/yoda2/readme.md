# Tex Yoda 2

HT32F1654, 8 MHz HSE crystal, 72 MHz CPU/AHB and 48 MHz USB.
The default keymap assigns the 61 ANSI keys and three confirmed mouse buttons.
The electrical matrix is 8×9; eight unused intersections have no key assignment.
PB1 is the active-low host Caps Lock indicator.

## Default keymap

The former Esc key sends grave/backtick (tilde with Shift). Caps Lock is
`LT(_NAV, KC_ESC)`: tap Escape, hold the momentary second layer. Fn is
`MO(_FN)`: hold the momentary third layer, with no Menu action.

Layer 2 is transparent except H/J/K/L = Left/Down/Up/Right.
Layer 3 is transparent except:

| Physical key | Fn action |
| --- | --- |
| 1 through = | F1 through F12 |
| Caps Lock | Caps Lock |
| Z / X / C | Previous track / play-pause / next track |
| V / B / N | Volume down / up / mute |
| I / O / L | Insert / Home / End |
| P / ; | Page Up / Page Down |
| [ / ] | Print Screen / Scroll Lock |

All other keys, including Backspace, are transparent on the two extra layers.
The third layer has priority when both are active. Left/right clicks remain
unchanged on every layer; the middle button is a hold-only scroll control.
Consumer/system HID reports are enabled.
Caps tap/hold uses QMK's standard tapping behavior and timing.

### Hold-to-scroll

The middle switch (`r0 c8`) is `DRAG_SCROLL`, not a mouse-button keycode.
A tap sends nothing. While held, movement accumulates until either axis reaches
8 counts, then locks to the larger axis (vertical wins exact ties). The other
axis is ignored until release; cursor movement is suppressed. In Windows mode,
upward stick movement sends wheel-up. Mac mode (DIP 1 ON) reverses both wheel
axes; macOS's Natural scrolling setting also affects the resulting direction.
Thirty-two counts make one wheel step (one quarter the previous speed), with signed
remainders retained on the selected axis. Release resets the lock/remainders
and restores cursor movement. Both transitions discard queued raw motion
to avoid transferring movement into the wrong mode. Left/right clicks are
preserved, and the scroll key remains accessible through both extra layers.

The 26,496-byte build passes a sanitizer host smoke check covering the 32-count
wheel threshold, unchanged 8-count axis lock, signed remainders, both axes in
Windows/Mac mode, release reset and unchanged cursor/buttons. It was downloaded
through DFU, completed manifestation with status OK and reset into enumerated
keyboard/mouse HID collections. Physical scroll speed/direction remain to be confirmed.


## DIP switches

All six are active-low inputs with pull-ups and explicit HT32 GPIO AF1:

| Physical switch | Pin | Function |
| --- | --- | --- |
| 1 | PB2 | OFF: Windows; ON: Mac (swap left/right Alt/GUI and reverse both scroll axes) |
| 2 | PB3 | Unassigned |
| 3 | PB4 | Unassigned |
| 4 | PB5 | Unassigned |
| 5 | PC7 | Unassigned |
| 6 | PC8 | Unassigned |

States are applied at startup and monitored while running. DIP 1 is authoritative;
there is no EEPROM write or OS detection. Mode changes release current keyboard
keys/modifiers before remapping to prevent stuck modifiers, so release held keys
when switching. DIPs 2–6 are read but have no assigned action.

The 26,164-byte DIP-enabled image was downloaded through DFU, completed
manifestation with status OK and reset into enumerated keyboard/mouse HID
collections. dfu-util returned 74 after printing Done; runtime enumeration
independently confirms handoff. Physical DIP toggling still needs confirmation.


## TrackPoint receiver

`trackpoint.c` implements a keyboard-local QMK custom pointing-device driver,
based on the tested Rust receiver and the [hardware/protocol notes](https://codetector.org/post/tex_yoda/tex_yoda_part2/).

- PA10 low powers Q10 and the TrackPoint/SN8 bridge. GPIO AF1 is selected explicitly.
- PC9/PC10 use I2C1 AF7, open-drain with pull-ups, as a 7-bit slave at **0x30**.
- The I2C1 interrupt receives `flags, signed Y, signed X`. Address and STOP
  flags are read-to-clear; the ISR preserves one snapshot and drains pending
  data before discarding partial frames at STOP. Bus errors discard partial data.
- Each complete three-byte group adds motion to bounded signed accumulators;
  the report callback atomically drains them within HID bounds, retaining excess
  motion for subsequent reports. Accumulators saturate at ±32767.
- Flags do not alter buttons. Matrix left/right clicks are preserved by QMK's
  pointing-device/mousekey integration; the middle switch controls scrolling.
- Hardware movement was confirmed: physical up originally moved right and right
  moved down. `POINTING_DEVICE_ROTATION_90` now applies `X = raw Y, Y = -raw X`.
  Normal pointer motion is unscaled. Hold-to-scroll is implemented in the
  default keymap; no CPI-setting protocol is added.
- I2C is configured before enabling module power. UART output and the sample's
  8 MHz system-clock setup are not copied; QMK stays at 72 MHz.

The preceding three-layer image built at 25,796 bytes. A temporary host C harness with address/undefined
behavior sanitizers passed signed-axis, button-preservation, partial/error-frame,
motion cancellation, residual-motion and accumulator-saturation checks.
The application was programmed/readback-verified using probe-rs, preserving the
bootloader. Physical reception/motion is confirmed by the direction experiment;
the corrected orientation is user-confirmed; dragging and cold power-on remain unverified.

After programming, test left/right/up/down independently, then typing and
holding/releasing each mouse button while moving. Check cold power-on as well.


## Wiring and coordinates

`high-side pin -> diode -> switch -> low-side pin`.
High-side pins are logical rows, sensed with pull-ups. Low-side pins are logical
columns, driven low one at a time (`ROW2COL`); unselected lines are pulled-up
inputs, not driven high. HT32 GPIO alternate function 1 is selected explicitly.
QMK's standard matrix scanner and 5 ms debounce are used.
Microsecond settling delays use QMK's `WAIT_US_TIMER` with HT32 BFTM0, not DWT.
The DWT-based default stalled scanning in `chSysPolledDelayX` on this board.

| Index | Row / high side | Column / low side |
| --- | --- | --- |
| 0 | PC14 | PA1 |
| 1 | PC15 | PA11 |
| 2 | PA4 | PB11 |
| 3 | PC13 | PD0 |
| 4 | PA7 | PB6 |
| 5 | PC12 | PC4 |
| 6 | PA5 | PA2 |
| 7 | PA6 | PA0 |
| 8 | — | PA3 |

PA3 was added as column 8 after the initial experiments. Existing columns 0–7
and all recorded coordinates are unchanged. The three mouse buttons were then
confirmed on column 8 by individual press/release logs.

PB1 is the **active-low Caps Lock indicator**, following host LED reports.
Toggle Caps Lock from another keyboard to test it without assigning diagnostic
switches. The bootloader recovery key PC15/PC4 is matrix position **r1 c5**.

## Build and capture

From `qmk_firmware` with direnv loaded:

```sh
qmk compile -kb tex/yoda2 -km default
```

The default keymap emits keyboard/mouse HID reports; diagnostic console logging
is no longer enabled. Development USB identity: **FEED:5942**.

## Matrix discovery record

First physical row, pressed left-to-right: 14 distinct intersections observed,
in this order: `(1,5), (1,4), (1,3), (1,2), (1,1), (1,7), (1,0), (1,6),
(0,5), (0,4), (0,3), (0,2), (0,1), (0,0)`.
There was an additional press/release of `(1,0)` before `(1,6)`; treating it
as a repeated key. These are physical positions; the first now sends grave/backtick.

Tab row, pressed left-to-right: 14 press/release pairs, no repeated positions:
`(0,6), (3,5), (3,4), (3,3), (3,2), (3,1), (3,7), (3,0), (3,6),
(2,5), (2,4), (2,3), (2,2), (2,1)`.
Interpreted legends: `Tab Q W E R T Y U I O P [ ] Backslash`.

Home row, pressed left-to-right: 13 clean press/release pairs:
`(2,7), (2,0), (2,6), (4,5), (4,4), (4,3), (4,2), (4,1), (4,7),
(4,0), (4,6), (7,5), (7,4)`.
Interpreted legends: `CapsLock A S D F G H J K L Semicolon Quote Enter`.

Shift row, pressed left-to-right: 12 clean press/release pairs:
`(7,3), (7,1), (7,7), (7,0), (7,6), (6,5), (6,4), (6,3), (6,2),
(6,1), (6,7), (6,0)`.
Interpreted legends: `LeftShift Z X C V B N M Comma Period Slash RightShift`.

Bottom keyboard row, pressed left-to-right: 8 clean press/release pairs:
`(5,5), (5,4), (5,3), (5,2), (5,1), (5,7), (5,0), (5,6)`.
Confirmed physical legends, left-to-right:
`LeftCtrl LeftGUI LeftAlt Space RightAlt RightGUI App/Fn? RightCtrl`.
The seventh key is now momentary Fn only; Caps supplies tap Escape / hold navigation.

Confirmed mouse buttons (user-provided console capture, left/middle/right order):

| Button | Matrix | High side | Low side |
| --- | --- | --- | --- |
| Left | r3 c8 | PC13 | PA3 |
| Middle | r0 c8 | PC14 | PA3 |
| Right | r1 c8 | PC15 | PA3 |

Earlier ambiguous bottom-row events are not mouse assignments. Experiments
identified 61 keyboard positions plus 3 mouse-button positions (64 switches).
The physical `LAYOUT` uses those positions; unused electrical intersections
remain unassigned.

## Application and recovery

Application vectors start at **0x2000**, matching the current Rust bootloader in
`ht32-rs/usb_dfu_bootloader`. Flash is restricted to **[0x2000, 0xFC00)** (55 KiB);
the bootloader's first 8 KiB and final 1 KiB are untouched. SRAM is 16 KiB.
Settings use transient EEPROM and are not persistent.

Build output is `tex_yoda2_default.bin`; the ELF is `.build/tex_yoda2_default.elf`.
Program only the application, preserving the bootloader:

```sh
probe-rs download --chip HT32F1654 --protocol swd --speed 100 --verify --reset .build/tex_yoda2_default.elf
```

Alternatively, hold the recovery key while resetting to enter the Rust DFU
bootloader (FEED:6969), then download the raw application:

```sh
dfu-util -d feed:6969 -a 0 -t 512 -D tex_yoda2_default.bin -R
```

The bootloader checks the key only at reset for application selection; its live
key reads change blink speed while in DFU. QMK's software reset does not force
DFU without the held recovery key. `qmk flash` has no custom command configured.

## Platform and verification

Keep the local fixes in both `lib/chibios` and `lib/chibios-contrib` with this
keyboard: HT32 USB low-level/control-transfer fixes, HSE startup, and corrected
CPU-delay clock selection. Do not discard the submodule changes.

The three-layer default keymap builds and was programmed/readback-verified over
SWD. A compiled-image check verifies all three requested base changes, all four
Vim overrides, all 26 Fn overrides, transparent defaults and unused intersections.
The earlier base keymap downloaded through DFU (23,684 bytes),
completed manifestation with status OK, and reset into enumerated keyboard and
mouse HID collections. This required fixing early status polling in the Rust
bootloader. Physical typing/clicks, Caps Lock LED and suspend/resume await
board-side verification. TrackPoint implementation status is recorded above.
