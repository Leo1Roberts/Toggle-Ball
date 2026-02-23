#version 300 es
in vec3 inPos;
in vec2 inUV;
in vec3 inNormal;
in vec4 inColor;

out vec2 fragUV;
out vec3 fragNormal;
out vec4 fragPos;
out vec3 fragColor;

uniform mat4 uProjectionFull;

void main() {
    fragUV = inUV;
    fragNormal = inNormal;
    fragPos = vec4(inPos, 1.0);
    fragColor = inColor.rgb;
    gl_Position = uProjectionFull * fragPos;
}