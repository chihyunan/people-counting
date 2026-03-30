# People Counting Integrated

This folder contains the merged ESP32 people-counting sketch that combines:

- `people-counting-chihyun` for break-beam direction detection
- `people-counting` for Grid-EYE thermal validation and OLED output

The original source folders were left untouched. Untouched reference copies are also stored here under:

- `reference/people-counting/`
- `reference/people-counting-chihyun/`

## Integration Summary

The integrated design uses break-beam as the trigger and Grid-EYE as the validator.

Break-beam is still the first sensor that decides "something passed through the doorway" because it is fast and interrupt-driven. Grid-EYE runs continuously in the background and stores recent signed crossing events with timestamps. When break-beam returns `+1` or `-1`, the integrated sketch looks at Grid-EYE events in a short time window around that beam event and decides whether Grid-EYE should override the beam result.

Final rule:

- If Grid-EYE and break-beam agree on direction, Grid-EYE wins.
- If they disagree, break-beam wins.
- If Grid-EYE sees nothing in the window, break-beam wins.

This means Grid-EYE can replace a beam result of `+1` with `+2`, or `-1` with `-2`, if the thermal events in that window support the same direction.

Occupancy starts at `0` and is allowed to go negative.

## Files In This Folder

- `people-counting-integrated.ino`
  Main merged sketch.
- `ir_beam.cpp`
  Copied break-beam implementation.
- `ir_beam.h`
  Copied break-beam header.
- `src/eyegrid/eyegrid.h`
  Refactored Grid-EYE helper for continuous sampling and event buffering.
- `src/oled/oled_display.h`
  OLED helper used by the integrated sketch.

## Main Runtime Logic

### 1. Break-beam still detects the doorway event

Break-beam logic was copied from `people-counting-chihyun` without changing its detection behavior.

Relevant code:

- `ir_beam.cpp:3-4`
  Pin definitions for `PIN_A` and `PIN_B`
- `ir_beam.cpp:17-25`
  Interrupt setup
- `ir_beam.cpp:52-95`
  `getDirectionalCount()`, which returns:
  - `+1` for entry
  - `-1` for exit
  - `0` for no valid event

### 2. Grid-EYE now runs continuously

The original Grid-EYE code only exposed a `poll()` function that returned immediate `entered/exited` counts. In the integrated version, Grid-EYE was changed to run as a background sampler.

Relevant code:

- `src/eyegrid/eyegrid.h:25-30`
  Main thermal thresholds and timing constants
- `src/eyegrid/eyegrid.h:32-33`
  Rolling event buffer storage
- `src/eyegrid/eyegrid.h:109-144`
  `update()`, which:
  - reads a thermal frame every `80 ms`
  - finds the hottest pixel
  - maps that pixel into `Out`, `Mid`, or `In`
  - drives the state machine continuously
- `src/eyegrid/eyegrid.h:146-165`
  `consumeWindowDelta()`, which sums Grid-EYE events in a time window and marks them as used

### 3. The original Grid-EYE FSM was preserved, but its output changed

The original Grid-EYE state machine still detects:

- `Out -> Mid -> In` as entry
- `In -> Mid -> Out` as exit

But instead of immediately returning `entered/exited` counters, it now records signed events into a rolling buffer.

Relevant code:

- `src/eyegrid/eyegrid.h:56-107`
  `processZone()`
- `src/eyegrid/eyegrid.h:13-17`
  `SignedEvent`
- `src/eyegrid/eyegrid.h:51-54`
  `recordEvent()`

## Beam + Grid-EYE Merge Logic

The merged decision logic lives in `people-counting-integrated.ino`.

### Queueing beam events

When break-beam returns a nonzero result, the sketch does not update occupancy immediately. It stores the beam event in a short queue and waits for the Grid-EYE lookahead window to pass.

Relevant code:

- `people-counting-integrated.ino:7-10`
  Merge timing constants
- `people-counting-integrated.ino:12-20`
  Pending beam event queue structure and storage
- `people-counting-integrated.ino:71-86`
  `enqueueBeamEvent()`

### Finalizing a beam event

After the lookahead window expires, the sketch totals Grid-EYE events around that beam timestamp and chooses the final result.

Relevant code:

- `people-counting-integrated.ino:50-69`
  `finalizeOldestPending()`
- `people-counting-integrated.ino:28-30`
  `sameSign()`

Decision rule in code:

- same sign -> use Grid-EYE total
- different sign -> use break-beam result

### Updating occupancy and display

Once a final signed delta is chosen, the sketch updates:

- `occupancy`
- `totalEntered`
- `totalExited`
- OLED display
- serial debug output

Relevant code:

- `people-counting-integrated.ino:22-26`
  Main counters and state
- `people-counting-integrated.ino:32-48`
  `applyFinalDelta()`

## OLED Output

The OLED now explicitly shows:

- `Entry`
- `Exit`
- `Room`

`Room` is the final occupancy value, computed as `Entry - Exit`.

Relevant code:

- `src/oled/oled_display.h:17-30`
  `showCounts()`

## Setup And Loop

### setup()

At startup the sketch:

- starts serial
- sets up the break-beam interrupts
- starts the OLED
- initializes the Grid-EYE sensor
- resets the Grid-EYE tracker state

Relevant code:

- `people-counting-integrated.ino:100-117`

### loop()

On each pass through `loop()` the sketch:

1. updates Grid-EYE continuously
2. checks whether break-beam produced a directional event
3. queues the beam event if needed
4. finalizes any queued beam events whose Grid-EYE window has expired
5. prints a periodic heartbeat

Relevant code:

- `people-counting-integrated.ino:119-139`

## Pin And Bus Configuration

### Break-beam pins

Update these in `ir_beam.cpp`:

- `ir_beam.cpp:3`
  `PIN_A`
- `ir_beam.cpp:4`
  `PIN_B`

### I2C pins for OLED and Grid-EYE

Default I2C pins are currently:

- `SDA = 21`
- `SCL = 22`

These defaults are defined in:

- `src/eyegrid/eyegrid.h:41-43`
  `Eyegrid::start(int sda = 21, int scl = 22)`
- `src/oled/oled_display.h:11-15`
  `Oled::begin(int sda = 21, int scl = 22, uint8_t i2cAddr = 0x3C)`

If you want to override them from the sketch, edit:

- `people-counting-integrated.ino:105`
  `Oled::begin();`
- `people-counting-integrated.ino:108`
  `Eyegrid::start();`

## Major Changes From The Original Files

### From `people-counting-chihyun`

Kept:

- break-beam detection logic
- interrupt handlers
- directional return values

Changed in integration:

- break-beam no longer updates occupancy directly
- break-beam now feeds a pending validation queue in `people-counting-integrated.ino`

### From `people-counting`

Kept:

- the basic zone-crossing FSM idea
- I2C startup pattern
- OLED helper structure

Changed in integration:

- removed the old `PeopleDelta { entered, exited }` output model
- replaced it with timestamped signed events
- changed Grid-EYE from foreground polling to continuous background updates
- added consume-once event matching around each break-beam trigger

## Compile Status

This integrated sketch was compile-checked with:

- board target: `esp32:esp32:esp32`

If upload issues happen in Arduino IDE, the sketch itself is already known to compile.
