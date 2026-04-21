clc;
clear;
close all;

%% ========================= SETTINGS =========================
port = "COM9";                  % Change to your board COM port
baud = 115200;
numSamples = 5000;              % Number of UART samples to capture
filename = "run_test.csv";      % Output CSV file
fs = 8000;                      % DSP sample rate
W = 100;                        % Learning-curve window length
eps_floor = 1e-12;              % Small floor for SNR calculations

%% ========================= CONNECT =========================
s = serialport(port, baud);
configureTerminator(s, "CR/LF");
flush(s);

disp("Connected... Receiving Data");

%% ========================= STORAGE =========================
data = zeros(numSamples, 3);    % columns = [d, x, e]
count = 0;

%% ========================= READ LOOP =========================
while count < numSamples
    line = readline(s);
    vals = sscanf(line, '%f,%f,%f');

    if length(vals) == 3
        count = count + 1;
        data(count,1) = vals(1);   % d(n) = primary input
        data(count,2) = vals(2);   % x(n) = reference input
        data(count,3) = vals(3);   % e(n) = ANC output
    end
end

%% ========================= CLOSE PORT =========================
clear s

%% ========================= SAVE CSV =========================
writematrix(data, filename);

disp("Capture Complete.");
disp("Saved File: " + filename);

%% ========================= EXTRACT SIGNALS =========================
d = data(:,1);
x = data(:,2);
e = data(:,3);

N = length(d);
n = (0:N-1).';

%% ========================= RAW WAVEFORM PLOT =========================
figure;
plot(d, 'DisplayName', 'd(n) - primary');
hold on;
plot(x, 'DisplayName', 'x(n) - reference');
plot(e, 'DisplayName', 'e(n) - ANC output');
grid on;
legend;
title('Captured ANC Signals');
xlabel('Sample Index');
ylabel('Amplitude');

%% ========================= ZOOMED WAVEFORM PLOT =========================
idx1 = 1000;
idx2 = min(1400, N);

figure;
plot(idx1:idx2, d(idx1:idx2), 'DisplayName', 'd(n)');
hold on;
plot(idx1:idx2, x(idx1:idx2), 'DisplayName', 'x(n)');
plot(idx1:idx2, e(idx1:idx2), 'DisplayName', 'e(n)');
grid on;
legend;
title('Zoomed Time-Domain Signals');
xlabel('Sample Index');
ylabel('Amplitude');

%% ========================= LEARNING CURVE =========================
% Project-style practical learning curve using residual output power
mse_curve = movmean(e.^2, W);

figure;
plot(mse_curve, 'LineWidth', 1.2);
grid on;
title('Learning Curve: Windowed MSE of e(n)');
xlabel('Sample Index');
ylabel('MSE');

%% ========================= FFTS =========================
D = fft(d);
X = fft(x);
E = fft(e);

halfN = floor(N/2) + 1;
f = (0:halfN-1) * fs / N;

D_half = D(1:halfN);
X_half = X(1:halfN);
E_half = E(1:halfN);

Dmag_dB = 20*log10(abs(D_half) + eps_floor);
Xmag_dB = 20*log10(abs(X_half) + eps_floor);
Emag_dB = 20*log10(abs(E_half) + eps_floor);

%% ========================= FFT COMPARISON =========================
figure;
plot(f, Dmag_dB, 'LineWidth', 1.2, 'DisplayName', 'd(n)');
hold on;
plot(f, Emag_dB, 'LineWidth', 1.2, 'DisplayName', 'e(n)');
grid on;
legend;
title('FFT Comparison: d(n) vs e(n)');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

%% ========================= OPTIONAL REFERENCE FFT =========================
figure;
plot(f, Xmag_dB, 'LineWidth', 1.2, 'DisplayName', 'x(n)');
grid on;
legend;
title('FFT of Reference Signal x(n)');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

%% ========================= PROJECT-STYLE SNR ESTIMATE =========================
% From project handout logic:
%   d(n) = s(n) + v(n)
%   x(n) = noise reference
%   e(n) = ANC output
%
% Approximate:
%   P_noise(k)     = |X(k)|^2
%   P_signal(k)    = max(|D(k)|^2 - |X(k)|^2, eps)
%   P_noise_out(k) = max(|E(k)|^2 - P_signal(k), eps)

P_D = abs(D_half).^2;
P_X = abs(X_half).^2;
P_E = abs(E_half).^2;

P_signal = max(P_D - P_X, eps_floor);
P_noise_in = max(P_X, eps_floor);
P_noise_out = max(P_E - P_signal, eps_floor);

snr_in = 10 * log10(sum(P_signal) / sum(P_noise_in));
snr_out = 10 * log10(sum(P_signal) / sum(P_noise_out));
snr_improvement = snr_out - snr_in;

disp(" ");
disp("===== ANC SNR RESULTS =====");
disp("Estimated Input SNR       = " + num2str(snr_in, '%.3f') + " dB");
disp("Estimated Output SNR      = " + num2str(snr_out, '%.3f') + " dB");
disp("Estimated SNR Improvement = " + num2str(snr_improvement, '%.3f') + " dB");

%% ========================= BAR CHART FOR SNR =========================
figure;
bar([snr_in, snr_out, snr_improvement]);
grid on;
set(gca, 'XTickLabel', {'Input SNR', 'Output SNR', 'Improvement'});
ylabel('dB');
title('Estimated ANC SNR Metrics');

%% ========================= OPTIONAL MOVING RMS =========================
drms = sqrt(movmean(d.^2, W));
xrms = sqrt(movmean(x.^2, W));
erms = sqrt(movmean(e.^2, W));

figure;
plot(drms, 'DisplayName', 'RMS d(n)');
hold on;
plot(xrms, 'DisplayName', 'RMS x(n)');
plot(erms, 'DisplayName', 'RMS e(n)');
grid on;
legend;
title('Moving RMS of Captured Signals');
xlabel('Sample Index');
ylabel('RMS Amplitude');

%% ========================= SUMMARY TABLE OUTPUT =========================
fprintf('\n===== SUMMARY =====\n');
fprintf('Samples Captured       : %d\n', N);
fprintf('Sample Rate            : %.0f Hz\n', fs);
fprintf('Learning Window        : %d samples\n', W);
fprintf('Estimated Input SNR    : %.3f dB\n', snr_in);
fprintf('Estimated Output SNR   : %.3f dB\n', snr_out);
fprintf('Estimated Improvement  : %.3f dB\n', snr_improvement);
fprintf('Saved CSV              : %s\n', filename);