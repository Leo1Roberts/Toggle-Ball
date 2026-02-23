#version 300 es
in vec2 inPos;
in vec2 inUV;
in vec4 inCol;

out vec2 fragUV;
out vec4 fragCol;

uniform mat4 uProjection2D;

void main() {
    fragUV = inUV;
    fragCol = inCol;
    gl_Position = uProjection2D * vec4(inPos, 0.0, 1.0);
}