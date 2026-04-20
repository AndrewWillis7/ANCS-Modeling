
clc;
clear;
close all;

%% SETTINGS
port = "COM4";          % Change to your board COM port
baud = 115200;
numSamples = 5000;      % Capture amount
filename = "run_test.csv"; % CHANGE THIS WHEN NEEDED

%% CONNECT
s = serialport(port, baud);
configureTerminator(s,"CR/LF");
flush(s);

disp("Connected... Receiving Data");

%% STORAGE
data = zeros(numSamples,2);
count = 0;

%% READ LOOP
while count < numSamples

    line = readline(s);          % read one UART line
    
    vals = sscanf(line,'%f,%f');
    
    if length(vals) == 2
        count = count + 1;
        data(count,1) = vals(1);   % d(n)
        data(count,2) = vals(2);   % e(n)
    end
end

%% CLOSE PORT
clear s

%% SAVE CSV
writematrix(data, filename);

disp("Capture Complete.");
disp("Saved File: " + filename);

%% QUICK VALIDATION PLOT
figure;
plot(data(:,1));
hold on;
plot(data(:,2));
grid on;
legend('d(n)','e(n)');
title('ANC Validation Capture');
xlabel('Sample');
ylabel('Amplitude');