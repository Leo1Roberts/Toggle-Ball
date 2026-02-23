#version 300 es
precision mediump float;

in vec2 fragUV;
in vec3 fragNormal;
in vec4 fragPos;
in vec3 fragColor;

uniform vec4 uOutlineColor;

out vec4 outColor;

void main() {
    outColor = vec4(uOutlineColor.rgb, uOutlineColor.a + (fragUV.x + fragNormal.x + fragPos.x + fragColor.x) * 0.000001);
}