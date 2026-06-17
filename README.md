# Implement ANN in Control Motor

## 1. Project Introduction

This project applies an Artificial Neural Network (ANN) to support cascade PID motor control. Instead of using one fixed PID gain set for all operating conditions, the system uses motor response data to train an ANN model that predicts suitable PID parameters for different setpoints.

The project focuses on the practical control workflow: collecting motor data, tuning cascade PID gains, preparing the training dataset, training the ANN model, and using the predicted gains to improve motor position control.

---

## 2. Engineering Problem

In a cascade PID motor control system, the position loop and velocity loop affect each other. Tuning both loops separately can be difficult because the inner velocity loop influences the response of the outer position loop.

The main problem of this project is to identify suitable PID gains for different setpoints and use those data to train an ANN model. The trained model can then predict appropriate control parameters instead of relying on a single fixed-gain PID controller.

---

## 3. Methodology

The project method includes the following steps:

```text
Motor setpoint
      ↓
Run cascade PID control
      ↓
Collect motor response data
      ↓
Tune PID gains for different setpoints
      ↓
Prepare ANN training dataset
      ↓
Train ANN regression model
      ↓
Predict suitable PID parameters
      ↓
Apply predicted gains to motor control
```

---

## 4. Cascade PID Control Structure

The motor is controlled using a cascade PID structure:

```text
Position Setpoint
      ↓
Outer Position PID
      ↓
Velocity Setpoint
      ↓
Inner Velocity PID
      ↓
Motor Driver / PWM Command
      ↓
Motor Response
```

The outer loop controls motor position, while the inner loop controls motor velocity. This structure improves tracking response compared to using only one position PID loop.

---

## 5. ANN-Based Gain Prediction

The ANN is trained using input features extracted from motor control conditions and output labels representing suitable control gains.

The model learns the relationship:

```text
Setpoint / response features → ANN → PID gain parameters
```

After training, the ANN can estimate suitable PID gains for new setpoints. This allows the controller to adapt to different operating conditions more flexibly than a fixed-gain controller.

---

## 6. Data Processing

The dataset is divided into training, validation, and testing sets. Input data is normalized using the mean and standard deviation of the training dataset.

```text
Training data → compute mean and standard deviation
Validation/Test data → apply the same normalization values
```

This ensures that the ANN receives data with consistent scaling during training and testing.

---

## 7. ANN Model Architecture

The ANN model is a regression network with:

```text
Input: 4 features
Hidden layer 1: Dense 8 + ReLU
Dropout: 0.1
Hidden layer 2: Dense 16 + ReLU
Dropout: 0.1
Output: Dense 5 + Linear
```

The model is trained using Mean Squared Error loss and Adam optimizer.

---

## 8. Expected Control Improvement

The expected improvement of this method is better motor response compared to using one fixed PID gain set.

The main evaluation criteria are:

- Position setpoint tracking
- Response time
- Overshoot
- Stability of motor response
- Prediction quality of ANN output

---

## 9. Tools and Technologies

- Python
- Artificial Neural Network
- Cascade PID Control
- Motor Control
- Data Processing
- Visual Studio Code
- Training and validation loss analysis

---

## 10. Project Scope

This project focuses on the method of combining cascade PID control and ANN-based gain prediction. The theoretical background of PID, cascade PID, and ANN structure is documented separately in other project documents.
