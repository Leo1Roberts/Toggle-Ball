#version 300 es
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection2D;

void main() {
    fragUV = inUV;
    gl_Position = uProjection2D * vec4(inPos, 0.0, 1.0);
}