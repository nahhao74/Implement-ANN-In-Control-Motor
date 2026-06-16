%% ===== Nhận dạng G(s)_vel (vòng hở) từ dữ liệu PWM/velocity + Plot =====
clear; clc; close all;

%% --- Đọc dữ liệu motor ---
dataFolder = 'C:\Users\ADMIN\Documents\NCKH_RAI_Lab\Data_motor';
fileName = 'motor_data_009.csv';
dataPath = fullfile(dataFolder, fileName);

data = readtable(dataPath);
t_raw = data.time;          % thời gian (s)
y_raw = data.vel;           % vận tốc đo được (rad/s)
ggt = (0:length(y_filt)-1)'*Ts;

%% --- Smooth output ---
y_filt = smoothdata(y_filt,'movmean',5);

%% --- Chuẩn bị dữ liệu nhận dạng ---
data_id_filt = iddata(y_filt, u, Ts);

%% --- Tự chọn bậc tốt nhất (np = 1~4, nz = 0~np-1) ---
bestFit = -inf;
bestModel = [];
for np = 1:4
    for nz = 0:np-1
        try
            sys_try = tfest(data_id_filt, np, nz);
            report = sys_try.Report;
            if report.Fit.FitPercent > bestFit
                bestFit = report.Fit.FitPercent;
                bestModel = sys_try;
            end
        catch
            continue;
        end
    end
end

G_vel = bestModel; % G(s) vòng hở
fprintf('Mô hình G(s)_vel tốt nhất (Fit = %.2f%%):\n', bestFit);
G_vel

%% --- Mô phỏng đáp ứng với tín hiệu PWM thực ---
y_sim = lsim(G_vel, u, t);

%% --- Tính Fit% chuẩn ---
fit_val = 100 * (1 - norm(y_filt - y_sim)/norm(y_filt - mean(y_filt)));
fprintf('Fit%% giữa mô phỏng và dữ liệu thực: %.2f%%\n', fit_val);





%% --- Cấu hình tối ưu hóa ---
% Tối ưu 5 biến: [Kp_pos, Ki_pos, Kd_pos, Kp_vel, Ki_vel]
nVars = 5;
% Bounds (có thể điều chỉnh theo hệ của bạn)
lb = [0,    0,    0,   0,    0];   % min gains
ub = [20, 20,  10,  10,  10];      % max gains

%% ===== Khởi tạo parallel pool =====
if isempty(gcp('nocreate'))
    parpool('local', min(6, feature('numcores')));
end

%% --- Giảm hệ liên tục xuống dạng rời rạc để mô phỏng timestepping (sử dụng Ts) ---
Gd = c2d(G_vel, Ts, 'tustin');
[bp,ap] = tfdata(Gd,'v');  %  <== ap, bp defined HERE

%% --- Tham số mô phỏng ---
Tsim = 6;
Nsim = round(Tsim/Ts)+1;
time = (0:Nsim-1)'*Ts;
pos_setpoint = 4000;
PWM_max = 255;
PWM_min = -255;

%% GA options
opts = optimoptions('ga', ...
    'Display','iter', ...
    'MaxGenerations',80, ...
    'PopulationSize',80, ...
    'UseParallel', true, ...
    'PlotFcn',{@gaplotbestf,@gaplotscores});

%% ===== Chạy GA nhiều lần và lưu kết quả =====
num_runs = 20;            % số lần chạy GA
overshoot_limit = 0.7;    % % overshoot tối đa
results = [];             % lưu tất cả kết quả thỏa điều kiện

for run = 1:num_runs
    fprintf('\n--- GA Run %d / %d ---\n', run, num_runs);

    rng('shuffle'); % random seed
    fitness = @(x) objective_fun(x, ap, bp, Ts, Nsim, pos_setpoint, PWM_max, PWM_min, Tsim);
    [bestX,bestF,exitflag,output] = ga(fitness, nVars, [],[],[],[], lb, ub, [], opts);


    % --- Simulate với gains này để tính overshoot ---
    x = bestX;
    Kpp = x(1); Kip = x(2); Kdp = x(3);
    Kpv = x(4); Kiv = x(5);

    % reset states
    int_pos = 0; prev_pos_err = 0; int_vel = 0;
    na = length(ap); nb = length(bp);
    u_hist = zeros(nb,1); y_hist = zeros(na,1);
    pos = 0; vel = 0;
    pos_history = zeros(Nsim,1); vel_history = zeros(Nsim,1); u_history = zeros(Nsim,1);

    for k = 1:Nsim
        pos_err = pos_setpoint - pos;
        int_pos = int_pos + pos_err*Ts;
        d_pos = (pos_err - prev_pos_err)/Ts; prev_pos_err = pos_err;
        v_sp = Kpp*pos_err + Kip*int_pos + Kdp*d_pos;
        v_sp = max(min(v_sp,5000),-5000);

        vel_err = v_sp - vel;
        int_vel = int_vel + vel_err*Ts;
        u_unsat = Kpv*vel_err + Kiv*int_vel;
        u_sat = max(min(u_unsat, PWM_max), PWM_min);
        if u_unsat ~= u_sat
            int_vel = int_vel - (u_unsat - u_sat)/max(Kiv,1e-6);
        end

        u_hist = [u_sat; u_hist(1:end-1)];
        y_new = (bp * u_hist - (ap(2:end) * y_hist(1:end-1))) / ap(1);
        y_hist = [y_new; y_hist(1:end-1)];

        vel = y_new;
        pos = pos + vel*Ts;

        pos_history(k) = pos; vel_history(k) = vel; u_history(k) = u_sat;
    end

    % tính overshoot
    peak = max(pos_history);
    overshoot = (peak - pos_setpoint)/max(abs(pos_setpoint),1e-6)*100;

    if overshoot <= overshoot_limit
        fprintf('Overshoot %.3f %% < %.3f %% -> lưu bộ gain\n', overshoot, overshoot_limit);
        results = [results; x, overshoot];
    else
        fprintf('Overshoot %.3f %% > %.3f %% -> bỏ qua\n', overshoot, overshoot_limit);
    end
end

%% --- Lấy bộ gain tốt nhất (overshoot thấp nhất) ---
if isempty(results)
    warning('Không tìm thấy bộ gain nào thỏa điều kiện overshoot < %.2f %%', overshoot_limit);
else
    [~, idx_best] = min(results(:,6)); 
    best_gain = results(idx_best,1:5);
    overshoot_best = results(idx_best,6);

    fprintf('\n=== Bộ gain tốt nhất (overshoot %.3f %%) ===\n', overshoot_best);
    fprintf('Outer PID: Kp=%.4f, Ki=%.4f, Kd=%.4f\n', best_gain(1), best_gain(2), best_gain(3));
    fprintf('Inner PI:  Kp=%.4f, Ki=%.4f\n', best_gain(4), best_gain(5));

    % plot kết quả với bộ gain này
    % --- reset states ---
    x = best_gain;
    Kpp = x(1); Kip = x(2); Kdp = x(3);
    Kpv = x(4); Kiv = x(5);
    int_pos = 0; prev_pos_err = 0; int_vel = 0;
    u_hist = zeros(nb,1); y_hist = zeros(na,1);
    pos = 0; vel = 0;
    pos_history = zeros(Nsim,1); vel_history = zeros(Nsim,1); u_history = zeros(Nsim,1);
    for k = 1:Nsim
        pos_err = pos_setpoint - pos;
        int_pos = int_pos + pos_err*Ts;
        d_pos = (pos_err - prev_pos_err)/Ts; prev_pos_err = pos_err;
        v_sp = Kpp*pos_err + Kip*int_pos + Kdp*d_pos;
        v_sp = max(min(v_sp,5000),-5000);

        vel_err = v_sp - vel;
        int_vel = int_vel + vel_err*Ts;
        u_unsat = Kpv*vel_err + Kiv*int_vel;
        u_sat = max(min(u_unsat, PWM_max), PWM_min);
        if u_unsat ~= u_sat
            int_vel = int_vel - (u_unsat - u_sat)/max(Kiv,1e-6);
        end

        u_hist = [u_sat; u_hist(1:end-1)];
        y_new = (bp * u_hist - (ap(2:end) * y_hist(1:end-1))) / ap(1);
        y_hist = [y_new; y_hist(1:end-1)];

        vel = y_new;
        pos = pos + vel*Ts;

        pos_history(k) = pos; vel_history(k) = vel; u_history(k) = u_sat;
    end



   %% results: mỗi hàng [Kpp, Kip, Kdp, Kpv, Kiv, overshoot]
    T = array2table(results, ...
        'VariableNames', {'Kp_pos','Ki_pos','Kd_pos','Kp_vel','Ki_vel','Overshoot'});

    % đường dẫn lưu
    saveFolder = 'C:\Users\ADMIN\Documents\NCKH_RAI_Lab\Data_Gain';
    if ~exist(saveFolder, 'dir')
        mkdir(saveFolder);
    end
    csv_name = fullfile(saveFolder, sprintf('gain_overshoot_%d.csv', pos_setpoint));

    writetable(T, csv_name);
    fprintf('Tất cả bộ gain thỏa overshoot đã lưu thành file: %s\n', csv_name);


end



function J = objective_fun(x, ap, bp, Ts, Nsim, pos_setpoint, PWM_max, PWM_min, Tsim)

    % x = [Kpp, Kip, Kdp, Kpv, Kiv]
    Kpp = x(1); Kip = x(2); Kdp = x(3);
    Kpv = x(4); Kiv = x(5);

    na = length(ap);
    nb = length(bp);

    u_hist = zeros(nb,1);
    y_hist = zeros(na,1);

    % states
    int_pos = 0; prev_pos_err = 0; int_vel = 0;
    pos = 0; vel = 0;

    pos_history = zeros(Nsim,1);
    u_history = zeros(Nsim,1);

    for k = 1:Nsim

        % ===== OUTER PID =====
        pos_err = pos_setpoint - pos;
        int_pos = int_pos + pos_err*Ts;
        d_pos = (pos_err - prev_pos_err)/Ts;
        prev_pos_err = pos_err;

        v_sp = Kpp*pos_err + Kip*int_pos + Kdp*d_pos;
        v_sp = max(min(v_sp, 5000), -5000);

        % ===== INNER PI =====
        vel_err = v_sp - vel;
        int_vel = int_vel + vel_err*Ts;

        u_unsat = Kpv*vel_err + Kiv*int_vel;

        % saturation + anti-windup
        u_sat = max(min(u_unsat, PWM_max), PWM_min);
        if u_unsat ~= u_sat
            int_vel = int_vel - (u_unsat - u_sat)/max(Kiv,1e-6);
        end

        % ==== Discrete plant ====
        u_hist = [u_sat; u_hist(1:end-1)];
        y_new = (bp * u_hist - ap(2:end)*y_hist(1:end-1)) / ap(1);
        y_hist = [y_new; y_hist(1:end-1)];

        vel = y_new;
        pos = pos + vel*Ts;

        pos_history(k) = pos;
        u_history(k) = u_sat;
    end

    % ===== Evaluate =====
    peak = max(pos_history);
    overshoot = (peak - pos_setpoint)/max(abs(pos_setpoint),1e-6);

    tol = 0.02*abs(pos_setpoint);
    idx = find(abs(pos_history - pos_setpoint) <= tol, 1, 'first');
    if isempty(idx)
        settling_time = Tsim + 10;
    else
        settling_time = (idx-1)*Ts;
    end

    J = settling_time + ...
        1e4 * max(overshoot - 0.07, 0) + ...
        mean(u_history.^2) * 1e-4;

    if isnan(J) || ~isreal(J)
        J = 1e8;
    end
end
