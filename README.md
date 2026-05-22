# High-Speed PD Line Follower Robot

A high-speed line follower robot built on Arduino using a 6-channel QTR IR sensor array and a **Proportional-Derivative (PD) control loop** for smooth, fast tracking.

---

## Table of Contents

- [Overview](#overview)
- [Hardware](#hardware)
- [Wiring](#wiring)
- [Control Theory](#control-theory)
- [PD Tuning Guide](#pd-tuning-guide)
- [Software Dependencies](#software-dependencies)
- [How to Flash](#how-to-flash)
- [Configuration](#configuration)
- [Project Structure](#project-structure)

---

## Overview

The robot reads the line position from 6 IR sensors, computes an error from the track center, and applies a PD correction to the left and right motor speeds in real time. A hard-turn override kicks in when the line exits the sensor array entirely, acting as a bang-bang recovery mode.

```
Sensor array → position (0–5000) → error = position − 2500
                                          ↓
                              PD: u = Kp·e + Kd·(e − e_prev)
                                          ↓
              Left speed = BaseSpeed − u      Right speed = BaseSpeed + u
```

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | Arduino (Uno / Nano / compatible) |
| Sensor array | Pololu QTR-8RC or QTR-6RC (6 sensors used) |
| Motor driver | L298N or compatible dual H-bridge |
| Motors | DC gear motors (N20 or similar) |
| Power | 7.4 V LiPo (motors) + regulated 5 V (Arduino) |

---

## Wiring

### Sensor Array → Arduino

| QTR Sensor Pin | Arduino Pin |
|---|---|
| S1 | D3 |
| S2 | D4 |
| S3 | D5 |
| S4 | D6 |
| S5 | D7 |
| S6 | D8 |

> Emitter control is currently left floating (emitters always on). Uncomment `qtr.setEmitterPin(12)` and wire the emitter pin to D12 if you want software-controlled emitters.

### Motor Driver → Arduino

| Signal | Arduino Pin |
|---|---|
| Right motor IN1 | A3 |
| Right motor IN2 | A2 |
| Right motor PWM (ENA) | D10 |
| Left motor IN1 | A0 |
| Left motor IN2 | A1 |
| Left motor PWM (ENB) | D11 |
| Motor power enable | D8 |

---

## Control Theory

### 1. Position sensing

The QTR library computes a weighted centroid of all 6 sensor readings:

```
position = Σ(sensor_i × weight_i) / Σ(sensor_i)
```

Weights are `{0, 1000, 2000, 3000, 4000, 5000}`, so `position` ranges from **0 to 5000**. The line centered under the array reads **2500**.

### 2. Error

```
error = position − 2500
```

Positive error → line is right of center → steer right.
Negative error → line is left of center → steer left.

### 3. PD correction

```
motorSpeed = Kp × error + Kd × (error − lastError)
```

| Term | Role |
|---|---|
| `Kp × error` | Proportional — strength of correction scales with how far off-center the line is |
| `Kd × (error − lastError)` | Derivative — damps overshoot by opposing rapid changes in error |

### 4. Motor mixing

```
rightMotorSpeed = BaseSpeed + motorSpeed
leftMotorSpeed  = BaseSpeed − motorSpeed
```

Both motors average to `BaseSpeed` on a straight line. On a curve, one speeds up and the other slows down by equal amounts.

### 5. Hard-turn override

When the line exits the sensor array (`position < 300` or `position > 4700`), normal PD correction is too slow. The code bypasses the PD loop and commands a full counter-rotation at `speedturn`:

```
position > 4700 → right motor forward, left motor reverse   (spin right)
position < 300  → left motor forward, right motor reverse   (spin left)
```

---

## PD Tuning Guide

Start with `Kd = 0`, tune `Kp` first, then add `Kd`.

| Symptom | Cause | Fix |
|---|---|---|
| Wiggles/oscillates on straights | Kp too high | Lower Kp |
| Drifts wide on curves | Kp or Kd too low | Raise Kp, then Kd |
| Overshoots after sharp turns | Kd too low | Raise Kd |
| Jerky / unstable at speed | Kd too high | Lower Kd slightly |
| Falls off line on tight turns | `speedturn` too low | Raise `speedturn` |

**Current tuned values:**

```cpp
#define Kp        0.0005
#define Kd        0.092
#define BaseSpeed 210
#define MaxSpeed  230
#define speedturn 180
```

---

## Software Dependencies

- [QTRSensors](https://github.com/pololu/qtr-sensors-arduino) by Pololu

Install via Arduino Library Manager:
`Sketch → Include Library → Manage Libraries → search "QTRSensors"`

---

## How to Flash

1. Clone this repository:
   ```bash
   git clone https://github.com/devashishbuilds/fast-line-follower.git
   ```
2. Open `new_bot.ino` in the Arduino IDE.
3. Install the QTRSensors library (see above).
4. Select your board and COM port under `Tools`.
5. Upload.

On power-up the robot runs **calibration for ~6 seconds** (300 × 20 ms cycles). Slowly sweep the robot side-to-side over the track during this window so all sensors capture both black and white. The onboard LED (D13) lights up when calibration is complete and the robot starts following.

---

## Configuration

All tunable parameters are `#define` constants at the top of the sketch:

```cpp
#define Kp        0.0005   // Proportional gain
#define Kd        0.092    // Derivative gain
#define MaxSpeed  230      // PWM ceiling (0–255)
#define BaseSpeed 210      // Nominal forward speed
#define speedturn 180      // Speed used during hard-turn recovery
```

To change sensor pins, update the array passed to `qtr.setSensorPins()`:

```cpp
qtr.setSensorPins((const uint8_t[]){8, 7, 6, 5, 4, 3}, SensorCount);
```

---

## Project Structure

```
.
├── new_bot.ino   # Main Arduino sketch
└── README.md
```

---

## License

MIT License — free to use, modify, and distribute.
