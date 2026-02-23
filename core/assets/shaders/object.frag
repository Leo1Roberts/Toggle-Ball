#version 300 es
precision mediump float;

in vec2 fragUV;
in vec3 fragNormal;
in vec4 fragPos;
in vec3 fragColor;
in vec3 ambientColor;

uniform sampler2D uTexture;
uniform vec3 uSunColor;
uniform vec3 uSunDirection;
uniform float uAlpha;

out vec4 outColor;

void main() {
    vec3 normal = normalize(fragNormal);
    float ldotn = max(0.0, dot(normal, uSunDirection));
    vec3 sunComponent = uSunColor * ldotn;
    vec3 halfwayDir = normalize(uSunDirection - vec3(normalize(fragPos)));
    float ldoth = max(dot(normal, halfwayDir), 0.0);
    vec3 specular = uSunColor * pow(ldoth, 30.0) * (30.0 + 1.0) / 200.0;
    vec3 diffuseColor = texture(uTexture, vec2(fragUV.x, 1.0-fragUV.y)).rgb * fragColor;
    outColor.rgb = diffuseColor * (ambientColor + sunComponent) + specular;
    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0/gamma));
    outColor.a = uAlpha;
}