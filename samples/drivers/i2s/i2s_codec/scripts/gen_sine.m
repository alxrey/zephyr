clear;

fs = 44100; % Hz
f = 441; % Hz
amplitude = 0.9;

N = 10*round(fs/f);

t = (0:N-1)/fs;
y = amplitude * sin(2*pi*f*t);

figure;
plot(y, '.-')

% int16 conversion
y_int16 = int16(y * 32767);

% Conversion en uint16 (représentation binaire identique)
y_uint16 = typecast(y_int16, 'uint16');

% Extraction LSB et MSB
lsb = bitand(y_uint16, 255);
msb = bitshift(y_uint16, -8);

% stereo duplication
y_stereo = reshape([lsb.' msb.' lsb.' msb.'].', 1, []);

% generate C header
fid = fopen('la440.h', 'w');

fprintf(fid, ...
        "#ifndef LA440_H_\n" + ...
        "#define LA440_H_\n\n" + ...
        "#if CONFIG_NOCACHE_MEMORY\n");
fprintf(fid, '#define __NOCACHE	__attribute__((__section__(".nocache")))\n');
fprintf(fid, "#elif defined(CONFIG_DT_DEFINED_NOCACHE)\n" + ...
        "#define __NOCACHE	__attribute__((__section__(CONFIG_DT_DEFINED_NOCACHE_NAME)))\n" + ...
        "#else /* CONFIG_NOCACHE_MEMORY */\n" + ...
        "#define __NOCACHE\n" + ...
        "#endif /* CONFIG_NOCACHE_MEMORY */\n\n" + ...
        "unsigned char la440[] __NOCACHE;\n");

fprintf(fid, 'unsigned char la440[] = {\n');
for k = 1:length(y_stereo)
    if mod(k, 16) == 1
        fprintf(fid, '    ');
    end
    fprintf(fid, '%d', y_stereo(k));
    if k ~= length(y_stereo)
        fprintf(fid, ', ');
    end
    if mod(k, 16) == 0
        fprintf(fid, '\n');
    end
end
fprintf(fid, '\n};\n');

fprintf(fid, "const unsigned int la440_len = sizeof(la440);\n\n");

fprintf(fid, "#endif /* SINE_H_ */\n");

fclose(fid);