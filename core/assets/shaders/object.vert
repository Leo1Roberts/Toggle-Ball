#version 300 es
in vec3 inPos;
in vec2 inUV;
in vec3 inNormal;
in vec4 inColor;

out vec2 fragUV;
out vec3 fragNormal;
out vec4 fragPos;
out vec3 fragColor;
out vec3 ambientColor;

uniform mat3 uBodyToViewRot;
uniform mat4 uBodyToView;
uniform mat4 uProjection;
uniform vec3 uUpDirection;
uniform vec3 uGroundColor;
uniform vec3 uSkyColor;

void main() {
    fragUV = inUV;
    fragNormal = uBodyToViewRot * inNormal;
    fragPos = uBodyToView * vec4(inPos, 1.0);
    fragColor = inColor.rgb;
    ambientColor = mix(uGroundColor, uSkyColor, (dot(fragNormal, uUpDirection) + 1.0) * 0.5);
    gl_Position = uProjection * fragPos;
}