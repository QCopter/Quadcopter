function [] = quadFiltAnalyze()
    clc; close all;
    % Works off the data in the D: drive
    cd 'E:\';

    % List available files, pick the file
    s = dir('*.TXT');
    file_list = {s.name};
    

    % Open the selected file
    if length(file_list) < 2
        fid = fopen(filename,'r');
    else 
        selection = menu('Select a file',file_list);
        filename = s(selection,1).name;
        fid = fopen(filename,'r');
    end
    filename = s.name;
      
    analyzefrequency = menu('Run frequency analysis?','No','Yes');

    % Find the first line of .csv format
    sline = getLine(fid);
    fclose(fid);

    % Read the csv data
    csvdata = csvread(filename,sline,0);
    time = csvdata(:,1);
    dt = getDt(time);
    altitude = csvdata(:,2);
    targetAltitude = csvdata(:,3);
    rate = csvdata(:,4);
    pidOut = csvdata(:,5);
    pidSetpoint = csvdata(:,6);

    % Plot that there data
    figure(9); hold on; grid on;
    plot(time/(1e6), altitude, 'b');
    plot(time/(1e6), targetAltitude,'r');
    plot(time/(1e6), pidSetpoint, 'g');
    plot_title = sprintf('Data, sample freq = %f',1/dt);
    title(plot_title);
    legend('altitude','target','pid setpoint','Location','SouthEast');
    hold off;
    
    if (analyzefrequency -1)
        figure(10);
        [~, ~] = timefreq(altitude,dt,1,1,0,1);
        title('Complementary filtered data');
        figure(11);
        [~, ~] = timefreq(targetAltitude,dt,1,1,0,1);
        title('Accelerometer data');
    end
    
    % Close 'em
    fclose('all');
    cd 'C:\';
end

function dt = getDt(time)

deltas = zeros(length(time),1);

for n = 2:length(time)
    deltas(n-1) = (time(n)-time(n-1))/1000000;
end

dt = sum(deltas)/length(deltas);
end

function start = getLine(fid)
    lc = 0;
    while(~feof(fid))
        lc = lc + 1;
        line = fgetl(fid);
        if ~isempty(strfind(line,'Runtime data:'))
            start = lc + 3;
            break;
        else start = 0;
        end
    end
end