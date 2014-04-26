echo "data=dlmread('data.txt'); tri=delaunay(data(:,1), data(:,2)); dlmwrite('triangles.txt',tri-1,' '); disp(data)" | octave --silent
