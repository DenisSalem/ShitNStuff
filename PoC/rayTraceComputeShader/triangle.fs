#version 440

in vec4 inColor;
in vec2 TexCoord;
out vec4 color;

uniform sampler2D currentTexture;

void main() {
  color = vec4(1.0) * texture(currentTexture, TexCoord);
}
