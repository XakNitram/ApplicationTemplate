#version 460 core

/*
Translated from "LiveCoding - The Universe Within" Series.
Part 1 - https://www.youtube.com/watch?v=3CycKKJiwis
Part 2 - https://www.youtube.com/watch?v=KGJUl8Teipk
*/

in vec3 v_color;
layout(location = 0) out vec4 color;
uniform dvec2 u_Resolution;
uniform float u_Time;

double distLine(dvec2 p, dvec2 a, dvec2 b) {
    dvec2 pa = p - a;
    dvec2 ba = b - a;
    double t = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - ba * t);
}

double N21(dvec2 p) {
    p = fract(p * dvec2(233.34, 851.73));
    p += dot(p, p+23.45);
    return fract(p.x * p.y);
}

dvec2 N22(dvec2 p) {
    double n = N21(p);
    return dvec2(n, N21(p + n));
}

dvec2 getPos(dvec2 id, dvec2 offset) {
    dvec2 n = N22(id + offset) * u_Time;

    return offset + dvec2(sin(float(n))) * 0.4;
}

double line(dvec2 p, dvec2 a, dvec2 b) {
    double d = distLine(p, a, b);
    double m = smoothstep(.03, .01, d);
    double dab = length(a - b);

    m *= smoothstep(1.2, 0.8, dab) * 0.5 + smoothstep(0.05, 0.03, abs(dab - 0.75));

    return m;
}

dvec3 layer(dvec2 uv) {
    dvec2 gv = fract(uv) - 0.5;

    //if (gv.x > .48 || gv.y > .48) { 
    //    return dvec3(1, 0, 0);
    //}

    dvec2 id = floor(uv);

    dvec2 p[9];

    int i = 0;
    for (double y=-1.0; y <= 1.0; y++) {
        for (double x = -1.0; x <= 1.0; x++) {
            p[i++] = getPos(id, dvec2(x, y));
        }
    }

    double m = 0.0;
    float t = u_Time * 5.0;
    for (int i = 0; i < 9; i++) {
        m += line(gv, p[4], p[i]);

        dvec2 j = (p[i] - gv) * 20.0;
        double sparkle = 1.0 / dot(j, j);

        m += sparkle * (double(sin(t + float(fract(p[i].x) * 10.0))) * 0.5 + 0.5);
    }

    m += line(gv, p[1], p[3]);
    m += line(gv, p[1], p[5]);
    m += line(gv, p[7], p[3]);
    m += line(gv, p[7], p[5]);

    return dvec3(m);
}

void main() {
    dvec2 uv = (gl_FragCoord.xy - .5 * u_Resolution.xy) / u_Resolution.y;
    //dvec2 mouse = (u_Cursor.xy / u_Resolution.xy) - 0.5;

    double gradient = uv.y;

    dvec3 m = dvec3(0);
    float t = u_Time * 0.1;

    double s = sin(t);
    double c = cos(t);
    dmat2 rot = dmat2(c, -s, s, c);

    uv *= rot;
    //mouse *= rot;

    for (double i = 0.0; i < 1.0; i += 1.0 / 4.0) {
        double z = fract(i + t);
        double size = mix(30.0, 0.5, z);
        double fade = smoothstep(0.0, 0.5, z) * smoothstep(1.0, 0.8, z);
        //m += layer(uv * size + i * 20.0 - mouse) * fade;  // with mouse movement
        m += layer(uv * size + i * 20.0) * fade;
    }

    //dvec3 base = sin(t * dvec3(0.8, 0.4, 0.2)) * 0.4 + 0.6;
    vec3 base = v_color;
    vec3 col = float(m) * base;
    //col += 0.1 * base;

    color = vec4(vec3(col), 1.0);
}
