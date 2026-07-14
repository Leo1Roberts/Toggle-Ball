precision mediump float;

in vec2 fragUV;

uniform sampler2D screenTexture;

out vec4 outColor;

void main() {
    outColor = texture(screenTexture, fragUV);
}