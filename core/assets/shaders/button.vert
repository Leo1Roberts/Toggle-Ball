#version 300 es
in vec2 inPos;
in vec2 inUV;
in vec4 inCol;
in vec4 inOutlineCol;
in float inOutlineRad;

out vec2 fragUV;
out vec4 fragCol;
out vec4 fragOutlineCol;
out float fragOutlineRad;

uniform mat4 uProjection2D;

void main() {
    fragUV = inUV;
    fragCol = inCol;
    fragOutlineCol = inOutlineCol;
    fragOutlineRad = inOutlineRad;
    gl_Position = uProjection2D * vec4(inPos, 0.0, 1.0);
}