// ==================== CHÂN KẾT NỐI ====================
const uint8_t R_PWM = 26;
const uint8_t L_PWM = 25;
const uint8_t phase_a = 18;
const uint8_t phase_b = 19;

// ==================== BIẾN TOÀN CỤC ====================
volatile long pulse = 0;
volatile float setpoint = 0;

// PID vị trí (vòng ngoài) - GIÁ TRỊ NÀY SẼ BỊ GHI ĐÈ BỞI ANN
volatile float kp_pos = 1, ki_pos = 1, kd_pos = 1;

// PID vận tốc (vòng trong) - GIÁ TRỊ NÀY SẼ BỊ GHI ĐÈ BỞI ANN
volatile float kp_vel = 1, ki_vel = 1, kd_vel = 0; // kd_vel không được ANN tính, giữ nguyên 0

// Biến trung gian
volatile float pos_now = 0, pos_prev = 0;
volatile float velocityReq = 0, current_vel = 0;
volatile float output = 0, pwm = 0;
volatile float error_pos = 0, error_pos_prev = 0;
volatile float error_vel = 0, error_vel_prev = 0;
volatile float integral_pos = 0, integral_vel = 0;
volatile float pos_vel_cal = 0, delta_error_vel = 0;
volatile float delta_error_pos = 0;
volatile float Delta = 0.01; // Ts = 10ms mặc định
unsigned long last_time = 0;
String inputString = "";
bool newData = false;

// ==================== NGẮT ENCODER ====================
void IRAM_ATTR encoderISR() {
  if (digitalRead(phase_a) == digitalRead(phase_b))
    pulse--;
  else
    pulse++;
}

float VelCalculate(float Pos_now, float delta) {
  float Current_vel = (Pos_now / 200) / delta * 60;
  return Current_vel;
}



// ==================== NORMALIZATION & ANN ====================
const int n_inputs = 4;
// CÁC GIÁ TRỊ CHUẨN HÓA MỚI ĐÃ CẬP NHẬT
float x_mean[n_inputs] = {57.86237565, -19.95368601, -3.49094143, -2.98063114};
float x_std[n_inputs]  = {11214.51364636, 729.25088412, 305.74467692, 267.05022006};

const float NORM_EPS = 1e-8;

void normalize(float input[4], float output[4]) {
  for(int i=0;i<4;i++){
    output[i] = (input[i] - x_mean[i]) / (x_std[i] + NORM_EPS);
  }
}

// --- Layer 0 ---
const int L0_IN = 4;
const int L0_OUT = 8;
float W0[L0_IN][L0_OUT] = {
    { 0.03182799, 0.01378257, 0.00715465, -1.09894957, -0.10325591, 1.24757084, 0.00590211, 0.1686597 },
    { 0.24790379, 0.092559, 0.12168752, 0.10881395, 0.91771539, -0.07279743, -0.10457972, -0.25497047},
    {-0.65364869, -0.93211083, 1.16029773, -0.3696174, -1.09545171, -0.03792048, -1.41449909, 0.65886544},
    {-0.45458242, 0.37232029, -0.55970326, -0.07290906, -0.02459998, 0.22995356, 1.91573771, 0.43143413}
};
float layer0_bias[L0_OUT] = {
    0.03824704, 0.0354973, 0.02438357, -1.0198066, -0.01444324, -1.07167886, 0.04431007, -0.00506205
};

// --- Layer 3 ---
const int L3_IN = 8;
const int L3_OUT = 16;
float W3[L3_IN][L3_OUT] = {
    {-7.78635219e-02, 6.68201856e-02, 2.17776541e-01, 3.21136049e-02, -9.48780943e-02, -4.45454624e-01, -4.34199951e-01, 2.07637550e-01, 8.21334671e-02, 2.40299043e-05, -5.31410089e-01, -9.50018458e-02, 2.57961167e-02, -1.50406225e-01, -9.56299838e-02, 8.75508088e-02},
    {-1.44153208e-02, 1.18487616e-01, -3.13023295e-02, -1.32534722e-01, 3.08536631e-02, -3.72650504e-01, -6.08266453e-01, -2.18067885e-02, -1.49572121e-01, -4.19361105e-05, -4.35780367e-01, -1.00585884e-01, 1.82950698e-01, -1.80249321e-01, 4.13919456e-02, 8.70929307e-02},
    { 8.78927804e-02, -9.28456269e-03, 6.00744588e-03, 8.27059208e-03, -4.12857967e-02, -1.59829020e-01, -9.06046495e-01, -1.29245276e-03, 3.14446051e-02, 4.62557074e-05, -8.35405112e-01, 3.57875928e-02, 1.93993872e-02, 8.13783456e-02, -4.70184743e-02, -9.20404821e-03},
    { 7.72702907e-01, -1.95087201e-01, -9.36385763e-02, -5.24457309e-01, 1.99342443e-01, 7.03878105e-01, 3.39926589e-02, -1.08722169e-01, -3.16641032e-01, 6.22960836e-05, 2.27032504e-02, -1.11461083e-01, -1.85895041e-01, -2.39482808e-01, -2.06616751e-01, -1.88103628e-01},
    {-1.38761021e-01, 1.15984864e-02, -1.18947958e-01, 2.19687000e-02, -2.44859506e+00, 4.87021901e-02, 9.32987805e-02, -8.10009699e-02, 1.40629499e-02, 1.98261933e-05, 1.09424312e-01, -9.35423762e-02, -8.55588814e-03, 2.03362043e-01, 2.54915575e-01, 4.51075509e-02},
    { 6.49777718e-01, -2.21459788e-01, -1.99417511e-01, -2.97569148e-01, 1.86459062e-01, 6.10719738e-01, 9.05406585e-02, -1.59043922e-01, -2.48857472e-04, -5.88296631e-05, 7.75116127e-02, -3.62717020e-01, -2.61225694e-01, -2.67914920e-02, -2.33112454e-01, -2.14013810e-01},
    {-2.96249966e-02, 1.37379678e-02, 4.98719374e-02, 5.29477807e-02, -2.05725149e-02, -1.12534592e-01, -1.89560940e-01, 4.67641516e-02, -2.21231834e+00, 4.24152041e-06, -2.54807785e-01, 9.46341281e-02, -8.20178415e-03, 2.73699976e-02, -1.18627925e-01, 1.96511125e-02},
    {-4.16466705e-02, 1.19371002e-01, 4.31063618e-02, -5.10829830e-03, -8.15228539e-01, -9.91664414e-03, -1.30649355e-02, 7.05010830e-02, -8.14667727e-02, -4.32374598e-05, -8.02280204e-03, -5.10291522e-02, 6.92789993e-02, -5.73755335e-02, 1.89235533e-01, 7.35231217e-02}
};
float layer3_bias[L3_OUT] = {
    -1.79257373, 0.07263313, 0.11718118, 0.02287642, 0.16025795, -1.20119945, 0.22086069, 0.05388956,
    0.11658871, -0.09233825, 0.22705227, 0.03230952, 0.18559679, 0.15248552, 0.02428808, 0.13517814
};

// --- Layer 6 ---
const int L6_IN = 16;
const int L6_OUT = 5;
float W6[L6_IN][L6_OUT] = {
    { 5.18317369e+00, 5.01138267e-03, 4.99263889e-02, 4.47826444e-02, -2.13574541e+00},
    { 8.98719450e-01, 1.14448638e-03, 2.99126628e-01, 3.80084668e-03, -3.56185859e-01},
    { 6.56357597e-01, 1.07254829e-05, 5.04908616e-01, -1.87961040e-02, -4.85114157e-01},
    { 3.25157004e+00, 2.34079453e-03, 1.00130173e+00, -9.09131893e-04, -7.94645421e-01},
    {-3.14584833e+00, -2.83357560e-03, -4.50361281e-01, -1.62817468e-02, 2.50653294e+00},
    { 3.72664575e+00, 4.43977187e-03, -2.31151944e-01, 6.65028334e-02, 5.92022589e-01},
    { 4.87820854e+00, 2.42985968e-03, 4.02443616e-02, 3.22289608e-02, -1.59938752e+00},
    { 9.84125581e-01, 1.14496706e-03, 5.91107124e-01, -4.90928283e-04, -2.40062844e-01},
    { 5.09357783e+00, 7.58679160e-03, 1.52346979e+00, 1.88465465e-02, -1.43757585e+00},
    { 2.51335158e-02, -9.80846429e-04, 1.19523806e-02, 2.59944579e-02, -5.12026791e-02},
    { 4.61450476e+00, 2.03621105e-03, 1.85671192e-01, 2.49009687e-02, -2.08682648e+00},
    { 2.39673145e+00, 4.25223728e-03, 1.15476789e+00, 2.72276074e-03, -4.46349972e-01},
    { 5.66365893e-01, -8.50551614e-04, 2.17798190e-01, -1.56143063e-02, -5.03992233e-01},
    { 2.94293062e-01, -1.97485148e-03, 1.44167643e-01, -1.70123298e-02, -7.78610440e-01},
    { 1.42373599e+00, 3.22186894e-03, 6.31679292e-01, 2.12651849e-03, -2.84817647e-01},
    { 6.73829628e-01, -5.21304831e-05, 3.41124194e-01, -6.61806814e-03, -4.27870096e-01}
};
float layer6_bias[L6_OUT] = {
    1.30175214e+01, 1.28455415e-03, 3.52528198e-01, 4.38829940e-02, 4.26790113e+00
};

// --- Activation ---
float relu(float x){ return (x>0)?x:0; }
float linear(float x){ return x; }

// ==================== Forward Pass ====================
void predict(float input[4], float output[5]){
  float x_norm[4];
  normalize(input, x_norm);

  // Tính toán Layer 0
  float L0_out[L0_OUT];
  for(int i=0;i<L0_OUT;i++){ // Lặp qua OUTPUT (8 nơ-ron)
    L0_out[i] = layer0_bias[i];
    for(int j=0;j<L0_IN;j++){ // Lặp qua INPUT (4 nơ-ron)
      L0_out[i] += W0[j][i] * x_norm[j]; 
    }
    L0_out[i] = relu(L0_out[i]);
  }

  // Tính toán Layer 3
  float L3_out[L3_OUT];
  for(int i=0;i<L3_OUT;i++){ // Lặp qua OUTPUT (16 nơ-ron)
    L3_out[i] = layer3_bias[i];
    for(int j=0;j<L3_IN;j++){ // Lặp qua INPUT (8 nơ-ron)
      L3_out[i] += W3[j][i] * L0_out[j]; 
    }
    L3_out[i] = relu(L3_out[i]);
  }

  // Tính toán Layer 6 (Output)
  for(int i=0;i<L6_OUT;i++){ // Lặp qua OUTPUT (5 nơ-ron)
    output[i] = layer6_bias[i];
    for(int j=0;j<L6_IN;j++){ // Lặp qua INPUT (16 nơ-ron)
      output[i] += W6[j][i] * L3_out[j];
    }
    output[i] = linear(output[i]);
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(phase_a, INPUT_PULLUP);
  pinMode(phase_b, INPUT_PULLUP);
  attachInterrupt(phase_a, encoderISR, CHANGE);

  ledcAttach(R_PWM, 20000, 8);
  ledcAttach(L_PWM, 20000, 8);
  ledcWrite(R_PWM, 0);
  ledcWrite(L_PWM, 0);

  Serial.println("BOOT_OK");
  Serial.println("time,error_pos,delta_error_pos,error_vel,delta_error_vel,kp_pos,ki_pos,kd_pos,kp_vel,ki_vel");
  last_time = millis();
}

// ==================== MAIN LOOP ====================
void loop() {
  unsigned long now = millis();

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {  
      newData = true;
    } else {
      inputString += c;  
    }
  }

  if (newData) { 
    setpoint = inputString.toDouble();
    inputString = "";
    newData = false;
  }
  

  // --- PID LOOP ---
  if ((now - last_time) >= (Delta * 1000)) {
    noInterrupts();
    pos_now = pulse;
    pos_vel_cal = pos_now - pos_prev;
    interrupts();
    float t = now / 1000.0;

    // --- BƯỚC 1: TÍNH TOÁN LỖI VÀ ĐẦU VÀO ANN ---
    error_pos = setpoint - pos_now;
    delta_error_pos = error_pos - error_pos_prev;
    
    
    // kd_vel vẫn giữ giá trị 0.0 (hoặc giá trị được đặt thủ công)

    // --- BƯỚC 3: VÒNG PID VỊ TRÍ (Vòng ngoài) ---
    integral_pos += error_pos * Delta;
    integral_pos = constrain(integral_pos, -1000, 1000);
    float derivative_pos = (error_pos - error_pos_prev) / Delta;
    // Tính velocityReq mới (setpoint cho vòng vận tốc)
    velocityReq = kp_pos * error_pos + ki_pos * integral_pos + kd_pos * derivative_pos;
    velocityReq = constrain(velocityReq, -3000, 3000);
    
    // --- BƯỚC 4: VÒNG PID VẬN TỐC (Vòng trong) ---
    // error_vel đã được tính ở BƯỚC 1. Tiếp tục tính toán vòng vận tốc.
    
    current_vel = VelCalculate(pos_vel_cal, Delta); 
    // error_vel ở đây dùng velocityReq từ bước lặp trước (Đúng cho ANN tuning)
    error_vel = velocityReq - current_vel;
    delta_error_vel = error_vel - error_vel_prev;
    pos_vel_cal = 0; // Đặt lại sau khi tính toán
    

    integral_vel += error_vel * Delta;
    integral_vel = constrain(integral_vel, -1000, 1000);
    float derivative_vel = (error_vel - error_vel_prev) / Delta;

    // Tính toán đầu ra PWM
    output = kp_vel * error_vel + ki_vel * integral_vel + kd_vel * derivative_vel;
    output = constrain(output, -255, 255);
    pwm = output;

    // --- BƯỚC 2: DỰ ĐOÁN HỆ SỐ PID BẰNG ANN ---
    float ann_input[4] = {error_pos, delta_error_pos, error_vel, delta_error_vel};
    float pid_out[5];
    predict(ann_input, pid_out);

     // Cập nhật hệ số PID từ ANN
    kp_pos = pid_out[0]; ki_pos = pid_out[1]; kd_pos = pid_out[2];
    kp_vel = pid_out[3]; ki_vel = pid_out[4];

    // --- BƯỚC 5: ĐIỀU KHIỂN ĐỘNG CƠ VÀ CẬP NHẬT BIẾN ---
    if (pwm > 0) {
      ledcWrite(R_PWM, pwm);
      ledcWrite(L_PWM, 0);
    } else {
      ledcWrite(R_PWM, 0);
      ledcWrite(L_PWM, -pwm);
    }

    // --- BƯỚC 6: CẬP NHẬT GIÁ TRỊ LẶP ---
    pos_prev = pos_now;
    error_pos_prev = error_pos;
    error_vel_prev = error_vel;
    last_time = now;

    // --- GHI DỮ LIỆU ---
    Serial.print(t, 3); Serial.print(",");
    Serial.print(pos_now); Serial.print(",");
    Serial.print(error_pos); Serial.print(",");
    Serial.print(delta_error_pos); Serial.print(",");
    Serial.print(error_vel); Serial.print(",");
    Serial.print(delta_error_vel); Serial.print(",");
    Serial.print(kp_pos, 3); Serial.print(",");
    Serial.print(ki_pos, 3); Serial.print(",");
    Serial.print(kd_pos, 3); Serial.print(",");
    Serial.print(kp_vel, 3); Serial.print(",");
    Serial.println(ki_vel, 3);
  }
}