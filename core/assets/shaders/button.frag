#version 300 es
precision mediump float;

in vec2 fragUV;
in vec4 fragCol;
in vec4 fragOutlineCol;
in float fragOutlineRad;

out vec4 outColor;

void main() {
    float rad = length(fragUV);
    float innerAAfrac = smoothstep(fragOutlineRad - fwidth(rad), fragOutlineRad, rad);
    vec4 tempCol = mix(fragCol, fragOutlineCol, innerAAfrac);
    float outerAAfrac = smoothstep(1.0 - fwidth(rad), 1.0, rad);
    outColor = vec4(tempCol.rgb, mix(tempCol.a, 0.0, outerAAfrac));
}