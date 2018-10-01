clear 

// Naive parameterization
function [x,y,z] = unitsphere(theta, phi)
    x = cos(theta).*cos(phi);
    y = cos(theta).*sin(phi);
    z = sin(theta);
endfunction

// Cosine theta parameterization
function [x,y,z] = unitsphere2(phi, u)
    r = sqrt(1 - u.*u)
    x = r.*cos(phi);
    y = r.*sin(phi);
    z = u
endfunction

// [0,1] --> [0,2pi]
function x = azimuth(val)
    x = 2 .* %pi .*val
endfunction

// [0,1] --> [0,pi/2]
function x = elevation(val)
    x = val .* %pi .* 0.5
endfunction

// Van der Corput sequence
function x = lds(sample, base)
    x = 0;
    f = 1;
    while mtlb_any(sample > 0)
        f = f ./ base;
        x = x + f .* modulo(sample, base)
        sample = floor(sample ./ base)
    end
endfunction

// Generate LDS points
seq1 = lds([1:1:100], 2);  
seq2 = lds([1:1:100], 3);    
//scatter(seq1, seq2)

// Uniform distribution
//seq1 = rand(1, 100, "uniform")
//seq2 = rand(1, 100, "uniform")

theta = elevation(seq1)
phi = azimuth(seq2)

// Plot hemisphere
[xx,yy,zz]=eval3dp(unitsphere,linspace(0,%pi/2,40),linspace(0,%pi*2,40));
clf();
plot3d(xx,yy,zz)

// Plot naive sample points
[px1, py1, pz1] = unitsphere(theta, phi);
scatter3(1.01*px1, 1.01*py1, 1.01*pz1, 10, 'red', 'fill')

// Plot cosine sample points
[px2, py2, pz2] = unitsphere2(phi, seq1);
scatter3(1.01*px2, 1.01*py2, 1.01*pz2, 10, 'green', 'fill')
