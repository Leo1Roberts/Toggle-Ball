precision mediump float;

in vec2 fragUV;
in vec3 fragNormal;
in vec4 fragPos;
in vec3 fragColor;

uniform vec4 uOutlineColor;

out vec4 outColor;

void main() {
    outColor = uOutlineColor;
}