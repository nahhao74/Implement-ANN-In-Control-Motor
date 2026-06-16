# ANN Model Architecture

## 1. Overview

This document describes the Artificial Neural Network (ANN) architecture used for predicting suitable control parameters from motor response data.

The ANN is designed as a regression model. Its purpose is to learn the relationship between input motor/control features and output control gains or parameters.

General objective:

```text
Input motor/control features → ANN model → Predicted control parameters
```

---

## 2. ANN Concept

An Artificial Neural Network is a computational model composed of connected layers of neurons. Each neuron receives input values, applies a weighted sum, adds a bias, and passes the result through an activation function.

For one neuron:

```math
z = w_1x_1 + w_2x_2 + \cdots + w_nx_n + b
```

or in vector form:

```math
z = Wx + b
```

where:

| Symbol | Meaning |
|---|---|
| `x` | Input vector |
| `W` | Weight matrix |
| `b` | Bias vector |
| `z` | Linear output before activation |

The activated output is:

```math
a = f(z)
```

where `f` is the activation function.

---

## 3. Model Input

The model uses 4 input features:

```math
x =
\begin{bmatrix}
x_1 \\
x_2 \\
x_3 \\
x_4
\end{bmatrix}
```

These inputs can represent motor setpoint information, feedback data, or extracted control features depending on the dataset.

In the implementation:

```python
x_train = _Data_train[:, 1:5]
```

Therefore, the input dimension is:

```text
Input size = 4
```

---

## 4. Model Output

The model produces 5 output values:

```math
y =
\begin{bmatrix}
y_1 \\
y_2 \\
y_3 \\
y_4 \\
y_5
\end{bmatrix}
```

In the implementation:

```python
y_train = _Data_train[:, 5:]
```

Therefore, the output dimension is:

```text
Output size = 5
```

For a motor control application, these outputs can represent predicted controller parameters such as PID gains or related tuning values.

---

## 5. Data Normalization

Before training, the input data is normalized using the mean and standard deviation of the training data:

```math
x_{norm} = \frac{x - \mu}{\sigma + \epsilon}
```

where:

| Symbol | Meaning |
|---|---|
| `x` | Original input data |
| `mu` | Mean value of training data |
| `sigma` | Standard deviation of training data |
| `epsilon` | Small value to avoid division by zero |

In the code:

```python
EPS = 1e-8

def normalize(X):
    mean = np.mean(X, axis=0)
    std  = np.std(X, axis=0)
    X_norm = (X - mean) / (std + EPS)
    return X_norm, mean, std
```

The validation and test data use the same training mean and standard deviation:

```python
X_norm_test = normalize_apply(x_test, x_mean, x_std)
X_norm_valid = normalize_apply(x_valid, x_mean, x_std)
```

This is important because validation, test, and real-time inputs must be scaled using the same reference statistics from the training data.

---

## 6. ANN Architecture

The implemented ANN architecture is:

```text
Input Layer: 4 features
        ↓
Dense Layer: 8 neurons
        ↓
ReLU Activation
        ↓
Dropout: 0.1
        ↓
Dense Layer: 16 neurons
        ↓
ReLU Activation
        ↓
Dropout: 0.1
        ↓
Dense Layer: 5 neurons
        ↓
Linear Activation
        ↓
Output Layer: 5 values
```

The model in code:

```python
Model = NNModel.Model()

Model.add(layer.Layer_Dense_Regularization(
    4, 8,
    weights_regularizer_l2=5e-4
))
Model.add(activation.Activation_ReLU())
Model.add(layer.Dense_Dropout(0.1))

Model.add(layer.Layer_Dense_Regularization(
    8, 16,
    weights_regularizer_l2=5e-4
))
Model.add(activation.Activation_ReLU())
Model.add(layer.Dense_Dropout(0.1))

Model.add(layer.Layer_Dense_Regularization(16, 5))
Model.add(activation.Activation_Linear())
```

---

## 7. Layer-by-Layer Explanation

### 7.1 Input Layer

The input layer receives 4 normalized input features:

```text
x1, x2, x3, x4
```

These values are passed into the first dense layer.

---

### 7.2 First Dense Layer

The first dense layer maps the 4 input features into 8 neurons:

```text
Dense(4 → 8)
```

Mathematically:

```math
z_1 = W_1x + b_1
```

where:

| Symbol | Meaning |
|---|---|
| `W1` | Weight matrix of the first dense layer |
| `b1` | Bias vector of the first dense layer |
| `z1` | Output before activation |

---

### 7.3 ReLU Activation

The ReLU activation function is used after the first dense layer:

```math
f(z) = \max(0, z)
```

ReLU helps the model learn nonlinear relationships between the inputs and outputs.

---

### 7.4 Dropout Layer

Dropout randomly disables part of the neurons during training.

In this model:

```text
Dropout rate = 0.1
```

This means 10% of the neurons are randomly dropped during training to reduce overfitting.

---

### 7.5 Second Dense Layer

The second dense layer maps 8 neurons to 16 neurons:

```text
Dense(8 → 16)
```

This layer increases the model capacity so the ANN can learn more complex relationships from the data.

---

### 7.6 Output Layer

The final dense layer maps 16 neurons to 5 outputs:

```text
Dense(16 → 5)
```

The output activation is linear:

```math
f(z) = z
```

A linear activation is suitable because this is a regression problem, not a classification problem.

---

## 8. Regularization

The model uses L2 weight regularization in the hidden layers:

```python
weights_regularizer_l2=5e-4
```

L2 regularization adds a penalty to large weights:

```math
L_{total} = L_{data} + \lambda \sum W^2
```

where:

| Symbol | Meaning |
|---|---|
| `L_total` | Total loss |
| `L_data` | Original prediction loss |
| `lambda` | Regularization factor |
| `W` | Network weights |

This helps reduce overfitting and improves generalization.

---

## 9. Loss Function

The model uses Mean Squared Error (MSE) as the loss function:

```math
MSE = \frac{1}{n}\sum_{i=1}^{n}(y_i - \hat{y}_i)^2
```

where:

| Symbol | Meaning |
|---|---|
| `y_i` | True output |
| `hat(y_i)` | Predicted output |
| `n` | Number of samples |

MSE is suitable for regression problems because it measures the difference between predicted continuous values and target continuous values.

In the code:

```python
loss=loss_function.Loss_MeanSquaredError()
```

---

## 10. Optimizer

The model uses the Adam optimizer:

```python
optimizer=optimization.Optimizer_Adam(
    learning_rate=0.01,
    decay=3e-4
)
```

Adam updates the network weights using adaptive learning rates based on gradient information.

The weight update is based on minimizing the loss function during training.

---

## 11. Training Configuration

The model is trained using:

```python
epochs = 3000
```

Training data is used for updating model weights, while validation data is used to monitor whether the model generalizes well.

```python
history = Model.train(
    X_norm_train, y_train,
    validation_data=(X_norm_valid, y_valid),
    epochs=3000,
    print_every=10
)
```

---

## 12. Prediction Process

After training, the test data is passed through the model:

```python
y_pre = Model.forward(X_norm_test)
```

The prediction process is:

```text
Normalized input data
        ↓
Dense 8 + ReLU
        ↓
Dropout disabled during inference
        ↓
Dense 16 + ReLU
        ↓
Dropout disabled during inference
        ↓
Dense 5 + Linear
        ↓
Predicted output values
```

During inference or real-time operation, the same `x_mean` and `x_std` from training must be used to normalize new input data.

---

## 13. Training and Validation Loss

The training process records:

- Training loss
- Validation loss

These values are plotted to evaluate learning behavior:

```python
plt.plot(history['loss'], label='Train Loss')
plt.plot(history['val_loss'], label='Validation Loss')
```

Interpretation:

| Observation | Meaning |
|---|---|
| Training loss decreases | Model is learning training data |
| Validation loss decreases | Model is generalizing well |
| Training loss low, validation loss high | Possible overfitting |
| Both losses high | Model may be underfitting |

---

## 14. Model Summary

The ANN structure can be summarized as:

| Layer | Type | Size / Function |
|---|---|---|
| 1 | Input | 4 features |
| 2 | Dense | 8 neurons |
| 3 | Activation | ReLU |
| 4 | Dropout | 0.1 |
| 5 | Dense | 16 neurons |
| 6 | Activation | ReLU |
| 7 | Dropout | 0.1 |
| 8 | Dense | 5 neurons |
| 9 | Activation | Linear |

Compact architecture:

```text
Input 4
→ Dense 8 + ReLU
→ Dropout 0.1
→ Dense 16 + ReLU
→ Dropout 0.1
→ Dense 5 + Linear
→ Output 5
```

---

## 15. Application in Motor Control

In the motor control project, the ANN can be used to predict suitable controller parameters from input features.

A typical workflow is:

```text
Motor setpoint and response features
        ↓
Data normalization
        ↓
ANN prediction
        ↓
Predicted control parameters
        ↓
Cascade PID controller
        ↓
Motor response
```

This allows the control system to use different controller parameters for different operating conditions instead of relying on one fixed set of PID gains.

---

## 16. Notes for Real-Time Implementation

For real-time or embedded implementation, the following values must be saved:

```text
x_mean
x_std
weights
biases
```

The real-time prediction process is:

```text
1. Read input features.
2. Normalize inputs using x_mean and x_std.
3. Run forward propagation through the ANN.
4. Use predicted outputs as control parameters.
```

If the ANN is deployed directly on a microcontroller, memory and computation time must be considered.

Important factors:

- Number of layers
- Number of neurons
- Floating-point computation cost
- Available RAM and Flash memory
- Sampling time requirement of the control system

---

## 17. Summary

This ANN is a regression model with 4 input features and 5 output values. It uses two hidden dense layers with ReLU activation, dropout, and L2 regularization. The model is trained using MSE loss and Adam optimizer.

The architecture is suitable for learning nonlinear relationships between motor/control input features and output control parameters. In a cascade motor control system, this allows the controller gains to be adjusted based on different setpoints or operating conditions.
