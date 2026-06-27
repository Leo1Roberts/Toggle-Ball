#version 300 es
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inFillCol;
layout(location = 3) in vec4 inOutlineCol;
layout(location = 4) in float inOutlineRad;

out vec2 fragUV;
out vec4 fragFillCol;
out vec4 fragOutlineCol;
out float fragOutlineRad;

uniform mat4 uProjection2D;

void main() {
    fragUV = inUV;
    fragFillCol = inFillCol;
    fragOutlineCol = inOutlineCol;
    fragOutlineRad = inOutlineRad;
    gl_Position = uProjection2D * vec4(inPos, 0.0, 1.0);
}