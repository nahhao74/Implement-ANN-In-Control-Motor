# PID and Cascade PID Control Theory

## 1. Overview

This document summarizes the basic theory of PID control and Cascade PID control. These control methods are commonly used in motor control, robotics, automation systems, and embedded control applications.

PID control is used to reduce the error between a desired setpoint and the measured system output. Cascade PID improves control performance by using multiple feedback loops, usually an outer loop and an inner loop.

---

## 2. Basic Feedback Control Structure

A closed-loop control system compares the desired value with the measured output:

```math
e(t) = r(t) - y(t)
```

where:

| Symbol | Meaning |
|---|---|
| `r(t)` | Reference input or setpoint |
| `y(t)` | Measured output |
| `e(t)` | Control error |

The controller generates a control signal:

```math
u(t)
```

This signal is sent to the actuator or plant to make the output follow the setpoint.

Basic closed-loop structure:

```text
Setpoint r(t) ──► Error e(t) ──► Controller ──► Plant ──► Output y(t)
                      ▲                                       │
                      └──────────────── Feedback ◄────────────┘
```

---

## 3. PID Controller

A PID controller consists of three terms:

- Proportional term
- Integral term
- Derivative term

The continuous-time PID control law is:

```math
u(t) = K_p e(t) + K_i \int_0^t e(\tau)d\tau + K_d \frac{de(t)}{dt}
```

where:

| Symbol | Meaning |
|---|---|
| `Kp` | Proportional gain |
| `Ki` | Integral gain |
| `Kd` | Derivative gain |
| `u(t)` | Controller output |
| `e(t)` | Control error |

---

## 4. Role of Each PID Term

### 4.1 Proportional Term

```math
u_P(t) = K_p e(t)
```

The proportional term reacts directly to the current error.

Effects of increasing `Kp`:

- Faster response
- Smaller rise time
- Larger overshoot if too high
- Possible oscillation or instability if excessive

---

### 4.2 Integral Term

```math
u_I(t) = K_i \int_0^t e(\tau)d\tau
```

The integral term accumulates error over time.

Main purpose:

- Eliminates steady-state error

Effects of increasing `Ki`:

- Better steady-state accuracy
- Larger overshoot
- Slower settling if too high
- Possible integral windup

---

### 4.3 Derivative Term

```math
u_D(t) = K_d \frac{de(t)}{dt}
```

The derivative term predicts the future trend of the error.

Main purpose:

- Reduces overshoot
- Improves damping
- Helps stabilize fast-changing systems

Effects of increasing `Kd`:

- Less overshoot
- Better transient damping
- More sensitivity to measurement noise if too high

---

## 5. Discrete PID Controller

In embedded systems, PID is implemented in discrete time with sampling period:

```math
T_s
```

The error at sample `k` is:

```math
e[k] = r[k] - y[k]
```

A common discrete PID form is:

```math
u[k] = K_p e[k] + K_i T_s \sum_{i=0}^{k} e[i] + K_d \frac{e[k] - e[k-1]}{T_s}
```

A practical implementation uses accumulated integral error:

```math
I[k] = I[k-1] + e[k]T_s
```

and derivative error:

```math
D[k] = \frac{e[k] - e[k-1]}{T_s}
```

Then:

```math
u[k] = K_p e[k] + K_i I[k] + K_d D[k]
```

---

## 6. Practical PID Implementation

A basic PID loop can be written as:

```c
error = setpoint - feedback;

integral += error * Ts;

derivative = (error - prev_error) / Ts;

output = Kp * error + Ki * integral + Kd * derivative;

prev_error = error;
```

For motor control, `output` can be:

- PWM duty cycle
- Motor voltage command
- Motor current command
- Speed reference for another controller

---

## 7. Common PID Issues

### 7.1 Overshoot

Overshoot occurs when the output exceeds the setpoint.

Possible causes:

- `Kp` too high
- `Ki` too high
- Insufficient damping
- Actuator saturation

---

### 7.2 Steady-State Error

Steady-state error is the remaining error after the transient response.

Possible solutions:

- Increase `Ki`
- Improve model or actuator capability
- Reduce friction or load disturbance

---

### 7.3 Integral Windup

Integral windup occurs when the integral term continues to grow while the actuator is saturated.

Example:

```text
Controller output wants 150%
PWM limit is 100%
Integral term keeps increasing
System overshoots after saturation ends
```

Common solutions:

- Limit the integral term
- Stop integration during saturation
- Use anti-windup compensation

Simple integral limit:

```c
if (integral > I_MAX) integral = I_MAX;
if (integral < I_MIN) integral = I_MIN;
```

---

### 7.4 Derivative Noise

The derivative term is sensitive to noise because it reacts to rapid signal changes.

Common solutions:

- Apply low-pass filtering
- Use derivative on measurement instead of derivative on error
- Reduce `Kd`

Filtered derivative example:

```math
D_f[k] = \alpha D_f[k-1] + (1-\alpha)D[k]
```

where:

```math
0 < \alpha < 1
```

---

## 8. PID Tuning Guidelines

A common manual tuning process is:

1. Set `Ki = 0` and `Kd = 0`.
2. Increase `Kp` until the system responds quickly but does not oscillate excessively.
3. Add `Ki` to remove steady-state error.
4. Add `Kd` to reduce overshoot and oscillation.
5. Test the system under different setpoints and loads.

General effects:

| Gain | Rise Time | Overshoot | Settling Time | Steady-State Error |
|---|---|---|---|---|
| Increase `Kp` | Decreases | Increases | May increase | Decreases |
| Increase `Ki` | Decreases | Increases | Increases | Eliminates |
| Increase `Kd` | Small effect | Decreases | Decreases | Small effect |

---

## 9. Cascade PID Control

Cascade PID uses two or more control loops connected in layers.

A common motor control structure is:

```text
Position Setpoint
      │
      ▼
Outer PID Controller
(Position Loop)
      │
      ▼
Velocity Setpoint
      │
      ▼
Inner PID Controller
(Velocity Loop)
      │
      ▼
PWM / Voltage Command
      │
      ▼
Motor
      │
      ▼
Position and Velocity Feedback
```

In this structure:

- The outer loop controls position.
- The inner loop controls velocity.
- The output of the outer loop becomes the setpoint of the inner loop.

---

## 10. Why Use Cascade PID?

Cascade PID is useful when a system has multiple related variables.

For a motor system:

- Position changes slowly.
- Velocity changes faster.
- Motor voltage or PWM directly affects acceleration and velocity.

Using a velocity loop inside a position loop improves control quality because the inner loop handles fast motor dynamics first.

Benefits:

- Better disturbance rejection
- Smoother motor response
- Improved setpoint tracking
- Reduced overshoot
- Easier separation between position and velocity control

---

## 11. Cascade PID Equations

### 11.1 Outer Position Loop

The position error is:

```math
e_{\theta}[k] = \theta_{ref}[k] - \theta[k]
```

The position controller output is the velocity setpoint:

```math
\omega_{ref}[k] =
K_{p,\theta}e_{\theta}[k]
+
K_{i,\theta}\sum e_{\theta}[k]T_s
+
K_{d,\theta}\frac{e_{\theta}[k]-e_{\theta}[k-1]}{T_s}
```

where:

| Symbol | Meaning |
|---|---|
| `theta_ref` | Desired motor position |
| `theta` | Measured motor position |
| `omega_ref` | Velocity setpoint generated by outer loop |

---

### 11.2 Inner Velocity Loop

The velocity error is:

```math
e_{\omega}[k] = \omega_{ref}[k] - \omega[k]
```

The velocity controller output is the motor command:

```math
u[k] =
K_{p,\omega}e_{\omega}[k]
+
K_{i,\omega}\sum e_{\omega}[k]T_s
+
K_{d,\omega}\frac{e_{\omega}[k]-e_{\omega}[k-1]}{T_s}
```

where:

| Symbol | Meaning |
|---|---|
| `omega_ref` | Desired motor velocity |
| `omega` | Measured motor velocity |
| `u` | PWM, voltage, or torque command |

---

## 12. Cascade PID Timing Requirement

The inner loop should be faster than the outer loop.

Typical rule:

```text
Inner loop bandwidth should be 5 to 10 times higher than outer loop bandwidth.
```

For example:

| Loop | Purpose | Sampling / Response |
|---|---|---|
| Inner loop | Velocity control | Fast |
| Outer loop | Position control | Slower |

This is important because the outer loop assumes that the inner loop can track the velocity command quickly.

---

## 13. Cascade PID Tuning Procedure

A common tuning process is:

### Step 1: Tune the Inner Velocity Loop

Disable or bypass the outer loop first.

Tune the velocity PID so the motor can track velocity setpoints quickly and stably.

Main goals:

- Fast velocity response
- Low oscillation
- Low steady-state velocity error

---

### Step 2: Tune the Outer Position Loop

After the velocity loop works well, enable the position loop.

Tune the position PID so the motor reaches the desired position smoothly.

Main goals:

- Accurate position tracking
- Low overshoot
- Short settling time
- Stable response at different setpoints

---

### Step 3: Test the Full Cascade System

Test different position setpoints and load conditions.

Evaluate:

- Rise time
- Settling time
- Overshoot
- Position error
- Velocity response
- Control signal saturation

---

## 14. Cascade PID for Motor Position Control

For motor position control, the feedback signals are usually:

| Signal | Source |
|---|---|
| Position | Encoder count converted to angle |
| Velocity | Difference of position over time |
| Motor command | PWM or voltage signal |

Position calculation:

```math
\theta[k] = \frac{2\pi N[k]}{PPR}
```

Velocity calculation:

```math
\omega[k] = \frac{\theta[k] - \theta[k-1]}{T_s}
```

where:

| Symbol | Meaning |
|---|---|
| `N[k]` | Encoder count |
| `PPR` | Pulses per revolution |
| `Ts` | Sampling time |

---

## 15. Practical Cascade PID Notes

### 15.1 Saturation

Both loops should include output limits.

Example:

```text
Outer loop output limit: maximum allowed velocity
Inner loop output limit: maximum PWM or voltage
```

This prevents unsafe or unrealistic motor commands.

---

### 15.2 Anti-Windup

Integral terms should be limited in both loops.

This is important when:

- Motor command saturates
- Position setpoint is too large
- Load torque is high
- The motor cannot reach the requested speed

---

### 15.3 Setpoint Filtering

Sudden setpoint changes can cause aggressive motion.

A filtered setpoint or trajectory generator can improve smoothness.

Example:

```text
Position step command → smooth ramp command
```

---

### 15.4 Velocity Estimation

Velocity calculated from encoder difference can be noisy.

Common solutions:

- Moving average filter
- Low-pass filter
- Encoder count over fixed time window
- Observer-based velocity estimation

---

## 16. PID vs Cascade PID

| Feature | Single PID | Cascade PID |
|---|---|---|
| Structure | One feedback loop | Two or more feedback loops |
| Example target | Position only | Position and velocity |
| Tuning complexity | Lower | Higher |
| Performance | Good for simple systems | Better for systems with fast inner dynamics |
| Disturbance rejection | Moderate | Better |
| Motor control quality | Basic | Smoother and more robust |

---

## 17. Summary

PID control is a basic closed-loop control method that uses proportional, integral, and derivative actions to reduce tracking error.

Cascade PID extends this idea by using multiple control loops. In motor control, the outer loop usually controls position and generates a velocity setpoint, while the inner loop controls velocity and generates the motor command.

For motor systems, Cascade PID can improve tracking performance, reduce overshoot, and provide better disturbance rejection compared to a single fixed PID controller.
