# Rotary Pendulum Swing-Up Control

This project implements swing-up and balance control for a rotary inverted pendulum system using the Quanser QUBE-Servo 2 platform. The objective is to swing the pendulum from the downward position to the upright region and stabilize it around the equilibrium point.

The system is modeled using a state-space representation with rotary arm angle, pendulum angle, and their angular velocities as feedback states. A velocity-based resonant swing-up controller is used to increase the pendulum oscillation amplitude, while an LQR-based balance controller combined with PID support is applied near the upright position for stabilization.

This project focuses on system modeling, controller design, parameter tuning, and experimental validation of the rotary inverted pendulum control process.

## Key Features

- Rotary inverted pendulum modeling using state-space representation
- Velocity-based resonant swing-up control
- LQR-based upright stabilization
- PID support for response improvement and oscillation reduction
- Quanser QUBE-Servo 2 hardware platform
- MATLAB/Simulink-based modeling, simulation, and control design

## System Overview

The rotary inverted pendulum consists of a motor-driven rotary arm and a passive pendulum attached to the arm. The controller first drives the pendulum from the downward position toward the upright region. Once the pendulum is close enough to the upright equilibrium point, the balance controller is activated to stabilize the system.

The main feedback states are:

- Rotary arm angle
- Pendulum angle
- Rotary arm angular velocity
- Pendulum angular velocity

## Control Strategy

The control process includes two main stages:

1. **Swing-Up Stage**  
   A velocity-based resonant swing-up controller is used to increase the pendulum oscillation amplitude and bring it near the upright position.

2. **Balance Stage**  
   When the pendulum reaches the upright region, an LQR-based controller with PID support is used to stabilize the pendulum around the equilibrium point and reduce oscillation.

## Tools and Technologies

- Quanser QUBE-Servo 2
- MATLAB
- Simulink
- State-space modeling
- LQR control
- PID control
- Rotary inverted pendulum control

## Project Goal

The goal of this project is to understand and implement the complete control process of a rotary inverted pendulum, including system modeling, swing-up control, upright stabilization, and controller tuning.
