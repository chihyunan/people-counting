# People Counting Integrated

Your project documentation and README structure are well-organized. Below is a polished, professional GitHub-style README based on your specific engineering decisions, fusion logic, and the "Hero image" requirements.

***

# Room Occupancy Counter
### Door-mounted, real-time, WiFi-capable sensor system using ESP32, Dual IR Break Beams, and GridEye Thermal Fusion.



## 1. Overview
This system provides a reliable, low-power solution for monitoring room occupancy in real-time. By utilizing **sensor fusion**, the device overcomes the limitations of single-modality sensors: **IR Break Beams** provide microsecond-latency movement detection, while an **AMG8833 GridEye Thermal Camera** resolves complex edge cases, such as side-by-side entry or tailgating. 

Developed as the final project for **EECS 300: Electrical Engineering Systems Design II** at the University of Michigan by a five-person team.

## 2. System Architecture
The system utilizes an interrupt-driven architecture to ensure no movement events are missed, paired with a polling-based thermal pipeline for validation.



### Hardware Stack
* **MCU:** ESP32 (chosen over Arduino for dual-core processing and integrated WiFi).
* **Primary Sensing:** 2x IR Break Beam sensors (flipped orientation to eliminate optical crosstalk).
* **Secondary Sensing:** AMG8833 GridEye (8x8 Thermal Array).
* **Power:** 5V Power Bank (regulated via voltage dividers for 3.3V ESP32 logic).
* **Display:** I2C OLED Screen for local count visualization.

## 3. Design Decisions
* **Sensor Orientation:** We flipped one break beam sensor (1 TX + 1 RX on each side). Our testing showed signal failure at >10° misalignment; flipping prevents one transmitter from accidentally triggering the adjacent receiver.
* **Primary vs. Secondary:** Break beams act as the primary trigger due to their **100µs response time**. The GridEye is secondary, invoked only when the beams are broken to validate "blob count" (e.g., distinguishing one person from two people walking shoulder-to-shoulder).
* **Interrupt Logic:** We implemented a **5ms software noise filter**. Since an EMI spike is ~30µs and a fast human move is ~15ms, this 5ms window provides a **167x safety factor** against false triggers.
* **Mechanical Mount:** A Velcro-based moment-balance design was chosen. Calculations confirm a **10x Safety Factor** for the cantilevered arm, ensuring stability without permanent door modification.

## 4. Performance & Edge Cases
The system is designed to handle the "Chaos of the Doorway" through the following logic:

| Condition | Sensor Behavior | System Output |
| :--- | :--- | :--- |
| **Standard Entry** | Beam A then B | +1 Occupant |
| **Fast Run-through** | Interrupts triggered <10ms | +1 Occupant |
| **Side-by-Side Entry** | Beams broken once; GridEye sees 2 blobs | +2 Occupants |
| **Partial Entry** | Beam A broken, then A cleared (B never hit) | 0 (No change) |
| **Stationary Occupant** | Beams clear; GridEye detects heat signature | Maintains Count |

## 5. Engineering Specifications (Quantitative)
As an engineering project, the design is backed by empirical data:
* **Interrupt Latency:** ~2µs (calculated at 160MHz), negligible relative to human movement.
* **Thermal FOV:** 60° (covers ~2.3m floor width at a 2m mount height), ensuring full doorway coverage.
* **Alignment Tolerance:** Stable reception up to 10° misalignment at 1m distance.

## 6. Mathematical Foundations
To ensure the 3.3V ESP32 pins were protected from the 5V sensor output, the following voltage divider was implemented:

$$V_{out} = V_{in} \cdot \frac{R_2}{R_1 + R_2}$$
*Where $V_{in} = 5V$, $R_1 = 1.8k\Omega$, $R_2 = 3.3k\Omega$, resulting in $V_{out} \approx 3.23V$.*

The cantilevered mount stability was verified using the moment balance equation:
$$\sum M = 0 \implies F_{velcro} \cdot d_1 > F_{gravity} \cdot d_2$$
Our testing confirmed a 10x safety margin ($SF \approx 10$) using a single 2-inch Velcro strip.

## 7. How to Run
### Repository Structure
```text
├── /hardware
│   ├── /pcb          <-- Altium Designer files & BOM
│   ├── /mount        <-- SolidWorks CAD & GD&T
│   └── /images       <-- Schematics & Renders
└── /firmware
    ├── /breakbeam    <-- Standalone driver
    ├── /grideye      <-- AMG8833 driver
    └── integrated.ino <-- Main Fusion Logic
```

### Installation
1.  **Dependencies:** Install `Adafruit_AMG88xx` and `Adafruit_SSD1306` via Arduino IDE Library Manager.
2.  **Wiring:** Connect sensors to ESP32 according to the schematic in `/hardware/images`. Ensure the 5V-to-3.3V voltage divider is in place for the IR receivers.
3.  **Flash:** Open `integrated.ino`, select **ESP32 Dev Module**, and upload.
4.  **Monitor:** Access the local IP address displayed on the OLED to view the real-time web dashboard.

---
**Credits:** Created by Team [Number/Name] for EECS 300, University of Michigan.
