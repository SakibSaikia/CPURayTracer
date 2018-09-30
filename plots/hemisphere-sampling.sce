clear 

function [x,y,z] = unitsphere(theta, phi)
    x = cos(theta).*cos(phi);
    y = cos(theta).*sin(phi);
    z = sin(theta);
endfunction

function [x,y,z] = unitsphere2(phi, u)
    r = sqrt(1 - u.*u)
    x = r.*cos(phi);
    y = r.*sin(phi);
    z = u
endfunction

function x = azimuth(val)
    x = 2 .* %pi .*val
endfunction

function x = elevation(val)
    x = val .* %pi .* 0.5
endfunction

function x = lds(sample, base)
    x = 0;
    f = 1;
    while mtlb_any(sample > 0)
        f = f ./ base;
        x = x + f .* modulo(sample, base)
        sample = floor(sample ./ base)
    end
endfunction

seq1 = lds([1:1:100], 2);  
seq2 = lds([1:1:100], 3);    
//scatter(seq1, seq2)

theta = elevation(seq1)
phi = azimuth(seq2)

[xx,yy,zz]=eval3dp(unitsphere,linspace(0,%pi/2,40),linspace(0,%pi*2,40));
clf();
plot3d(xx,yy,zz)
[px1, py1, pz1] = unitsphere(theta, phi);
//scatter3(1.01*px1, 1.01*py1, 1.01*pz1, 10, 'scilab red2', 'fill')

[px2, py2, pz2] = unitsphere2(phi, seq1);
scatter3(1.01*px2, 1.01*py2, 1.01*pz2, 10, 'scilab red2', 'fill')
