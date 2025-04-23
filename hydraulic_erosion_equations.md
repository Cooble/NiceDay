# Equations from "Fast Hydraulic Erosion Simulation and Visualization on GPU"

This document summarizes the main mathematical equations from the paper and provides a description of each variable used.

## Equations and Variable Descriptions

### (1) Height Displacement for Visualization
**Equation:**  
$$ h(x,y) = b(x,y) + d(x,y) $$  
**Description:**  
- \( h(x,y) \): Final surface height at cell (x, y)  
- \( b(x,y) \): Bedrock or terrain base height  
- \( d(x,y) \): Water depth

---

### (2) Water Increment (Rainfall/Source)
**Equation:**  
$$ d_1(x,y) = d^t(x,y) + \Delta t\,r^t(x,y) $$  
**Description:**  
- \( d_1(x,y) \): Updated water depth after rainfall  
- \( d^t(x,y) \): Water depth at time t  
- \( \Delta t \): Time step  
- \( r^t(x,y) \): Rainfall rate

---

### (3) Outflow Flux Update (Pipe Model)
**Equation:**  
$$ f_L^{t+\Delta t}(x,y) = \max\left(0, f_L^t(x,y) + \Delta t\,A\,\frac{g\,\Delta h_L(x,y)}{l} \right) $$  
**Description:**  
- \( f_L \): Outflow flux to the left  
- \( A \): Cross-sectional area coefficient  
- \( g \): Gravitational acceleration  
- \( \Delta h_L \): Height difference between neighboring cells  
- \( l \): Distance between cells

---

### (4) Height Difference in Left Pipe
**Equation:**  
$$ \Delta h_L^t(x,y) = b^t(x,y) + d_1(x,y) - b^t(x-1,y) - d_1(x-1,y) $$  
**Description:**  
- Height difference between current and left neighbor cell

---

### (5) Flux Scaling Factor to Prevent Negative Water
**Equation:**  
$$ K = \min\left(1, \frac{d_1\,l_X\,l_Y}{(f_L+f_R+f_T+f_B)\,\Delta t} \right) $$  
**Description:**  
- \( K \): Scaling factor  
- \( l_X, l_Y \): Cell dimensions  
- \( f_L, f_R, f_T, f_B \): Outflow fluxes in respective directions

---

### (6) Scaled Outflow Flux
**Equation:**  
$$ f_i^{t+\Delta t}(x,y) = K\,f_i^{t+\Delta t}(x,y),\quad i \in \{L, R, T, B\} $$

---

### (7) Net Volume Change in a Cell
**Equation:**  
$$ \Delta V(x,y) = \Delta t(f_R(x-1,y) + f_T(x,y-1) + f_L(x+1,y) + f_B(x,y+1) - \sum_i f_i(x,y)) $$

---

### (8) Water Surface Update
**Equation:**  
$$ d_2(x,y) = d_1(x,y) + \frac{\Delta V(x,y)}{l_X\,l_Y} $$

---

### (9) Net Water Flow in X-Direction
**Equation:**  
$$ \Delta W_X = \frac{1}{2}(f_R(x-1,y) - f_L(x,y) + f_R(x,y) - f_L(x+1,y)) $$

---

### (10) Relation to Horizontal Velocity
**Equation:**  
$$ l_Y\,\bar d\,u = \Delta W_X,\quad \bar d = \frac{d_1 + d_2}{2} $$  
**Description:**  
- \( u \): Horizontal velocity  
- \( \bar d \): Mean water depth

---

### (11) Sediment Transport Capacity
**Equation:**  
$$ C(x,y) = K_c\,\sin(\alpha(x,y))\,|\mathbf{v}(x,y)| $$  
**Description:**  
- \( C \): Sediment capacity  
- \( K_c \): Transport constant  
- \( \alpha \): Slope angle  
- \( \mathbf{v} \): Water velocity vector

---

### (12) Erosion (if \( C > s^t \))
**Equations:**  
$$ b^{t+\Delta t} = b^t - K_s(C - s^t) $$  
$$ s_1 = s^t + K_s(C - s^t) $$

---

### (13) Deposition (if \( C \le s^t \))
**Equations:**  
$$ b^{t+\Delta t} = b^t + K_d(s^t - C) $$  
$$ s_1 = s^t - K_d(s^t - C) $$

---

### (14) Sediment Advection Equation
**Equation:**  
$$ \frac{\partial s}{\partial t} + (\mathbf{v} \cdot \nabla s) = 0 $$

---

### (15) Semi-Lagrangian Sediment Transport
**Equation:**  
$$ s^{t+\Delta t}(x,y) = s_1(x - u\,\Delta t, y - v\,\Delta t) $$

---

### (16) Evaporation Step
**Equation:**  
$$ d^{t+\Delta t}(x,y) = d_2(x,y)(1 - K_e\,\Delta t) $$