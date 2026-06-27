#version 300 es
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inCol;

out vec2 fragUV;
out vec4 fragCol;

uniform mat4 uProjection2D;

void main() {
    fragUV = inUV;
    fragCol = inCol;
    gl_Position = uProjection2D * vec4(inPos, 0.0, 1.0);
}