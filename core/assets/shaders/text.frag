#version 300 es
precision mediump float;

in vec2 fragUV;
in vec4 fragCol;

uniform sampler2D uTexture;

out vec4 outColor;

void main() {
    outColor = vec4(fragCol.rgb, fragCol.a * texture(uTexture, fragUV));
}