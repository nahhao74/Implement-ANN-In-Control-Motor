# NF5475 Servo Motor with Encoder

## 1. Overview

The NF5475 is a compact DC servo motor with an integrated encoder, commonly used in small motion-control, positioning, and feedback-control applications. The motor belongs to the NISCA / Canon Finetech Nisca servo motor family and is designed for applications that require speed or position feedback.

This document summarizes the main technical information, operating characteristics, encoder information, and possible usage of the NF5475 motor for control and robotics projects.

---

## 2. Motor Type

The NF5475 is a DC servo motor with encoder feedback.

Main characteristics:

- DC motor
- Built-in encoder
- Ball bearing support
- Permanent magnet rotor
- Suitable for closed-loop speed and position control
- Commonly used in precision motion mechanisms

Because the motor includes encoder feedback, it can be used in control systems such as:

- PID speed control
- PID position control
- Cascade PID control
- Motor response data collection
- Servo positioning experiments

---

## 3. General Specifications

According to Canon Finetech Nisca product information, the NF5475 motor family has the following general specifications:

| Parameter | Value |
|---|---|
| Motor family | NF5475 with encoder |
| Rated voltage | 24 VDC |
| E-type voltage | 38 VDC |
| Weight | 700 g |
| Bearing | Ball bearing on both sides |
| Permanent magnet | 4 poles |
| Encoder type | Optical incremental encoder |
| Encoder supply voltage | 5 V |
| Encoder current | Max 50 mA |

---

## 4. Model Variants

The NF5475 family includes several model variants, such as:

- NA5475F
- NF5475A
- NF5475E

Each variant has different torque, speed, current, output power, and efficiency characteristics.

---

## 5. Performance at Maximum Efficiency

| Model | Torque (mN.m) | Speed (rpm) | Current (A) | Output (W) | Efficiency (%) |
|---|---:|---:|---:|---:|---:|
| NA5475F | 71.07 | 3234 | 1.494 | 24.07 | 67.1 |
| NF5475A | 81.40 | 3199 | 1.617 | 27.27 | 70.3 |
| NF5475E | 74.83 | 4162 | 1.256 | 32.62 | 68.3 |

These values describe the operating point where the motor reaches maximum efficiency.

---

## 6. Continuous Operation Capability

At continuous operation condition, the possible maximum output is:

| Model | Possible Maximum Output (W) |
|---|---:|
| NA5475F | 33.7 |
| NF5475A | 38.9 |
| NF5475E | 38.9 |

This means the NF5475 is suitable for small to medium power motion-control systems, not for high-power traction applications.

---

## 7. No-Load Characteristics

| Model | No-Load Speed (rpm) | No-Load Current (A) |
|---|---:|---:|
| NA5475F | 3818 | 0.270 |
| NF5475A | 3716 | 0.261 |
| NF5475E | 4884 | 0.218 |

The no-load speed shows the approximate free-running speed when the motor is powered without external mechanical load.

---

## 8. Starting Torque

| Model | Starting Torque (mN.m) | Starting Current (A) |
|---|---:|---:|
| NA5475F | 464.10 | 8.264 |
| NF5475A | 585.10 | 10.003 |
| NF5475E | 506.57 | 7.246 |

The starting current is much higher than the operating current. Therefore, the motor driver and power supply must be selected with enough current margin.

---

## 9. Encoder Characteristics

The NF5475 uses an optical incremental encoder.

| Parameter | Value |
|---|---|
| Output phase | 2 phases |
| Resolution options | 100 P/R or 200 P/R |
| Supply voltage | 5 V |
| Encoder current | Max 50 mA |
| Frequency response | 10 kHz or 20 kHz |
| Signal type | Incremental encoder |

A product listing for the NF5475 200PPR version also describes it as having a 200-pulse encoder and a pre-mounted timing pulley.

---

## 10. Encoder Use in Control

The encoder provides feedback pulses that can be used to calculate motor position and speed.

### Position Calculation

If the encoder resolution is `PPR`, the angular position can be calculated as:

```math
theta = 2*pi*N/PPR
```

where:

| Symbol | Meaning |
|---|---|
| `theta` | Motor shaft angle in radians |
| `N` | Encoder count |
| `PPR` | Encoder pulses per revolution |

For quadrature decoding, the effective count may increase depending on the decoding mode:

| Decoding Mode | Effective Count |
|---|---|
| x1 | `PPR` |
| x2 | `2 * PPR` |
| x4 | `4 * PPR` |

For example, with a 200 P/R encoder and x4 decoding:

```text
Effective resolution = 200 * 4 = 800 counts/rev
```

---

### Speed Calculation

Motor speed can be estimated from the change in encoder count:

```math
omega = (theta[k] - theta[k-1]) / Ts
```

where:

| Symbol | Meaning |
|---|---|
| `omega` | Angular velocity |
| `theta[k]` | Current position |
| `theta[k-1]` | Previous position |
| `Ts` | Sampling period |

In rpm:

```math
RPM = (Delta_N / PPR) * (60 / Ts)
```

For quadrature decoding, use the effective count per revolution instead of the base PPR.

---

## 11. Motor Driver Requirements

When selecting a motor driver for the NF5475, the following points should be considered:

- The driver voltage must match the motor voltage.
- The driver must handle the motor starting current.
- The driver should support bidirectional control if position control is required.
- PWM control is useful for speed control.
- Current protection is recommended.
- Encoder inputs must be read by a microcontroller or motion controller.

For the 24 V NF5475 variant, a suitable driver should support:

```text
Motor voltage: 24 VDC
Peak current: higher than starting current
Control input: PWM or analog control
Direction control: required for position systems
```

---

## 12. Application in Control Projects

The NF5475 motor is suitable for projects that require feedback control, such as:

- Motor speed control
- Motor position control
- Cascade PID control
- Encoder-based feedback systems
- Data collection for ANN-based motor control
- Small servo mechanisms
- Educational control experiments

In a cascade PID motor control project, the encoder can provide:

```text
Position feedback -> outer PID loop
Velocity feedback -> inner PID loop
```

The control structure can be:

```text
Position Setpoint
        ↓
Outer Position PID
        ↓
Velocity Setpoint
        ↓
Inner Velocity PID
        ↓
Motor Driver
        ↓
NF5475 Motor
        ↓
Encoder Feedback
```

---

## 13. Notes for Practical Use

Important practical notes:

- Do not connect the encoder directly to a voltage higher than 5 V.
- Use common ground between encoder, motor driver, and microcontroller.
- The motor starting current is high, so the power supply must not be undersized.
- Encoder signals should be filtered or debounced in software if noise appears.
- For accurate speed calculation, use a fixed sampling time.
- Mechanical load should be matched with the motor torque capability.
- Use current limiting or protection when testing the motor.

---

## 14. Summary

The NF5475 is a compact DC servo motor with optical incremental encoder feedback. It operates mainly at 24 VDC, with an E-type variant at 38 VDC. The motor has a weight of about 700 g, uses ball bearings, and includes a 4-pole permanent magnet structure.

With encoder feedback, the NF5475 is suitable for closed-loop motor control experiments such as PID, cascade PID, and ANN-based gain prediction projects. However, because its output power is in the tens of watts range, it is better suited for small motion-control systems rather than high-power mobile robot traction.

---

## 15. References

- Canon Finetech Nisca - NF5475 with Encoder: https://ftn.canon/en/product/motor/encoder.html
- Motor2Hand - Dong co DC Nisca DC servo Motor NF5475 200PPR: https://motor2hand.com/san-pham/dong-co-dc-nisca-dc-servo-motor-nf5475-200ppr-sp652357.html
- Scribd - NF5475 Servo Motor with Encoder Data: https://www.scribd.com/doc/175650982/servomotor-NF5475
