data = csvread('nm_opt_20100402_145637.txt');

subplot(2,1,1);

plot(data(:,1)/1000, data(:,2), '-b.');
ylabel('IR cavity detector');
xlabel('Time [s]');
ylim([1.3 1.35]);

subplot(2,1,2);

plot(data(:,1)/1000, data(:,3), data(:,1)/1000, data(:,4));
ylabel('Wavplate angle');
xlabel('Time [s]');
